// renderer.h — band-based frame compositor: SRAM compose → bulk push to panel FB.
#pragma once

#include <stdint.h>

namespace renderer {
bool init();                   // allocate band buffers (internal SRAM) + start push task
void renderFrame(float alpha); // compose + push all bands; alpha = update-tick fraction for interpolation
}
