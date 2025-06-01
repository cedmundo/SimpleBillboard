#ifndef SBI_BILLBOARD_H
#define SBI_BILLBOARD_H

#include <SDL3/SDL_gpu.h>
#include "xmath.h"

typedef struct {
  SDL_GPUDevice* device;
  SDL_GPUGraphicsPipeline* pipeline;
  SBI_ALIGN_VEC3 SBI_Vec3 position;
} SBI_Billboard;

bool SBI_BillboardLoad(SBI_Billboard* billboard,
                       SDL_GPUDevice* device,
                       SDL_Window* window);

void SBI_BillboardDraw(SBI_Billboard* grid,
                       const SBI_Mat4 proj,
                       const SBI_Mat4 view,
                       const SBI_Vec3 view_pos,
                       SDL_GPUCommandBuffer* cmd_buf,
                       SDL_GPURenderPass* render_pass);

void SBI_BillboardDestroy(SBI_Billboard* billboard);

#endif /* SBI_BILLBOARD_H */
