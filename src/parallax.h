// parallax.h — 3 pre-rendered tileable cave layers; scroll state + band composition.
#pragma once

#include <stdint.h>
#include "display.h"

namespace parallax {
bool init();                       // pre-render layer tiles into PSRAM (once, at boot)
void update(float dt);             // advance scroll offsets; dt in seconds
// Set interpolated render position for this frame: alpha in [0,1) is the
// fraction of an update tick elapsed. Fixes 60 Hz-update/~27 fps-render beat
// (visible scroll jerk on fast layers).
void beginRender(float alpha);
// Draw all layers (back to front) into a band canvas covering
// screen rows [bandY, bandY + band.height).
void composeBand(lgfx::LGFX_Sprite& band, int32_t bandY);
float worldX();                    // current world scroll position, px
}
