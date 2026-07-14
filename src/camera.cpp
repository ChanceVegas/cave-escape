// camera.cpp — one-way ratchet follow: cam = max(cam, playerX - anchor).
#include "camera.h"
#include "entities.h"
#include "config.h"

namespace {

float s_x = 0.0f;       // logic position (update-tick resolution)
float s_prevX = 0.0f;   // one tick ago (render interpolation)
float s_drawX = 0.0f;   // interpolated, latched by beginRender

} // namespace

namespace camera {

bool init() {
  s_x = s_prevX = s_drawX = 0.0f;
  return true;
}

void update(float) {
  s_prevX = s_x;
  // Hard follow, forward only. No smoothing: player velocity is already
  // capped (PLAYER_SPEED_PX_S) so camera velocity is too; render interp
  // supplies the smoothness. Revisit at M5 only if it feels stiff.
  float target = entities::playerX() - CAM_ANCHOR_X;
  if (target > s_x) s_x = target;
}

void beginRender(float alpha) {
  if (alpha < 0.0f) alpha = 0.0f;
  if (alpha > 1.0f) alpha = 1.0f;
  s_drawX = s_prevX + (s_x - s_prevX) * alpha;
}

float x()     { return s_x; }
float drawX() { return s_drawX; }

} // namespace camera
