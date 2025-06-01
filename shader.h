#ifndef SBI_SHADER_H
#define SBI_SHADER_H

#include <SDL3/SDL_gpu.h>

// General shader options such name and object count.
typedef struct {
  const char* filename;
  Uint32 sampler_count;
  Uint32 uniform_buffer_count;
  Uint32 storage_buffer_count;
  Uint32 storage_texture_count;
  SDL_GPUShaderStage stage;
} SBI_ShaderOptions;

// Load a shader from a SPV file.
SDL_GPUShader* SBI_ShaderLoad(SDL_GPUDevice* device, SBI_ShaderOptions options);

#endif /* SBI_SHADER_H */
