#ifndef SBI_CAMERA_H
#define SBI_CAMERA_H

#include <SDL3/SDL_video.h>
#include "xmath.h"

// An orbiting camera implementation
typedef struct {
  SBI_ALIGN_MAT4 SBI_Mat4 proj;
  SBI_ALIGN_MAT4 SBI_Mat4 view;
  SBI_ALIGN_XFORM SBI_XForm xform;
  SBI_ALIGN_VEC3 SBI_Vec3 orbit_point;

  float azimuth;
  float polar;
  float radius;
  float target_radius;

  float orbit_speed;
  float move_speed;
  float zoom_speed;
  float zoom_step;
  float zoom_in_limit;
  float zoom_out_limit;
} SBI_Camera;

// Load camera using perspective projection and default parameters
void SBI_CameraLoad(SBI_Camera* camera, float aspect);

// Notify the camera that viewport size has changed
void SBI_CameraViewportResize(SBI_Camera* camera, float aspect);

// Update the camera position using the default controls
void SBI_CameraUpdate(SBI_Camera* camera,
                      SDL_Window* window,
                      float relative_mouse_wheel,
                      float dt);

#endif /* SBI_CAMERA_H */
