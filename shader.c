#include "shader.h"

#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_log.h>

SDL_GPUShader *SBI_ShaderLoad(SDL_GPUDevice *device,
                              SBI_ShaderOptions options) {
  char full_path[512] = {0};
  SDL_snprintf(full_path, sizeof(full_path), "%sassets/shaders/%s.spv",
               SDL_GetBasePath(), options.filename);
  SDL_GPUShaderFormat supported_formats = SDL_GetGPUShaderFormats(device);
  if (!(supported_formats & SDL_GPU_SHADERFORMAT_SPIRV)) {
    SDL_Log("GPU device doesn't support SPIR-V shader format");
    return NULL;
  }

  size_t code_size;
  void *code_data = SDL_LoadFile(full_path, &code_size);
  if (code_data == NULL) {
    SDL_Log("Couldn't load shader code: %s", SDL_GetError());
    return NULL;
  }

  SDL_GPUShaderCreateInfo shader_create_info = {
      .code = code_data,
      .code_size = code_size,
      .entrypoint = "main",
      .stage = options.stage,
      .format = SDL_GPU_SHADERFORMAT_SPIRV,
      .num_samplers = options.sampler_count,
      .num_uniform_buffers = options.uniform_buffer_count,
      .num_storage_buffers = options.storage_buffer_count,
      .num_storage_textures = options.storage_texture_count,
  };
  SDL_GPUShader *shader = SDL_CreateGPUShader(device, &shader_create_info);
  SDL_Log("Loaded shader: %s", full_path);
  SDL_free(code_data);
  return shader;
}
