#include <SDL3/SDL_log.h>
#include <SDL3/SDL_timer.h>

#include "billboard.h"
#include "simulation.h"

bool SBI_SimulationLoad(SBI_Simulation* state) {
  SBI_CameraLoad(&state->camera, state->viewport.w / state->viewport.h);
  if (!SBI_GridLoad(&state->grid, state->device, state->window)) {
    return false;
  }

  if (!SBI_BillboardLoad(&state->billboard, state->device, state->window)) {
    return false;
  }

  return true;
}

void SBI_SimulationEvent(SBI_Simulation* state, SDL_Event* event) {
  switch (event->type) {
    case SDL_EVENT_WINDOW_RESIZED:
      state->viewport.w = (float)event->window.data1;
      state->viewport.h = (float)event->window.data2;
      SBI_CameraViewportResize(&state->camera,
                               state->viewport.w / state->viewport.h);
      break;
    case SDL_EVENT_MOUSE_WHEEL:
      state->relative_mouse_wheel = -event->wheel.y;
    default:
      break;
  }
}

void SBI_SimulationUpdate(SBI_Simulation* state, float dt) {
  {
    SBI_CameraUpdate(&state->camera, state->window, state->relative_mouse_wheel,
                     dt);
  }
  state->relative_mouse_wheel = 0.0f;
}

bool SBI_SimulationRender(SBI_Simulation* state, float dt) {
  SDL_GPUCommandBuffer* cmd_buf = SDL_AcquireGPUCommandBuffer(state->device);
  if (cmd_buf == NULL) {
    SDL_Log("Could not acquire GPU command buffer: %s", SDL_GetError());
    return false;
  }

  // Get window swap chain texture
  SDL_GPUTexture* swapchain_texture = NULL;
  if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmd_buf, state->window,
                                             &swapchain_texture, NULL, NULL)) {
    SDL_Log("Could not acquire swap chain texture: %s", SDL_GetError());
  }

  // Render when we have a texture
  if (swapchain_texture != NULL) {
    SDL_GPUColorTargetInfo color_target_info = {
        .texture = swapchain_texture,
        .clear_color = (SDL_FColor){0.2f, 0.2f, 0.2f, 1.0f},
        .load_op = SDL_GPU_LOADOP_CLEAR,
        .store_op = SDL_GPU_STOREOP_STORE,
    };

    SDL_GPURenderPass* render_pass =
        SDL_BeginGPURenderPass(cmd_buf, &color_target_info, 1, NULL);
    {
      SDL_SetGPUViewport(render_pass, &state->viewport);

      // Get the camera where we are going to be drawing everything
      SBI_Camera* camera = &state->camera;

      // Draw the grid
      SBI_GridDraw(&state->grid, camera->proj, camera->view, cmd_buf,
                   render_pass);

      // Draw the billboard
      SBI_ALIGN_VEC3 SBI_Vec3 view_pos = {0};
      SBI_XFormGetPosition(camera->xform, view_pos);
      SBI_BillboardDraw(&state->billboard, camera->proj, camera->view, view_pos,
                        cmd_buf, render_pass);
    }
    SDL_EndGPURenderPass(render_pass);
  }

  SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(cmd_buf);
  SDL_WaitForGPUFences(state->device, true, &fence, 1);
  SDL_ReleaseGPUFence(state->device, fence);
  return true;
}

void SBI_SimulationDestroy(SBI_Simulation* state) {
  SBI_GridDestroy(&state->grid);
  SBI_BillboardDestroy(&state->billboard);
}
