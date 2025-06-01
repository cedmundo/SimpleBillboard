#ifndef SBI_GRID_H
#define SBI_GRID_H

#include <SDL3/SDL_gpu.h>
#include "xmath.h"

// Debug grid in XZ plane.
typedef struct {
  SDL_GPUDevice* device;
  SDL_GPUGraphicsPipeline* pipeline;
} SBI_Grid;

// Load the debug grid shaders and resources
bool SBI_GridLoad(SBI_Grid* grid, SDL_GPUDevice* device, SDL_Window* window);

// Draw the debug grid on scene
void SBI_GridDraw(SBI_Grid* grid,
                  const SBI_Mat4 proj,
                  const SBI_Mat4 view,
                  SDL_GPUCommandBuffer* cmd_buf,
                  SDL_GPURenderPass* render_pass);

// Unload the debug grid resources
void SBI_GridUnload(SBI_Grid* grid);

#endif /* SBI_GRID_H */
