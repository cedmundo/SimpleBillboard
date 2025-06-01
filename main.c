// clang-format off
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_log.h>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
// clang-format on

#include "simulation.h"

#define GAME_CALLBACK __attribute__((unused))
#define WINDOW_TITLE ("SimpleBillboard")
#define WINDOW_WIDTH (800)
#define WINDOW_HEIGHT (800)
#define FIXED_UPDATE_TIME (0.0333333333333f)
#define FIXED_FRAME_TIME (0.0166666666667f)

GAME_CALLBACK SDL_AppResult SDL_AppInit(void** appstate,
                                        int argc,
                                        char** argv) {
  (void)argc;
  (void)argv;

  // Initialize SDL
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    return SDL_APP_FAILURE;
  }

  // Allocate game state
  SBI_Simulation* state = SDL_malloc(sizeof(SBI_Simulation));
  if (state == NULL) {
    SDL_Log("Could not allocate memory for game state");
    return SDL_APP_FAILURE;
  }
  SDL_memset(state, 0, sizeof(SBI_Simulation));

  // Initialize SDL-specific attributes of game state
  state->device =
      SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, "vulkan");
  if (state->device == NULL) {
    SDL_Log("Could not create GPU device: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  state->window = SDL_CreateWindow(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT,
                                   SDL_WINDOW_RESIZABLE);
  if (state->window == NULL) {
    SDL_Log("Could not create window: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  SDL_SetWindowPosition(state->window, SDL_WINDOWPOS_CENTERED,
                        SDL_WINDOWPOS_CENTERED);

  if (!SDL_ClaimWindowForGPUDevice(state->device, state->window)) {
    SDL_Log("Could not claim window for GPU device: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  state->viewport = (SDL_GPUViewport){
      .x = 0,
      .y = 0,
      .w = WINDOW_WIDTH,
      .h = WINDOW_HEIGHT,
      .min_depth = 0.0f,
      .max_depth = 1.0f,
  };

  if (!SBI_SimulationLoad(state)) {
    SDL_Log("Could not load game state");
    return SDL_APP_FAILURE;
  }

  *appstate = state;
  return SDL_APP_CONTINUE;
}

GAME_CALLBACK SDL_AppResult SDL_AppIterate(void* appstate) {
  SBI_Simulation* state = (SBI_Simulation*)appstate;
  Uint64 current_tick = SDL_GetPerformanceCounter();
  state->iter_delta_time = (float)(current_tick - state->last_tick) /
                           (float)SDL_GetPerformanceFrequency();
  state->last_tick = current_tick;
  {
    state->cur_update_time += state->iter_delta_time;
    state->cur_frame_time += state->iter_delta_time;
    if (state->cur_update_time >= FIXED_UPDATE_TIME) {
      // should probably use state->cur_update_time
      SBI_SimulationUpdate(state, FIXED_UPDATE_TIME);
      state->cur_update_time = 0.0f;
    }

    if (state->cur_frame_time >= FIXED_FRAME_TIME) {
      if (!SBI_SimulationRender(state, state->cur_frame_time)) {
        return SDL_APP_FAILURE;
      }
      state->cur_frame_time = 0.0f;
    }
  }

  return SDL_APP_CONTINUE;
}

GAME_CALLBACK SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
  SBI_Simulation* state = (SBI_Simulation*)appstate;
  switch (event->type) {
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
      return SDL_APP_SUCCESS;
    default:
      break;
  }

  SBI_SimulationEvent(state, event);
  return SDL_APP_CONTINUE;
}

GAME_CALLBACK void SDL_AppQuit(void* appstate, SDL_AppResult result) {
  SBI_Simulation* state = (SBI_Simulation*)appstate;
  if (result != SDL_APP_SUCCESS) {
    SDL_Log("Application quit with error: %d", result);
  }

  SBI_SimulationDestroy(state);
  if (state->window != NULL) {
    SDL_ReleaseWindowFromGPUDevice(state->device, state->window);
    SDL_DestroyWindow(state->window);
    SDL_DestroyGPUDevice(state->device);
    state->window = NULL;
    state->device = NULL;
  }

  SDL_free(state);
}
