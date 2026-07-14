// entities.h — game entities. M2b: player only (enemies/pickups arrive at M3).
#pragma once

#include <stdint.h>
#include "display.h"

namespace entities {

bool init();               // set player at spawn; no allocation needed
void update(float dt);     // consume input::state(), run physics, handle pit respawn
float playerX();           // player world-space X (camera follow reads this)

// Latch interpolated positions for this frame (prev/curr lerp by alpha).
// MUST be called once per renderFrame before composeBand — same contract as
// parallax::beginRender. Skipping it re-creates the M1 scroll-jerk on the player.
void beginRender(float alpha);

// Draw placeholder terrain + player rect into a band covering rows
// [bandY, bandY + band.height). World→screen X transform via camera::drawX()
// (M3a). Called per band by renderer::renderFrame.
void composeBand(lgfx::LGFX_Sprite& band, int32_t bandY);

}
