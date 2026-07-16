// camera.h — world-space camera: follows player with one-way forward ratchet.
// Owns the world→screen transform origin. Decision (M3a): camera NEVER moves
// backward — bounds chunk-recycling memory at M3b and fits endless-forward design.
#pragma once

namespace camera {

bool init();               // camera at world origin
void update(float dt);     // ratchet toward player (reads entities::playerX)

// RESPawn-only exception to the one-way ratchet (DECISION AMENDED 2026-07-15):
// death can leave the checkpoint behind the camera's left edge, so respawn
// snaps the camera back to it (Pitfall-style). prev==curr after snap — no
// interpolation sweep across the jump. Never call this outside respawn.
void snapTo(float x);

// Latch interpolated camera position for this frame. Same contract/order rules
// as parallax/entities beginRender; MUST run BEFORE theirs (they read drawX).
void beginRender(float alpha);

float x();        // logic camera left edge, world px (update-tick resolution)
float drawX();    // interpolated left edge — valid only after beginRender

}
