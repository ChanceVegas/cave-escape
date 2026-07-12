# PLANNING.md — Architecture & Roadmap

## Game Concept — "Cave Escape" (defined 2026-07-11)
Pitfall-inspired side-scroller. Player character traverses a scrolling cave,
avoiding hazards; contact with a hazard = death.

Core loop (v1 scope — keep it shippable):
- Move left/right + jump (drag joystick per Open Decision #5).
- Hazards: pits (fall = death), stalactites/spikes, patrolling creatures (bats,
  snakes), rolling boulders. Start with 2 hazard types at M3, add the rest after.
- Lives: 3, respawn at last checkpoint. [ASSUMPTION — user may override]
- Score: treasure pickups + distance. [ASSUMPTION]
- Parallax layers: distant cave wall / mid rock formations / foreground floor & hazards.

- Structure: ENDLESS for v1 — procedurally chained hand-authored chunks (each chunk =
  a short obstacle pattern), difficulty ramps with distance. Chunks are data, not code,
  so fixed levels later = an ordered chunk list. Checkpoints every N chunks.

Explicitly OUT of v1 (candidate M5+ stretch): vine swings, ladders/vertical
sections, crouch, boss, audio. Do not build these early.

## Rendering Approach (2.5D)
- 3–4 parallax background layers scrolling at different speeds → depth illusion.
- Sprites drawn back-to-front (painter's algorithm).
- Strategy: PSRAM framebuffer via esp_lcd RGB driver; compose in internal-SRAM
  buffers, blit to framebuffer. Candidate libs: esp_lcd native or LovyanGFX
  (both support S3 RGB panels). TFT_eSPI is OUT — no RGB parallel support.
- GO/NO-GO at M1: measured fps with 3 parallax layers + 10 sprites.

## Module Map (planned — update as built)
| Module | Files | Purpose | Status |
|---|---|---|---|
| board_config | include/board_config.h | pins, display driver, bus | DONE (M0) |
| config | include/config.h | all tunable gameplay constants | M1 render tunables done |
| main | src/main.cpp | init + fixed-timestep loop | DONE (M1 loop + fps stats) |
| display | src/display.* | driver init, frame present | DONE (M1) |
| renderer | src/renderer.* | layer/sprite compositing | M1 band compositor done |
| parallax | src/parallax.* | background layer scrolling | M1 placeholder layers done |
| sprites | src/sprites.* | sprite structs, animation frames | not started |
| input | src/input.* | touch zones → game actions | not started |
| physics | src/physics.* | movement, gravity, collision (AABB) | not started |
| entities | src/entities.* | player, enemies, pickups | not started |
| level | src/level.* | chunk library + procedural chaining, difficulty ramp | not started |
| game_state | src/game_state.* | menu / playing / paused / game over | not started |
| hud | src/hud.* | score, health overlay | not started |

## Performance Budget (revise after M1 measurements)
- Target: 30 fps = 33 ms/frame. Render ≤ 22 ms, update ≤ 5 ms, headroom ≥ 6 ms.
- Full frame = 480x272x2 bytes = 255 KB → framebuffer requires PSRAM.

## Milestones
- **M0 — Toolchain proof:** PlatformIO project builds, flashes, blinks a pixel/fills screen. Confirms board config.
- **M1 — Render skeleton:** parallax layers scrolling at measured fps. GO/NO-GO: ≥30 fps or strategy revisited.
- **M2 — Player:** sprite, touch input, jump physics, ground collision. (Concept must be defined by now.)
- **M3 — World:** level data, obstacles, enemies, camera scroll.
- **M4 — Game loop:** states, score, HUD, lose/win.
- **M5 — Polish:** animation, tuning pass on config.h, optional audio.

## Open Decisions
1. ~~Board model~~ RESOLVED 2026-07-11: CrowPanel 4.3" Basic, S3 N4R2, RGB parallel, resistive touch.
2. ~~Graphics lib~~ RESOLVED 2026-07-12: LovyanGFX (verified on hardware at M0). esp_lcd native remains fallback if M1 fps gate fails.
3. PC emulator layer (Raylib wrapper, per reference article). Deferred; revisit if flash-test
   cycle exceeds ~2 min or after M1.
4. Game concept/theme (user decision, needed before M2).
5. ~~Input scheme~~ RESOLVED 2026-07-11: single-touch drag joystick.
   - Touch-down sets anchor; horizontal drag offset from anchor = move direction/speed (dead zone ~10px).
   - Upward flick (fast vertical delta) = jump. Release = stop.
   - Input module exposes abstract actions ONLY (moveX float, jump bool) so a BLE
     gamepad backend can be swapped in later without touching game code.
