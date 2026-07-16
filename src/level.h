// level.h — chunk library + procedural chaining, checkpoint respawn, distance.
// Owns the live collision set: rebuilds a fixed solids pool from active chunks
// and registers it with physics. Chunks are const flash data (chunk-relative X).
// Authoring rules (FEEL-2): every chunk STARTS with >=96 px flat floor at
// FLOOR_TOP_Y (safe respawn); wide pits (>=70 px) carry their own run-up
// inside the chunk (previous chunk is random — no cross-boundary assumptions).
#pragma once
#include "physics.h"   // AABB

namespace level {

bool init();               // seed RNG, lay start chunk + prefetch, register solids

// Per-tick: advance checkpoint when the player crosses a chunk boundary,
// recycle chunks fully behind the checkpoint, append chunks ahead of the view.
// Call AFTER entities/camera update (reads entities::playerX, camera::x).
void update(float dt);

float checkpointX();       // world X of last crossed chunk boundary (respawn anchor)
const physics::AABB* hazards(int& countOut);  // static hazard boxes (M3c spikes):
                                              // overlap = death; NOT collidable ground
float distancePx();        // score basis: furthest camera X reached

}
