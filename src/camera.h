// camera.h — world-space camera: follows player with one-way forward ratchet.
// Owns the world→screen transform origin. Decision (M3a): camera NEVER moves
// backward — bounds chunk-recycling memory at M3b and fits endless-forward design.
#pragma once

namespace camera {

bool init();               // camera at world origin
void update(float dt);     // ratchet toward player (reads entities::playerX)

// Latch interpolated camera position for this frame. Same contract/order rules
// as parallax/entities beginRender; MUST run BEFORE theirs (they read drawX).
void beginRender(float alpha);

float x();        // logic camera left edge, world px (update-tick resolution)
float drawX();    // interpolated left edge — valid only after beginRender

}
