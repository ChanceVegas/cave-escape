// parallax.cpp — flash-resident indexed layers + custom row compositor.
// Layers live in flash as const 8-bit indexed arrays (gen_layers.h) per
// CLAUDE.md rule 3. Compose reads flash (cached, no PSRAM contention) and
// writes the internal-SRAM band; palette LUTs are copied to SRAM at init.
#include "parallax.h"
#include <string.h>
#include "config.h"
#include "board_config.h"
#include "gen_layers.h"

namespace {

float s_worldX = 0.0f;

// Palette copies in internal SRAM (flash palette reads would hit cache anyway,
// but SRAM LUTs make the inner loop unconditional-fast).
uint16_t s_palFar[sizeof(far_pal) / 2];
uint16_t s_palMid[sizeof(mid_pal) / 2];
uint16_t s_palNear[sizeof(near_pal) / 2];

inline void rowOpaque(uint16_t* dst, const uint8_t* src, const uint16_t* pal, int n) {
  for (int i = 0; i < n; ++i) dst[i] = pal[src[i]];
}
inline void rowKeyed(uint16_t* dst, const uint8_t* src, const uint16_t* pal, int n) {
  for (int i = 0; i < n; ++i) { uint8_t v = src[i]; if (v) dst[i] = pal[v]; }
}

// Compose one layer row into dst (LCD_WIDTH px) with horizontal wrap.
template <bool KEYED>
inline void layerRow(uint16_t* dst, const uint8_t* rowBase, const uint16_t* pal, int32_t off) {
  int n1 = GEN_LAYER_W - off;              // pixels before wrap
  if (n1 > LCD_WIDTH) n1 = LCD_WIDTH;
  if (KEYED) {
    rowKeyed(dst, rowBase + off, pal, n1);
    if (n1 < LCD_WIDTH) rowKeyed(dst + n1, rowBase, pal, LCD_WIDTH - n1);
  } else {
    rowOpaque(dst, rowBase + off, pal, n1);
    if (n1 < LCD_WIDTH) rowOpaque(dst + n1, rowBase, pal, LCD_WIDTH - n1);
  }
}

inline int32_t layerOffset(float mult) {
  int32_t off = (int32_t)(s_worldX * mult) % GEN_LAYER_W;
  return off < 0 ? off + GEN_LAYER_W : off;
}

} // namespace

namespace parallax {

bool init() {
  memcpy(s_palFar,  far_pal,  sizeof(far_pal));
  memcpy(s_palMid,  mid_pal,  sizeof(mid_pal));
  memcpy(s_palNear, near_pal, sizeof(near_pal));
  return true;
}

void update(float dt) { s_worldX += SCROLL_SPEED_PX_S * dt; }

void composeBand(lgfx::LGFX_Sprite& band, int32_t bandY) {
  uint16_t* buf = (uint16_t*)band.getBuffer();
  const int32_t bandH = band.height();
  const int32_t offFar  = layerOffset(PAR_FAR_MULT);
  const int32_t offMid  = layerOffset(PAR_MID_MULT);
  const int32_t offNear = layerOffset(PAR_NEAR_MULT);

  for (int32_t r = 0; r < bandH; ++r) {
    const int32_t y = bandY + r;
    if (y >= LCD_HEIGHT) break;
    uint16_t* dst = buf + r * LCD_WIDTH;
    const uint8_t* rowFar  = far_px  + y * GEN_LAYER_W;
    const uint8_t* rowMid  = mid_px  + y * GEN_LAYER_W;
    const uint8_t* rowNear = near_px + y * GEN_LAYER_W;
    layerRow<false>(dst, rowFar, s_palFar, offFar);           // opaque base
    if (mid_rowflag[y])  layerRow<true>(dst, rowMid,  s_palMid,  offMid);
    if (near_rowflag[y]) layerRow<true>(dst, rowNear, s_palNear, offNear);
  }
}

float worldX() { return s_worldX; }

} // namespace parallax
