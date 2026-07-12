// main.cpp — M0 smoke test ONLY. No game code.
// Init NV3047 RGB panel via LovyanGFX, cycle full-screen fills
// (red/green/blue/white), report heap + PSRAM each cycle over serial.

#include <Arduino.h>
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <lgfx/v1/platforms/esp32s3/Panel_RGB.hpp>
#include <lgfx/v1/platforms/esp32s3/Bus_RGB.hpp>
#include "board_config.h"

// LGFX device config for CrowPanel 4.3" Basic, per Elecrow wiki reference.
class LGFX_CrowPanel43 : public lgfx::LGFX_Device {
  lgfx::Bus_RGB     _bus;
  lgfx::Panel_RGB   _panel;
  lgfx::Light_PWM   _light;

public:
  LGFX_CrowPanel43() {
    {
      auto cfg = _panel.config();
      cfg.memory_width  = LCD_WIDTH;
      cfg.memory_height = LCD_HEIGHT;
      cfg.panel_width   = LCD_WIDTH;
      cfg.panel_height  = LCD_HEIGHT;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      _panel.config(cfg);
    }
    {
      auto cfg = _bus.config();
      cfg.panel = &_panel;

      cfg.pin_d0  = LCD_PIN_B0;  cfg.pin_d1  = LCD_PIN_B1;
      cfg.pin_d2  = LCD_PIN_B2;  cfg.pin_d3  = LCD_PIN_B3;
      cfg.pin_d4  = LCD_PIN_B4;
      cfg.pin_d5  = LCD_PIN_G0;  cfg.pin_d6  = LCD_PIN_G1;
      cfg.pin_d7  = LCD_PIN_G2;  cfg.pin_d8  = LCD_PIN_G3;
      cfg.pin_d9  = LCD_PIN_G4;  cfg.pin_d10 = LCD_PIN_G5;
      cfg.pin_d11 = LCD_PIN_R0;  cfg.pin_d12 = LCD_PIN_R1;
      cfg.pin_d13 = LCD_PIN_R2;  cfg.pin_d14 = LCD_PIN_R3;
      cfg.pin_d15 = LCD_PIN_R4;

      cfg.pin_henable = LCD_PIN_DE;
      cfg.pin_vsync   = LCD_PIN_VSYNC;
      cfg.pin_hsync   = LCD_PIN_HSYNC;
      cfg.pin_pclk    = LCD_PIN_PCLK;
      cfg.freq_write  = LCD_PCLK_HZ;

      cfg.hsync_polarity    = LCD_HSYNC_POLARITY;
      cfg.hsync_front_porch = LCD_HSYNC_FRONT_PORCH;
      cfg.hsync_pulse_width = LCD_HSYNC_PULSE_WIDTH;
      cfg.hsync_back_porch  = LCD_HSYNC_BACK_PORCH;
      cfg.vsync_polarity    = LCD_VSYNC_POLARITY;
      cfg.vsync_front_porch = LCD_VSYNC_FRONT_PORCH;
      cfg.vsync_pulse_width = LCD_VSYNC_PULSE_WIDTH;
      cfg.vsync_back_porch  = LCD_VSYNC_BACK_PORCH;
      cfg.pclk_active_neg   = LCD_PCLK_ACTIVE_NEG;
      cfg.de_idle_high      = LCD_DE_IDLE_HIGH;
      cfg.pclk_idle_high    = LCD_PCLK_IDLE_HIGH;

      _bus.config(cfg);
      _panel.setBus(&_bus);
    }
    {
      auto cfg = _light.config();
      cfg.pin_bl = LCD_PIN_BL;
      _light.config(cfg);
      _panel.light(&_light);
    }
    setPanel(&_panel);
  }
};

static LGFX_CrowPanel43 lcd;

static void printMemory(const char* tag) {
  Serial.printf("[%s] heap free: %u bytes | PSRAM total: %u | PSRAM free: %u\n",
                tag,
                (unsigned)ESP.getFreeHeap(),
                (unsigned)ESP.getPsramSize(),
                (unsigned)ESP.getFreePsram());
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== Cave Escape M0 smoke test ===");
  printMemory("pre-init");

  if (!lcd.init()) {
    Serial.println("FATAL: lcd.init() failed");
  }
  lcd.setBrightness(255);
  printMemory("post-init");

  // Sanity check: PSRAM must be present (framebuffer lives there).
  if (ESP.getPsramSize() == 0) {
    Serial.println("WARNING: PSRAM not detected — check memory_type=qio_qspi");
  }
}

void loop() {
  struct { uint16_t color; const char* name; } fills[] = {
    { TFT_RED,   "RED"   },
    { TFT_GREEN, "GREEN" },
    { TFT_BLUE,  "BLUE"  },
    { TFT_WHITE, "WHITE" },
  };
  for (auto& f : fills) {
    uint32_t t0 = millis();
    lcd.fillScreen(f.color);
    uint32_t dt = millis() - t0;
    Serial.printf("fill %-5s took %lu ms | ", f.name, (unsigned long)dt);
    printMemory("loop");
    delay(1000);
  }
}
