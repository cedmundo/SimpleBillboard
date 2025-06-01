#ifndef SBI_PARTICLE_SYSTEM_H
#define SBI_PARTICLE_SYSTEM_H

#include <SDL3/SDL_gpu.h>
#include "xmath.h"

// Single simulated particle.
typedef struct {
  SBI_ALIGN_VEC3 SBI_Vec3 position;
  SBI_ALIGN_VEC3 SBI_Vec3 velocity;
  float mass;
} SBI_Particle;

// Particle simulation, it can also render the partciles.
typedef struct {
  SDL_GPUDevice* device;
  SDL_GPUGraphicsPipeline* pipeline;
  SDL_GPUBuffer* vp_buffer;
  SDL_GPUBuffer* pp_buffer;
  SDL_GPUTransferBuffer* upload_transfer_buffer;
  SBI_Particle* particles;
  Uint64 particle_count;
} SBI_ParticleSystem;

// Initializes the particle system with a fixed count of partciles.
bool SBI_ParticleSystemInit(SBI_ParticleSystem* ps,
                            Uint64 count,
                            SDL_GPUDevice* device,
                            SDL_Window* window);

// Prints to logs the particle positions and mass.
void SBI_ParticleSystemDebug(SBI_ParticleSystem* ps);

// Draws the partcile system entirely.
bool SBI_ParticleSystemDraw(SBI_ParticleSystem* ps,
                            const SBI_Mat4 proj,
                            const SBI_Mat4 view,
                            SDL_GPURenderPass* render_pass);

// Updates the particle system simulation.
void SBI_ParticleSystemUpdate(SBI_ParticleSystem* ps, float dt);

// Releases the resources used by the particle system simulation.
void SBI_ParticleSystemDestroy(SBI_ParticleSystem* ps);

#endif /* SBI_PARTICLE_SYSTEM_H */
