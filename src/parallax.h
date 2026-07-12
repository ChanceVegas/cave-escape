// parallax.h — 3 pre-rendered tileable cave layers; scroll state + band composition.
#pragma once

#include <stdint.h>
#include "display.h"

namespace parallax {
bool init();                       // pre-render layer tiles into PSRAM (once, at boot)
void update(float dt);             // advance scroll offsets; dt in seconds
// Draw all layers (back to front) into a band canvas covering
// screen rows [bandY, bandY + band.height).
void composeBand(lgfx::LGFX_Sprite& band, int32_t bandY);
float worldX();                    // current world scroll position, px
}
