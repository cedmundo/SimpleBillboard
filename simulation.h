#ifndef SBI_SIMULATION_H
#define SBI_SIMULATION_H
// clang-format off
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_gpu.h>
// clang-format on

#include "camera.h"
#include "grid.h"
#include "shader.h"

#define MAX_PARTICLES (10000)

// Global values for the simulation
typedef struct {
  SDL_Window* window;
  SDL_GPUDevice* device;
  SDL_GPUViewport viewport;
  SBI_Camera camera;
  SBI_Grid grid;
  Uint64 last_tick;
  float iter_delta_time;
  float cur_frame_time;
  float cur_update_time;
  float relative_mouse_wheel;
} SBI_Simulation;

// Load the simulation.
bool SBI_SimulationLoad(SBI_Simulation* state);

// Let simulation handle an event from SDL.
void SBI_SimulationEvent(SBI_Simulation* state, SDL_Event* event);

// Update the simulation (fixed rate).
void SBI_SimulationUpdate(SBI_Simulation* state, float dt);

// Render the simulation (fixed rate).
bool SBI_SimulationRender(SBI_Simulation* state, float dt);

// Release the resources creates by the simulation.
void SBI_SimulationDestroy(SBI_Simulation* state);

#endif /* SBI_SIMULATION_H */
