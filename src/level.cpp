// level.cpp — chunk chaining: ring of active chunks -> fixed solids pool.
#include "level.h"
#include "physics.h"
#include "entities.h"
#include "camera.h"
#include "config.h"
#include "board_config.h"
#include <Arduino.h>

namespace {

// --- Chunk data format ---------------------------------------------------
struct ChunkSolid { int16_t dx, y, w, h; };   // dx relative to chunk origin
struct Chunk {
  uint16_t width;                             // px; next chunk starts here
  uint8_t  nSolids;
  const ChunkSolid* solids;
};

// --- Chunk library (const, flash) ----------------------------------------
// Floor top = 240, floor slab h = 32 (matches M2/M3a terrain). Ledges = 40 px
// step (y 200), reachable with 50 px apex. Pit widths honor FEEL-1 (<=86 px
// effective) and FEEL-2 (wide pits get in-chunk run-up).

// CH0 "flat" — breather / guaranteed-safe start chunk.
static const ChunkSolid CH0_S[] = {
  { 0, 240, 480, 32 },
};
// CH1 "pit60" — 200 px run-up, 60 px pit, 220 px landing.
static const ChunkSolid CH1_S[] = {
  {   0, 240, 200, 32 },
  { 260, 240, 220, 32 },
};
// CH2 "ledge" — floor with a 40 px step-up platform mid-chunk.
static const ChunkSolid CH2_S[] = {
  {   0, 240, 480, 32 },
  { 190, 200,  96, 16 },
};
// CH3 "pit80" — long chunk: 320 px run-up (FEEL-2), 80 px pit, 240 px landing.
static const ChunkSolid CH3_S[] = {
  {   0, 240, 320, 32 },
  { 400, 240, 240, 32 },
};

static const Chunk CHUNK_LIB[] = {
  { 480, 1, CH0_S },   // 0: flat
  { 480, 2, CH1_S },   // 1: pit60
  { 480, 2, CH2_S },   // 2: ledge
  { 640, 2, CH3_S },   // 3: pit80
};
static const int N_CHUNKS = sizeof(CHUNK_LIB) / sizeof(CHUNK_LIB[0]);
static const int START_CHUNK = 0;             // spawn chunk must be "flat"

// --- Active window --------------------------------------------------------
struct ActiveChunk { const Chunk* c; float originX; };
static ActiveChunk s_active[LEVEL_MAX_ACTIVE];
static int   s_nActive = 0;
static float s_nextOriginX = 0.0f;            // where the next appended chunk starts
static float s_checkpointX = 0.0f;            // last boundary crossed by player
static int   s_lastPick = -1;                 // avoid same chunk twice in a row
static float s_maxDist = 0.0f;                // furthest camera X (score is monotonic
                                              // — respawn camera snap must not dip it)

// Fixed solids pool handed to physics (rebuilt on any window change).
static physics::AABB s_pool[LEVEL_SOLIDS_POOL];
static int s_nPool = 0;

void rebuildPool() {
  s_nPool = 0;
  for (int i = 0; i < s_nActive; ++i) {
    const Chunk* c = s_active[i].c;
    for (int j = 0; j < c->nSolids && s_nPool < LEVEL_SOLIDS_POOL; ++j) {
      const ChunkSolid& s = c->solids[j];
      s_pool[s_nPool++] = { s_active[i].originX + s.dx,
                            (float)s.y, (float)s.w, (float)s.h };
    }
  }
  physics::setSolids(s_pool, s_nPool);
}

int pickChunk() {
  int idx;
  do { idx = random(N_CHUNKS); } while (idx == s_lastPick && N_CHUNKS > 1);
  s_lastPick = idx;
  return idx;
}

// Append chunks until terrain covers view right edge + one full chunk margin.
// Returns true if anything changed.
bool appendAhead() {
  bool changed = false;
  float need = camera::x() + LCD_WIDTH + LEVEL_PREFETCH_PX;
  while (s_nextOriginX < need && s_nActive < LEVEL_MAX_ACTIVE) {
    const Chunk* c = &CHUNK_LIB[pickChunk()];
    s_active[s_nActive++] = { c, s_nextOriginX };
    s_nextOriginX += c->width;
    changed = true;
  }
  return changed;
}

// Drop chunks fully behind the CHECKPOINT (not the camera): respawn terrain
// must survive until the checkpoint advances past it.
bool recycleBehind() {
  int drop = 0;
  while (drop < s_nActive &&
         s_active[drop].originX + s_active[drop].c->width < s_checkpointX)
    ++drop;
  if (drop == 0) return false;
  for (int i = drop; i < s_nActive; ++i) s_active[i - drop] = s_active[i];
  s_nActive -= drop;
  return true;
}

} // namespace

namespace level {

bool init() {
  randomSeed(micros());
  s_nActive = 0;
  s_nextOriginX = 0.0f;
  s_checkpointX = 0.0f;
  s_maxDist = 0.0f;
  s_lastPick = START_CHUNK;                   // "flat" won't repeat immediately
  s_active[s_nActive++] = { &CHUNK_LIB[START_CHUNK], 0.0f };
  s_nextOriginX = CHUNK_LIB[START_CHUNK].width;
  appendAhead();
  rebuildPool();
  return true;
}

void update(float) {
  // Advance checkpoint across every boundary the player has passed.
  float px = entities::playerX();
  for (int i = 0; i < s_nActive; ++i) {
    float boundary = s_active[i].originX;
    if (boundary > s_checkpointX && px >= boundary) s_checkpointX = boundary;
  }

  if (camera::x() > s_maxDist) s_maxDist = camera::x();

  bool changed = recycleBehind();
  changed |= appendAhead();
  if (changed) rebuildPool();
}

float checkpointX() { return s_checkpointX; }
float distancePx()  { return s_maxDist; }

} // namespace level
