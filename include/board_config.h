// board_config.h — Pin map + panel timings for Elecrow CrowPanel 4.3" Basic
// (module DIS06043H, ESP32-S3-WROOM-1-N4R2, NV3047, 16-bit RGB parallel).
// Source of truth: Elecrow official wiki, verified 2026-07-12:
// https://www.elecrow.com/wiki/esp32-display-432727-intelligent-touch-screen-wi-fi26ble-480272-hmi-display.html
// DO NOT change values here without re-checking the schematic.

#pragma once

// ---------------- Display geometry ----------------
#define LCD_WIDTH   480
#define LCD_HEIGHT  272

// ---------------- 16-bit RGB parallel data bus ----------------
// RGB565: 5 red, 6 green, 5 blue. d0 = LSB (blue).
#define LCD_PIN_B0   8
#define LCD_PIN_B1   3
#define LCD_PIN_B2  46
#define LCD_PIN_B3   9
#define LCD_PIN_B4   1

#define LCD_PIN_G0   5
#define LCD_PIN_G1   6
#define LCD_PIN_G2   7
#define LCD_PIN_G3  15
#define LCD_PIN_G4  16
#define LCD_PIN_G5   4

#define LCD_PIN_R0  45
#define LCD_PIN_R1  48
#define LCD_PIN_R2  47
#define LCD_PIN_R3  21
#define LCD_PIN_R4  14

// ---------------- RGB sync/control ----------------
#define LCD_PIN_DE     40
#define LCD_PIN_VSYNC  41
#define LCD_PIN_HSYNC  39
#define LCD_PIN_PCLK   42
#define LCD_PIN_BL      2   // backlight (PWM-capable)

// ---------------- NV3047 panel timings (from Elecrow wiki) ----------------
#define LCD_PCLK_HZ           5000000  // M1 bandwidth experiment — was 8000000 (wiki value).
                                       // Lower pclk = slower panel scan = more PSRAM
                                       // bandwidth left for compose/push. Revert to
                                       // 8000000 if the panel flickers or won't sync.
#define LCD_HSYNC_POLARITY    0
#define LCD_HSYNC_FRONT_PORCH 8
#define LCD_HSYNC_PULSE_WIDTH 4
#define LCD_HSYNC_BACK_PORCH  43
#define LCD_VSYNC_POLARITY    0
#define LCD_VSYNC_FRONT_PORCH 8
#define LCD_VSYNC_PULSE_WIDTH 4
#define LCD_VSYNC_BACK_PORCH  12
#define LCD_PCLK_ACTIVE_NEG   1
#define LCD_DE_IDLE_HIGH      0
#define LCD_PCLK_IDLE_HIGH    0

// ---------------- Resistive touch: XPT2046 on SPI ----------------
// NOTE: shares the SPI bus with the TF card slot (SD CS = GPIO 10).
// NOTE: touch CS is GPIO0, a boot-strapping pin — safe post-boot, but keep
// it deselected (HIGH) during any future OTA/reset shenanigans.
#define TOUCH_PIN_SCK   12
#define TOUCH_PIN_MISO  13
#define TOUCH_PIN_MOSI  11
#define TOUCH_PIN_CS     0
#define TOUCH_PIN_INT   36
#define TOUCH_SPI_HZ    1000000
// Raw ADC calibration range per Elecrow reference config:
#define TOUCH_X_MIN   100
#define TOUCH_X_MAX  4000
#define TOUCH_Y_MIN   100
#define TOUCH_Y_MAX  4000

// ---------------- Other peripherals (reference only, unused at M0) ----------------
#define SD_PIN_CS      10   // shares SPI 11/12/13 with touch
#define UART1_PIN_RX   18
#define UART1_PIN_TX   17
#define I2S_PIN_MCLK   19
#define I2S_PIN_BCLK   35
#define I2S_PIN_SDIN   20
#define GPIO_D_A       37
#define GPIO_D_B       38
