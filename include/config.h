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

// --- Camera (M3a) ---
#define CAM_ANCHOR_X        160.0f  // px; player's screen X when pushing forward
                                    // (1/3 from left = 320 px of forward sight line;
                                    // more anchor = more reaction time for hazards)

// --- Parallax (camera-driven since M3a; M1 auto-scroll SCROLL_SPEED removed) ---
#define LAYER_TILE_W        512     // px; tileable width of each pre-rendered layer
#define PAR_FAR_MULT        0.25f   // far cave wall scrolls slowest
#define PAR_MID_MULT        0.50f   // mid rock formations
#define PAR_NEAR_MULT       1.00f   // foreground floor = world speed

// --- Player physics (M2b) ---
// Feel targets: apex ≈ 50 px (clears a 32 px ledge with margin), time-to-apex
// ≈ 0.33 s (snappy, Pitfall-ish). Derivation: apex = JUMP_VEL^2 / (2*GRAVITY).
#define GRAVITY_PX_S2        900.0f // px/s^2; higher = heavier, less floaty
#define PLAYER_JUMP_VEL_PX_S 300.0f // px/s initial jump impulse (upward)
#define PLAYER_MAX_FALL_PX_S 480.0f // px/s terminal fall; 8 px/tick @60 Hz keeps AABB tunnel-proof
#define PLAYER_SPEED_PX_S    140.0f // px/s full-stick run speed; slightly faster than world scroll
#define PLAYER_W             16     // px hitbox width
#define PLAYER_H             24     // px hitbox height
#define PLAYER_SPAWN_X       40.0f  // px; M2b test-harness spawn
#define PLAYER_SPAWN_Y       120.0f // px; spawn in the air → visible gravity on boot
#define PLAYER_PIT_MARGIN_PX 40     // px below screen bottom before pit-death respawn

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
#define COLOR_PLAYER_DEBUG  0xF800  // red — M2b placeholder player rect
#define COLOR_TERRAIN_DEBUG 0x8410  // gray — M2b placeholder terrain (drawn from physics solids)
