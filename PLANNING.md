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
| gen_layers | tools/gen_layers.py + include/gen_layers.h | placeholder layer generator + generated data | DONE (M1; replaced at M3) |
| main | src/main.cpp | init + fixed-timestep loop | DONE (M1 loop + fps stats) |
| display | src/display.* | driver init, frame present | DONE (M1) |
| renderer | src/renderer.* | dual-core band pipeline (compose core 1 / push core 0) | DONE (M1) |
| parallax | src/parallax.* | flash-resident indexed layers, interpolated scroll | DONE (M1; art is placeholder) |
| sprites | src/sprites.* | sprite structs, animation frames | not started |
| input | src/input.* | drag-joystick gestures → abstract actions (moveX/jump) | DONE (M2a, hw-verified 2026-07-13) |
| physics | src/physics.* | movement, gravity, collision (AABB) | not started |
| entities | src/entities.* | player, enemies, pickups | not started |
| level | src/level.* | chunk library + procedural chaining, difficulty ramp | not started |
| game_state | src/game_state.* | menu / playing / paused / game over | not started |
| hud | src/hud.* | score, health overlay | not started |

## Performance Budget (REVISED at M1 close, 2026-07-12 — measured, not assumed)
- Target: 25 fps floor = 40 ms/frame (revised from 30; see M1 outcome below).
- Measured at M1: 27.2 fps free-running; final = 25.3 fps LOCKED to panel refresh
  (frame limiter in main.cpp; render and panel share a crystal, so frequency-lock
  eliminates the ~2 Hz render-vs-scanout beat that caused visible scroll judder).
- Core-1 budget at M2/M3: frame period 39.6 ms − compose ~32 ms − updates ~1 ms
  ≈ 6 ms/frame for ALL game logic (input, physics, entities, level). Tight.
  First overdraft response: RLE compositor (below) to shrink compose.
- Binding constraint: flash + PSRAM share one memory bus; compose flash reads,
  push PSRAM writes, and panel scanout all serialize on it. More parallelism
  cannot help; only less traffic can.
- Levers already used: pclk 8→4 MHz (~25 Hz refresh, panel verified OK by eye),
  flash-resident indexed layers, row-skip flags, dual-core band pipeline.
- Reserve lever if M3 dips below 25 fps: RLE-span compositor (store layer rows
  as color runs; cuts flash reads hard on flat cave art). Deliberately deferred —
  tune it against real art, not placeholders.
- Full frame = 480x272x2 bytes = 255 KB → framebuffer requires PSRAM.
- M1 GO/NO-GO outcome: 30 fps not reachable with current architecture at
  acceptable panel refresh; target consciously revised to 25 fps floor per
  the "or strategy revisited" clause. Decision: user, 2026-07-12.

## Milestones
- **M0 — Toolchain proof:** PlatformIO project builds, flashes, blinks a pixel/fills screen. Confirms board config.
- **M1 — Render skeleton:** ✅ DONE 2026-07-12. 25.3 fps locked to panel refresh, 3 layers + 10 sprites, judder-free scroll. Gate resolved by revising target to 25 fps floor (documented above).
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
