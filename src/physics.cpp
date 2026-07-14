// physics.cpp — gravity + axis-separated AABB collision.
// Axis-separated (move X, resolve, move Y, resolve) is exact for static rects
// at our speeds: max displacement/tick = MAX_FALL/UPDATE_HZ ≈ 8 px << tile size,
// so no tunneling and no swept tests needed.
#include "physics.h"
#include "config.h"

namespace {

const physics::AABB* s_solids = nullptr;
int s_solidCount = 0;

inline bool overlap(const physics::AABB& a, const physics::AABB& b) {
  return a.x < b.x + b.w && a.x + a.w > b.x &&
         a.y < b.y + b.h && a.y + a.h > b.y;
}

} // namespace

namespace physics {

void setSolids(const AABB* solids, int count) {
  s_solids = solids;
  s_solidCount = count;
}

const AABB* solids(int& countOut) { countOut = s_solidCount; return s_solids; }

void step(Body& b, float dt) {
  // Gravity (units: px/s^2). Clamp fall speed so collision stays tunnel-proof.
  b.vy += GRAVITY_PX_S2 * dt;
  if (b.vy > PLAYER_MAX_FALL_PX_S) b.vy = PLAYER_MAX_FALL_PX_S;

  // --- X axis ---
  b.box.x += b.vx * dt;
  for (int i = 0; i < s_solidCount; ++i) {
    const AABB& s = s_solids[i];
    if (!overlap(b.box, s)) continue;
    if (b.vx > 0)      b.box.x = s.x - b.box.w;   // hit solid's left face
    else if (b.vx < 0) b.box.x = s.x + s.w;       // hit solid's right face
    b.vx = 0;
  }

  // --- Y axis ---
  b.grounded = false;
  b.box.y += b.vy * dt;
  for (int i = 0; i < s_solidCount; ++i) {
    const AABB& s = s_solids[i];
    if (!overlap(b.box, s)) continue;
    if (b.vy > 0) {                                // falling: land on top face
      b.box.y = s.y - b.box.h;
      b.grounded = true;
    } else if (b.vy < 0) {                         // rising: bonk underside
      b.box.y = s.y + s.h;
    }
    b.vy = 0;
  }
}

} // namespace physics
