// renderer.h — band-based frame compositor: SRAM compose → bulk push to panel FB.
#pragma once

#include <stdint.h>

namespace renderer {
bool init();               // allocate band buffer (internal SRAM) + test sprites
void update(float dt);     // move M1 test sprites (throwaway logic)
void renderFrame();        // compose + push all bands; one full frame
}
