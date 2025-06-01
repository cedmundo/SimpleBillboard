#include "grid.h"
#include "shader.h"

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_log.h>

typedef struct {
  SBI_ALIGN_MAT4 SBI_Mat4 pv;
  SBI_ALIGN_MAT4 SBI_Mat4 pv_inv;
} Grid_ViewParams;

bool SBI_GridLoad(SBI_Grid* grid, SDL_GPUDevice* device, SDL_Window* window) {
  grid->device = device;
  SBI_ShaderOptions vert_options = (SBI_ShaderOptions){
      .filename = "grid.vert",
      .stage = SDL_GPU_SHADERSTAGE_VERTEX,
      .sampler_count = 0,
      .uniform_buffer_count = 0,
      .storage_buffer_count = 1,
      .storage_texture_count = 0,
  };
  SDL_GPUShader* vert_shader = SBI_ShaderLoad(device, vert_options);
  if (vert_shader == NULL) {
    return false;
  }

  SBI_ShaderOptions frag_options = (SBI_ShaderOptions){
      .filename = "grid.frag",
      .stage = SDL_GPU_SHADERSTAGE_FRAGMENT,
      .sampler_count = 0,
      .uniform_buffer_count = 0,
      .storage_buffer_count = 0,
      .storage_texture_count = 0,
  };
  SDL_GPUShader* frag_shader = SBI_ShaderLoad(device, frag_options);
  if (frag_shader == NULL) {
    return false;
  }

  SDL_GPUGraphicsPipelineTargetInfo color_target_info = {
      .num_color_targets = 1,
      .color_target_descriptions = (SDL_GPUColorTargetDescription[]){{
          .format = SDL_GetGPUSwapchainTextureFormat(device, window),
          .blend_state =
              (SDL_GPUColorTargetBlendState){
                  .enable_blend = true,
                  .src_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
                  .dst_color_blendfactor =
                      SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                  .color_blend_op = SDL_GPU_BLENDOP_ADD,
                  .src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
                  .dst_alpha_blendfactor =
                      SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                  .alpha_blend_op = SDL_GPU_BLENDOP_ADD,
              },
      }},
  };

  SDL_GPUGraphicsPipelineCreateInfo pipeline_create_info = {
      .target_info = color_target_info,
      .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
      .vertex_shader = vert_shader,
      .fragment_shader = frag_shader,
  };
  grid->pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipeline_create_info);

  SDL_ReleaseGPUShader(device, vert_shader);
  SDL_ReleaseGPUShader(device, frag_shader);

  if (grid->pipeline == NULL) {
    SDL_Log("Couldn't create graphics pipeline for debug grid");
    return false;
  }

  // Create buffer location for transform
  SDL_GPUBufferCreateInfo buffer_create_info = {
      .usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ,
      .size = sizeof(Grid_ViewParams),
  };
  grid->buffer = SDL_CreateGPUBuffer(device, &buffer_create_info);
  if (grid->buffer == NULL) {
    SDL_Log("Couldn't create buffer to store the params of debug grid");
    return false;
  }

  // Create transfer buffer handle
  SDL_GPUTransferBufferCreateInfo upload_transfer_buffer_create_info = {
      .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
      .size = sizeof(Grid_ViewParams),
  };
  grid->upload_transfer_buffer =
      SDL_CreateGPUTransferBuffer(device, &upload_transfer_buffer_create_info);
  if (grid->upload_transfer_buffer == NULL) {
    SDL_Log("Couldn't create transfer buffer of debug grid");
    return false;
  }

  return true;
}

void SBI_GridDraw(SBI_Grid* grid,
                  const SBI_Mat4 proj,
                  const SBI_Mat4 view,
                  SDL_GPURenderPass* render_pass) {
  Grid_ViewParams view_params = {0};
  SBI_Mat4Mul(proj, view, view_params.pv);
  SBI_Mat4Invert(view_params.pv, view_params.pv_inv);

  SDL_BindGPUGraphicsPipeline(render_pass, grid->pipeline);
  {
    // Copy data to the staging of the GPU
    void* transfer_point =
        SDL_MapGPUTransferBuffer(grid->device, grid->upload_transfer_buffer, 0);
    SDL_memcpy(transfer_point, &view_params, sizeof(Grid_ViewParams));
    SDL_UnmapGPUTransferBuffer(grid->device, grid->upload_transfer_buffer);

    // Create a copy pass
    SDL_GPUCommandBuffer* upload_cmd_buf =
        SDL_AcquireGPUCommandBuffer(grid->device);
    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(upload_cmd_buf);
    {
      SDL_GPUTransferBufferLocation source = {
          .transfer_buffer = grid->upload_transfer_buffer,
          .offset = 0,
      };
      SDL_GPUBufferRegion destination = {
          .buffer = grid->buffer,
          .offset = 0,
          .size = sizeof(Grid_ViewParams),
      };

      SDL_UploadToGPUBuffer(copy_pass, &source, &destination, false);
      SDL_EndGPUCopyPass(copy_pass);
      SDL_SubmitGPUCommandBuffer(upload_cmd_buf);
    }

    SDL_BindGPUVertexStorageBuffers(render_pass, 0, &grid->buffer, 1);
    SDL_DrawGPUPrimitives(render_pass, 6, 1, 0, 0);
  }
}

void SBI_GridUnload(SBI_Grid* grid) {
  SDL_ReleaseGPUGraphicsPipeline(grid->device, grid->pipeline);
  SDL_ReleaseGPUTransferBuffer(grid->device, grid->upload_transfer_buffer);
  SDL_ReleaseGPUBuffer(grid->device, grid->buffer);
}
