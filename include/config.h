// config.h — ALL tunable constants live here (CLAUDE.md rule 5).
// M1: render-skeleton tunables only. Gameplay feel constants arrive at M2.

#pragma once

// --- Frame timing ---
#define TARGET_FPS          25      // fps floor; revised at M1 close (was 30).
                                    // Measured 27.2 with render skeleton; game logic
                                    // shares core 1 with compose, expect ~25 at M3.
                                    // If M3 dips below floor: RLE-span compositor (PLANNING).
#define UPDATE_HZ           60      // Hz; fixed-timestep logic rate, decoupled from render

// --- Band compositor ---
// Frame is composed in horizontal bands in internal SRAM, then bulk-pushed to
// the PSRAM framebuffer. 272 / 34 = 8 bands. Bigger band = fewer pushes but
// more SRAM (480 * BAND_HEIGHT * 2 bytes; 34 px = 32,640 B).
#define BAND_HEIGHT         34

// --- Parallax ---
#define LAYER_TILE_W        512     // px; tileable width of each pre-rendered layer
#define SCROLL_SPEED_PX_S   120.0f  // px/sec world scroll; brisk auto-run feel
#define PAR_FAR_MULT        0.25f   // far cave wall scrolls slowest
#define PAR_MID_MULT        0.50f   // mid rock formations
#define PAR_NEAR_MULT       1.00f   // foreground floor = world speed

// --- M1 test sprites (throwaway; replaced by real entities at M2/M3) ---
#define TEST_SPRITE_COUNT   10
#define TEST_SPRITE_SIZE    16      // px, square

// --- Input: single-touch drag joystick (PLANNING #5) ---
#define INPUT_DEADZONE_PX    10     // px from anchor before movement registers; kills resistive jitter
#define INPUT_JOY_RANGE_PX   60     // px of drag = full speed; smaller = twitchier
#define INPUT_JUMP_FLICK_PX  30     // finger must rise this far above anchor to jump
#define INPUT_JUMP_REARM_PX  15     // finger must drop back within this of anchor to re-arm jump
#define INPUT_EMA_ALPHA      0.5f   // touch smoothing 0..1; higher = snappier, noisier
#define INPUT_JUMP_VEL_PX    14     // upward px between consecutive samples = flick.
                                    // Touch samples at ~25 Hz (once/frame), so a real
                                    // flick steps 20-60 px/sample; jitter is 2-5 px.

// --- Colors (RGB565) ---
#define COLOR_TRANSPARENT   0xF81F  // magenta color key for layer/sprite transparency
