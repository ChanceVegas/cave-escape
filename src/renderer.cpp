// renderer.cpp — dual-core band pipeline.
// Core 1 (Arduino loop): composes bands into two ping-pong internal-SRAM buffers.
// Core 0 (push task): bulk-pushes finished bands into the PSRAM framebuffer.
// Frame time ~= max(compose, push) instead of compose + push.
#include "renderer.h"
#include "display.h"
#include "parallax.h"
#include "config.h"
#include "board_config.h"
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

namespace {

constexpr int NUM_BUFS = 2;
lgfx::LGFX_Sprite s_bands[NUM_BUFS];   // ping-pong compose buffers (internal SRAM)
lgfx::LGFX_Sprite s_tex;               // shared 16x16 test-sprite texture

struct BandMsg { uint8_t idx; int16_t y; };
QueueHandle_t s_qFree  = nullptr;      // buffer indices ready for compose
QueueHandle_t s_qReady = nullptr;      // composed bands awaiting push

struct TestSprite { float x, y, vx, vy; };
TestSprite s_sprites[TEST_SPRITE_COUNT];

uint32_t s_seed = 0xBADCAFE;
uint32_t prng() { s_seed = s_seed * 1664525u + 1013904223u; return s_seed; }
float prngF(float lo, float hi) { return lo + (hi - lo) * ((prng() & 0xFFFF) / 65535.0f); }

// Core 0: pull composed bands, write them into the panel framebuffer.
void pushTask(void*) {
  auto& lcd = display::lcd();
  BandMsg m;
  for (;;) {
    xQueueReceive(s_qReady, &m, portMAX_DELAY);
    s_bands[m.idx].pushSprite(&lcd, 0, m.y);
    uint8_t idx = m.idx;
    xQueueSend(s_qFree, &idx, portMAX_DELAY);
  }
}

} // namespace

namespace renderer {

bool init() {
  for (auto& b : s_bands) {
    b.setColorDepth(16);
    b.setPsram(false);  // MUST be internal SRAM — that's the whole strategy
    if (b.createSprite(LCD_WIDTH, BAND_HEIGHT) == nullptr) return false;
  }

  s_tex.setColorDepth(16);
  s_tex.setPsram(false);
  if (s_tex.createSprite(TEST_SPRITE_SIZE, TEST_SPRITE_SIZE) == nullptr) return false;
  s_tex.fillScreen(COLOR_TRANSPARENT);
  s_tex.fillCircle(TEST_SPRITE_SIZE / 2, TEST_SPRITE_SIZE / 2,
                   TEST_SPRITE_SIZE / 2 - 1, TFT_YELLOW);
  s_tex.drawCircle(TEST_SPRITE_SIZE / 2, TEST_SPRITE_SIZE / 2,
                   TEST_SPRITE_SIZE / 2 - 1, TFT_ORANGE);

  for (auto& s : s_sprites) {
    s.x  = prngF(0, LCD_WIDTH - TEST_SPRITE_SIZE);
    s.y  = prngF(0, LCD_HEIGHT - TEST_SPRITE_SIZE);
    s.vx = prngF(-90, 90);
    s.vy = prngF(-90, 90);
  }

  s_qFree  = xQueueCreate(NUM_BUFS, sizeof(uint8_t));
  s_qReady = xQueueCreate(NUM_BUFS, sizeof(BandMsg));
  if (!s_qFree || !s_qReady) return false;
  for (uint8_t i = 0; i < NUM_BUFS; ++i) xQueueSend(s_qFree, &i, 0);

  // Push task saturates core 0 when it's the bottleneck; without this the
  // idle-task watchdog on core 0 resets the chip every few seconds.
  disableCore0WDT();
  return xTaskCreatePinnedToCore(pushTask, "bandpush", 4096, nullptr, 3, nullptr, 0) == pdPASS;
}

void update(float dt) {
  for (auto& s : s_sprites) {
    s.x += s.vx * dt;
    s.y += s.vy * dt;
    if (s.x < 0)                              { s.x = 0; s.vx = -s.vx; }
    if (s.x > LCD_WIDTH  - TEST_SPRITE_SIZE)  { s.x = LCD_WIDTH  - TEST_SPRITE_SIZE; s.vx = -s.vx; }
    if (s.y < 0)                              { s.y = 0; s.vy = -s.vy; }
    if (s.y > LCD_HEIGHT - TEST_SPRITE_SIZE)  { s.y = LCD_HEIGHT - TEST_SPRITE_SIZE; s.vy = -s.vy; }
  }
}

void renderFrame(float alpha) {
  parallax::beginRender(alpha);

  for (int32_t bandY = 0; bandY < LCD_HEIGHT; bandY += BAND_HEIGHT) {
    uint8_t idx;
    xQueueReceive(s_qFree, &idx, portMAX_DELAY);     // wait for a free buffer

    parallax::composeBand(s_bands[idx], bandY);
    for (const auto& s : s_sprites) {
      int32_t sy = (int32_t)s.y;
      if (sy + TEST_SPRITE_SIZE <= bandY || sy >= bandY + BAND_HEIGHT) continue;
      s_tex.pushSprite(&s_bands[idx], (int32_t)s.x, sy - bandY, COLOR_TRANSPARENT);
    }
    BandMsg m{idx, (int16_t)bandY};
    xQueueSend(s_qReady, &m, portMAX_DELAY);         // hand off to core 0
  }
}

} // namespace renderer
