// main.cpp — M3a: fixed-timestep loop + camera-scroll test harness.
// Terrain below is throwaway (deleted at M3b): ~3.5 screens of floor with pits
// and ledges to verify camera follow, back-edge clamp, and scrolling collision.
#include <Arduino.h>
#include "display.h"
#include "parallax.h"
#include "renderer.h"
#include "input.h"
#include "physics.h"
#include "entities.h"
#include "camera.h"
#include "config.h"
#include "board_config.h"

// Exact panel refresh period, derived from pclk + timings (same crystal as
// micros(), so locking render to this period phase-locks us to scanout and
// kills the ~2 Hz render-vs-refresh beat). 535*296 clks @4MHz = 39590 us.
static const uint32_t FRAME_US =
    (uint64_t)(LCD_WIDTH  + LCD_HSYNC_FRONT_PORCH + LCD_HSYNC_PULSE_WIDTH + LCD_HSYNC_BACK_PORCH) *
    (LCD_HEIGHT + LCD_VSYNC_FRONT_PORCH + LCD_VSYNC_PULSE_WIDTH + LCD_VSYNC_BACK_PORCH) *
    1000000ULL / LCD_PCLK_HZ;

static const float UPDATE_DT = 1.0f / UPDATE_HZ;

// --- M3a test terrain (WORLD space, ~3.5 screens = 1680 px; throwaway) ---
// Pit widths honor FEEL-1: max clearance ~92 px minus latency margin → ≤86 px;
// widest here is 80 (stretch) after a 60 (warm-up). Reachable 40 px ledges.
static const physics::AABB TEST_SOLIDS[] = {
  {    0, 240,  400, 32 },   // start floor
  {  460, 240,  340, 32 },   // pit 1: x 400..460 (60 px, M2b-verified width)
  {  560, 200,   96, 16 },   // ledge A: 40 px step up, reachable (apex 50 px)
  {  870, 240,  410, 32 },   // pit 2: x 800..870 (70 px — FEEL-1 data point)
  { 1360, 240,  320, 32 },   // pit 3: x 1280..1360 (80 px), then final floor
  { 1420, 200,   96, 16 },   // ledge B on final floor
};

// --- fps instrumentation ---
static uint32_t s_frames = 0;
static uint32_t s_statT0 = 0;
static uint32_t s_frameMsMin = 0xFFFFFFFF, s_frameMsMax = 0, s_frameMsSum = 0;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== Cave Escape M3A-R1 ===");

  if (!display::init())  { Serial.println("FATAL: display init failed");  for(;;) delay(1000); }
  if (!parallax::init()) { Serial.println("FATAL: parallax init failed (PSRAM alloc?)"); for(;;) delay(1000); }
  if (!renderer::init()) { Serial.println("FATAL: renderer init failed (SRAM alloc?)");  for(;;) delay(1000); }
  if (!input::init())    { Serial.println("FATAL: input init failed"); for(;;) delay(1000); }
  physics::setSolids(TEST_SOLIDS, sizeof(TEST_SOLIDS) / sizeof(TEST_SOLIDS[0]));
  if (!camera::init())   { Serial.println("FATAL: camera init failed"); for(;;) delay(1000); }
  if (!entities::init()) { Serial.println("FATAL: entities init failed"); for(;;) delay(1000); }

  Serial.printf("post-init heap free: %u | PSRAM free: %u\n",
                (unsigned)ESP.getFreeHeap(), (unsigned)ESP.getFreePsram());
  s_statT0 = millis();
}

void loop() {
  // Fixed-timestep update, render as fast as possible (CLAUDE.md rule 4).
  static uint32_t lastUs = micros();
  static float acc = 0.0f;

  uint32_t nowUs = micros();
  acc += (nowUs - lastUs) / 1000000.0f;
  lastUs = nowUs;
  if (acc > 0.25f) acc = 0.25f;            // clamp after stalls (e.g. serial)

  while (acc >= UPDATE_DT) {
    input::update(UPDATE_DT);
    entities::update(UPDATE_DT);   // consumes input::state(); order matters
    camera::update(UPDATE_DT);     // AFTER entities — follows this tick's player
    acc -= UPDATE_DT;
  }

  // Frame limiter: lock render cadence to the panel refresh period.
  static uint32_t s_nextFrameUs = micros();
  int32_t wait = (int32_t)(s_nextFrameUs - micros());
  if (wait > 2000) delayMicroseconds(wait - 1000);       // coarse sleep
  while ((int32_t)(s_nextFrameUs - micros()) > 0) {}     // fine spin
  s_nextFrameUs += FRAME_US;
  if ((int32_t)(micros() - s_nextFrameUs) > (int32_t)FRAME_US)
    s_nextFrameUs = micros() + FRAME_US;                 // resync after a stall

  uint32_t t0 = millis();
  renderer::renderFrame(acc / UPDATE_DT);  // alpha: fraction of a tick elapsed
  uint32_t dt = millis() - t0;

  ++s_frames;
  s_frameMsSum += dt;
  if (dt < s_frameMsMin) s_frameMsMin = dt;
  if (dt > s_frameMsMax) s_frameMsMax = dt;

  uint32_t now = millis();
  if (now - s_statT0 >= 1000) {
    float fps = s_frames * 1000.0f / (now - s_statT0);
    Serial.printf("fps: %.1f | render ms avg %.1f min %lu max %lu | heap %u\n",
                  fps, s_frames ? (float)s_frameMsSum / s_frames : 0.0f,
                  (unsigned long)s_frameMsMin, (unsigned long)s_frameMsMax,
                  (unsigned)ESP.getFreeHeap());
    s_frames = 0; s_frameMsSum = 0; s_frameMsMin = 0xFFFFFFFF; s_frameMsMax = 0;
    s_statT0 = now;
  }
}
