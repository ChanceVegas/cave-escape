// renderer.cpp — dual-core band pipeline.
// Core 1 (Arduino loop): composes bands into two ping-pong internal-SRAM buffers.
// Core 0 (push task): bulk-pushes finished bands into the PSRAM framebuffer.
// Frame time ~= max(compose, push) instead of compose + push.
#include "renderer.h"
#include "display.h"
#include "parallax.h"
#include "camera.h"
#include "entities.h"
#include "config.h"
#include "board_config.h"
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

namespace {

constexpr int NUM_BUFS = 2;
lgfx::LGFX_Sprite s_bands[NUM_BUFS];   // ping-pong compose buffers (internal SRAM)

struct BandMsg { uint8_t idx; int16_t y; };
QueueHandle_t s_qFree  = nullptr;      // buffer indices ready for compose
QueueHandle_t s_qReady = nullptr;      // composed bands awaiting push

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

  s_qFree  = xQueueCreate(NUM_BUFS, sizeof(uint8_t));
  s_qReady = xQueueCreate(NUM_BUFS, sizeof(BandMsg));
  if (!s_qFree || !s_qReady) return false;
  for (uint8_t i = 0; i < NUM_BUFS; ++i) xQueueSend(s_qFree, &i, 0);

  // Push task saturates core 0 when it's the bottleneck; without this the
  // idle-task watchdog on core 0 resets the chip every few seconds.
  disableCore0WDT();
  return xTaskCreatePinnedToCore(pushTask, "bandpush", 4096, nullptr, 3, nullptr, 0) == pdPASS;
}

void renderFrame(float alpha) {
  camera::beginRender(alpha);     // FIRST — parallax + entities read camera::drawX
  parallax::beginRender(alpha);
  entities::beginRender(alpha);

  for (int32_t bandY = 0; bandY < LCD_HEIGHT; bandY += BAND_HEIGHT) {
    uint8_t idx;
    xQueueReceive(s_qFree, &idx, portMAX_DELAY);     // wait for a free buffer

    parallax::composeBand(s_bands[idx], bandY);
    entities::composeBand(s_bands[idx], bandY);      // terrain placeholder + player

    BandMsg m{idx, (int16_t)bandY};
    xQueueSend(s_qReady, &m, portMAX_DELAY);         // hand off to core 0
  }
}

} // namespace renderer
