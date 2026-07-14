// parallax.h — 3 pre-rendered tileable cave layers; scroll state + band composition.
#pragma once

#include <stdint.h>
#include "display.h"

namespace parallax {
bool init();                       // copy palettes to SRAM (layers live in flash)
// Latch this frame's scroll position from camera::drawX() (M3a: parallax is
// camera-driven; the M1 self-scroll is gone). camera::beginRender MUST have
// run first this frame.
void beginRender(float alpha);
// Draw all layers (back to front) into a band canvas covering
// screen rows [bandY, bandY + band.height).
void composeBand(lgfx::LGFX_Sprite& band, int32_t bandY);
}
