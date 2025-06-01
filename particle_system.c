#include "particle_system.h"
#include "shader.h"
#include "xmath.h"

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_stdinc.h>

typedef struct {
  SBI_ALIGN_MAT4 SBI_Mat4 pv;
} Particle_ViewParams;

void particle_compute_force(const SBI_Particle* particle, SBI_Vec3 dest);
float remap_value(float value,
                  float start1,
                  float stop1,
                  float start2,
                  float stop2);

bool SBI_ParticleSystemInit(SBI_ParticleSystem* ps,
                            Uint64 count,
                            SDL_GPUDevice* device,
                            SDL_Window* window) {
  ps->device = device;
  ps->particle_count = count;
  ps->particles = SDL_malloc(sizeof(SBI_Particle) * count);
  if (ps->particles == NULL) {
    return false;
  }

  // Initialize particle positions to random places
  SDL_memset(ps->particles, 0, sizeof(SBI_Particle) * count);
  for (Uint64 i = 0; i < count; i++) {
    float rand_x = remap_value(SDL_randf() * 20.0f, 0.0f, 20.0f, -10.0f, 10.0f);
    float rand_y = remap_value(SDL_randf() * 20.0f, 0.0f, 20.0f, 0.0f, 40.0f);
    float rand_z = remap_value(SDL_randf() * 20.0f, 0.0f, 20.0f, -10.0f, 10.0f);
    SBI_Vec3Make(rand_x, rand_y, rand_z, ps->particles[i].position);
    SBI_Vec3Make(0.0f, 0.0f, 0.0f, ps->particles[i].velocity);
    ps->particles[i].mass = 1.0f;
  }

  // Initialize pipeline
  SBI_ShaderOptions vs_opts = {
      .filename = "particle_system.vert",
      .stage = SDL_GPU_SHADERSTAGE_VERTEX,
      .sampler_count = 0,
      .uniform_buffer_count = 0,
      .storage_buffer_count = 2,
      .storage_texture_count = 0,
  };
  SBI_ShaderOptions fs_opts = {
      .filename = "particle_system.frag",
      .stage = SDL_GPU_SHADERSTAGE_FRAGMENT,
      .sampler_count = 0,
      .uniform_buffer_count = 0,
      .storage_buffer_count = 0,
      .storage_texture_count = 0,
  };

  SDL_GPUShader* vs = SBI_ShaderLoad(device, vs_opts);
  if (vs == NULL) {
    return false;
  }

  SDL_GPUShader* fs = SBI_ShaderLoad(device, fs_opts);
  if (fs == NULL) {
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
      .vertex_shader = vs,
      .fragment_shader = fs,
  };
  ps->pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipeline_create_info);

  SDL_ReleaseGPUShader(device, vs);
  SDL_ReleaseGPUShader(device, fs);

  if (ps->pipeline == NULL) {
    SDL_Log("Could not create graphics pipeline for particle system!: %s",
            SDL_GetError());
    return false;
  }

  // Create buffer and transfer buffer
  size_t vp_buffer_size = sizeof(Particle_ViewParams);
  size_t pp_buffer_size = sizeof(SBI_Particle) * ps->particle_count;
  SDL_GPUBufferCreateInfo buffer_create_info = {0};

  buffer_create_info.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;
  buffer_create_info.size = vp_buffer_size;
  ps->vp_buffer = SDL_CreateGPUBuffer(device, &buffer_create_info);
  if (ps->vp_buffer == NULL) {
    return false;
  }

  buffer_create_info.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;
  buffer_create_info.size = pp_buffer_size;
  ps->pp_buffer = SDL_CreateGPUBuffer(device, &buffer_create_info);
  if (ps->pp_buffer == NULL) {
    return false;
  }

  SDL_GPUTransferBufferCreateInfo upload_transfer_buffer_create_info = {
      .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
      .size = vp_buffer_size + pp_buffer_size,
  };
  ps->upload_transfer_buffer =
      SDL_CreateGPUTransferBuffer(device, &upload_transfer_buffer_create_info);
  if (ps->upload_transfer_buffer == NULL) {
    SDL_Log("Could not create transfer buffer of particle system");
    return false;
  }

  return true;
}

void SBI_ParticleSystemDebug(SBI_ParticleSystem* ps) {
  for (Uint64 i = 0; i < ps->particle_count; i++) {
    SBI_Particle p = ps->particles[i];
    SDL_Log("particle[%ld] = (%+.2f, %+.2f, %+.2f) mass = %+.2f\n", i,
            p.position[0], p.position[1], p.position[2], p.mass);
  }
}

bool SBI_ParticleSystemDraw(SBI_ParticleSystem* ps,
                            const SBI_Mat4 proj,
                            const SBI_Mat4 view,
                            SDL_GPURenderPass* render_pass) {
  Particle_ViewParams view_params = {0};
  SBI_Mat4Mul(proj, view, view_params.pv);

  SDL_BindGPUGraphicsPipeline(render_pass, ps->pipeline);
  {
    // Copy view params and particle positions to the GPU
    void* transfer_point =
        SDL_MapGPUTransferBuffer(ps->device, ps->upload_transfer_buffer, 0);
    SDL_memcpy(transfer_point, &view_params, sizeof(Particle_ViewParams));
    SDL_memcpy(transfer_point + sizeof(Particle_ViewParams), ps->particles,
               sizeof(SBI_Particle) * ps->particle_count);
    SDL_UnmapGPUTransferBuffer(ps->device, ps->upload_transfer_buffer);

    // Create a copy pass
    SDL_GPUCommandBuffer* upload_cmd_buf =
        SDL_AcquireGPUCommandBuffer(ps->device);
    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(upload_cmd_buf);
    {
      SDL_GPUTransferBufferLocation src;
      SDL_GPUBufferRegion dst;

      // Upload view params
      src.transfer_buffer = ps->upload_transfer_buffer;
      src.offset = 0;

      dst.buffer = ps->vp_buffer;
      dst.offset = 0;
      dst.size = sizeof(Particle_ViewParams);
      SDL_UploadToGPUBuffer(copy_pass, &src, &dst, false);

      // Upload positions
      src.transfer_buffer = ps->upload_transfer_buffer;
      src.offset = sizeof(Particle_ViewParams);

      dst.buffer = ps->pp_buffer;
      dst.offset = 0;
      dst.size = sizeof(SBI_Particle) * ps->particle_count;
      SDL_UploadToGPUBuffer(copy_pass, &src, &dst, false);

      SDL_EndGPUCopyPass(copy_pass);
      SDL_SubmitGPUCommandBuffer(upload_cmd_buf);
    }

    SDL_BindGPUVertexStorageBuffers(render_pass, 0, &ps->vp_buffer, 1);
    SDL_BindGPUVertexStorageBuffers(render_pass, 1, &ps->pp_buffer, 1);
    SDL_DrawGPUPrimitives(render_pass, 6, ps->particle_count, 0, 0);
  }

  return true;
}

void SBI_ParticleSystemUpdate(SBI_ParticleSystem* ps, float dt) {
  SBI_ALIGN_VEC3 SBI_Vec3 force = {0};
  SBI_ALIGN_VEC3 SBI_Vec3 acceleration = {0};
  SBI_ALIGN_VEC3 SBI_Vec3 aux = {0};

  for (Uint64 i = 0; i < ps->particle_count; i++) {
    SBI_Particle* p = &ps->particles[i];
    particle_compute_force(p, force);
    SBI_Vec3Scale(force, 1.0f / p->mass, acceleration);

    // p.velocity = p.velocity + acceleration * dt
    SBI_Vec3Scale(acceleration, dt, aux);
    SBI_Vec3Add(p->velocity, aux, p->velocity);

    // p.position = p.position + p.velocity * dt
    SBI_Vec3Scale(p->velocity, dt, aux);
    SBI_Vec3Add(p->position, aux, p->position);
  }
}

void SBI_ParticleSystemDestroy(SBI_ParticleSystem* ps) {
  SDL_ReleaseGPUGraphicsPipeline(ps->device, ps->pipeline);
  SDL_ReleaseGPUTransferBuffer(ps->device, ps->upload_transfer_buffer);
  SDL_ReleaseGPUBuffer(ps->device, ps->vp_buffer);
  SDL_ReleaseGPUBuffer(ps->device, ps->pp_buffer);

  if (ps->particles != NULL) {
    SDL_free(ps->particles);
    ps->particles = NULL;
    ps->particle_count = 0;
  }
}

void particle_compute_force(const SBI_Particle* particle, SBI_Vec3 dest) {
  // F = ma (gravity)
  dest[0] = 0.0f;
  dest[1] = particle->mass * -9.81;
  dest[2] = 0.0f;
}

float remap_value(float value,
                  float start1,
                  float stop1,
                  float start2,
                  float stop2) {
  return start2 + (stop2 - start2) * ((value - start1) / (stop1 - start1));
}
