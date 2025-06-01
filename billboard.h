#ifndef SBI_BILLBOARD_H
#define SBI_BILLBOARD_H

#include <SDL3/SDL_gpu.h>
#include "xmath.h"

typedef struct {
  SDL_GPUDevice* device;
  SDL_GPUGraphicsPipeline* pipeline;
  SDL_GPUBuffer* buffer;
  SDL_GPUTransferBuffer* upload_transfer_buffer;
  SBI_Vec4* instances;
  Uint64 instances_count;
} SBI_Billboard;

bool SBI_BillboardLoad(SBI_Billboard* billboard,
                       SDL_GPUDevice* device,
                       SDL_Window* window,
                       Uint64 instances_count);

void SBI_BillboardDraw(SBI_Billboard* grid,
                       const SBI_Mat4 proj,
                       const SBI_Mat4 view,
                       const SBI_Vec3 view_pos,
                       SDL_GPUCommandBuffer* cmd_buf,
                       SDL_GPURenderPass* render_pass);

void SBI_BillboardDestroy(SBI_Billboard* billboard);

#endif /* SBI_BILLBOARD_H */
