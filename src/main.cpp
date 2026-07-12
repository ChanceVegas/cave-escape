// main.cpp — M1: fixed-timestep loop, parallax + test sprites, fps measurement.
// GO/NO-GO gate: sustained >= 30 fps with 3 layers + 10 sprites (PLANNING.md M1).
#include <Arduino.h>
#include "display.h"
#include "parallax.h"
#include "renderer.h"
#include "config.h"

static const float UPDATE_DT = 1.0f / UPDATE_HZ;

// --- fps instrumentation ---
static uint32_t s_frames = 0;
static uint32_t s_statT0 = 0;
static uint32_t s_frameMsMin = 0xFFFFFFFF, s_frameMsMax = 0, s_frameMsSum = 0;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== Cave Escape M1 render skeleton ===");

  if (!display::init())  { Serial.println("FATAL: display init failed");  for(;;) delay(1000); }
  if (!parallax::init()) { Serial.println("FATAL: parallax init failed (PSRAM alloc?)"); for(;;) delay(1000); }
  if (!renderer::init()) { Serial.println("FATAL: renderer init failed (SRAM alloc?)");  for(;;) delay(1000); }

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
    parallax::update(UPDATE_DT);
    renderer::update(UPDATE_DT);
    acc -= UPDATE_DT;
  }

  uint32_t t0 = millis();
  renderer::renderFrame();
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
