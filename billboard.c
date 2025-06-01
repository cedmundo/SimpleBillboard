#include "billboard.h"
#include "shader.h"
#include "xmath.h"

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_stdinc.h>

typedef struct {
  SBI_ALIGN_MAT4 SBI_Mat4 pv;
  SBI_ALIGN_VEC3 SBI_Vec3 view_pos;
} BillboardUniforms;

float remap_value(float value,
                  float start1,
                  float stop1,
                  float start2,
                  float stop2);

bool SBI_BillboardLoad(SBI_Billboard* billboard,
                       SDL_GPUDevice* device,
                       SDL_Window* window,
                       Uint64 instances_count) {
  billboard->instances_count = instances_count;

  size_t instances_buffer_size = sizeof(SBI_Vec4) * instances_count;
  billboard->instances = SDL_aligned_alloc(16, instances_buffer_size);
  if (billboard->instances == NULL) {
    SDL_Log("Could not allocate memory for %ld billboards", instances_count);
    return false;
  }
  SDL_memset(billboard->instances, 0, instances_buffer_size);

  for (Uint64 i = 0; i < instances_count; i++) {
    float rx = remap_value(SDL_randf(), 0.0f, 1.0f, -10.0f, 10.0f);
    float ry = remap_value(SDL_randf(), 0.0f, 1.0f, -10.0f, 10.0f);
    float rz = remap_value(SDL_randf(), 0.0f, 1.0f, -10.0f, 10.0f);
    billboard->instances[i][0] = rx;
    billboard->instances[i][1] = ry;
    billboard->instances[i][2] = rz;
    billboard->instances[i][3] = 0.5f;
  }

  billboard->device = device;
  SBI_ShaderOptions vert_options = (SBI_ShaderOptions){
      .filename = "billboard.vert",
      .stage = SDL_GPU_SHADERSTAGE_VERTEX,
      .sampler_count = 0,
      .uniform_buffer_count = 1,
      .storage_buffer_count = 1,
      .storage_texture_count = 0,
  };
  SDL_GPUShader* vert_shader = SBI_ShaderLoad(device, vert_options);
  if (vert_shader == NULL) {
    return false;
  }

  SBI_ShaderOptions frag_options = (SBI_ShaderOptions){
      .filename = "billboard.frag",
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
  billboard->pipeline =
      SDL_CreateGPUGraphicsPipeline(device, &pipeline_create_info);

  SDL_ReleaseGPUShader(device, vert_shader);
  SDL_ReleaseGPUShader(device, frag_shader);

  if (billboard->pipeline == NULL) {
    SDL_Log("Couldn't create graphics pipeline for billboard");
    return false;
  }

  // Create buffer location for transform
  SDL_GPUBufferCreateInfo buffer_create_info = {
      .usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ,
      .size = instances_buffer_size,
  };
  billboard->buffer = SDL_CreateGPUBuffer(device, &buffer_create_info);
  if (billboard->buffer == NULL) {
    SDL_Log("Couldn't create buffer to store the params of debug grid");
    return false;
  }

  // Create transfer buffer handle
  SDL_GPUTransferBufferCreateInfo upload_transfer_buffer_create_info = {
      .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
      .size = instances_buffer_size,
  };
  billboard->upload_transfer_buffer =
      SDL_CreateGPUTransferBuffer(device, &upload_transfer_buffer_create_info);
  if (billboard->upload_transfer_buffer == NULL) {
    SDL_Log("Couldn't create transfer buffer of debug grid");
    return false;
  }

  return true;
}

void SBI_BillboardDraw(SBI_Billboard* billboard,
                       const SBI_Mat4 proj,
                       const SBI_Mat4 view,
                       const SBI_Vec3 view_pos,
                       SDL_GPUCommandBuffer* cmd_buf,
                       SDL_GPURenderPass* render_pass) {
  BillboardUniforms uniforms = {0};
  SBI_Mat4Mul(proj, view, uniforms.pv);
  SBI_Vec3Copy(view_pos, uniforms.view_pos);

  SDL_BindGPUGraphicsPipeline(render_pass, billboard->pipeline);
  {
    // Copy data to the staging of the GPU
    void* transfer_point = SDL_MapGPUTransferBuffer(
        billboard->device, billboard->upload_transfer_buffer, 0);
    SDL_memcpy(transfer_point, billboard->instances,
               sizeof(SBI_Vec4) * billboard->instances_count);
    SDL_UnmapGPUTransferBuffer(billboard->device,
                               billboard->upload_transfer_buffer);

    // Create a copy pass
    SDL_GPUCommandBuffer* upload_cmd_buf =
        SDL_AcquireGPUCommandBuffer(billboard->device);
    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(upload_cmd_buf);
    {
      SDL_GPUTransferBufferLocation source = {
          .transfer_buffer = billboard->upload_transfer_buffer,
          .offset = 0,
      };
      SDL_GPUBufferRegion destination = {
          .buffer = billboard->buffer,
          .offset = 0,
          .size = sizeof(SBI_Vec4) * billboard->instances_count,
      };

      SDL_UploadToGPUBuffer(copy_pass, &source, &destination, false);
      SDL_EndGPUCopyPass(copy_pass);
      SDL_SubmitGPUCommandBuffer(upload_cmd_buf);
    }
  }

  SDL_PushGPUVertexUniformData(cmd_buf, 0, &uniforms,
                               sizeof(BillboardUniforms));
  SDL_BindGPUVertexStorageBuffers(render_pass, 0, &billboard->buffer, 1);
  SDL_DrawGPUPrimitives(render_pass, 6, billboard->instances_count, 0, 0);
}

void SBI_BillboardDestroy(SBI_Billboard* billboard) {
  SDL_ReleaseGPUGraphicsPipeline(billboard->device, billboard->pipeline);
  SDL_ReleaseGPUBuffer(billboard->device, billboard->buffer);
  SDL_ReleaseGPUTransferBuffer(billboard->device,
                               billboard->upload_transfer_buffer);

  if (billboard->instances != NULL) {
    SDL_aligned_free(billboard->instances);
    billboard->instances = NULL;
    billboard->instances_count = 0;
  }
}

float remap_value(float value,
                  float start1,
                  float stop1,
                  float start2,
                  float stop2) {
  return start2 + (stop2 - start2) * ((value - start1) / (stop1 - start1));
}
