# PROGRESS.md — Single Source of Truth Across Sessions

Every session: read this first, update it last. If it isn't logged here, the next session doesn't know it happened.

## Current State
- Phase: M0 COMPLETE ✅ (2026-07-12) — builds, flashes, verified on hardware
- Builds: yes (pio run, espressif32@6.5.0, RAM 6.2%, Flash 25.9%)
- Runs on hardware: yes — RGBW fill cycle correct, colors in right order (R/B bus order verified)
- Measured FPS: n/a (M1). Baseline datum: full-screen fillScreen = 29–30 ms via LovyanGFX → PSRAM framebuffer.

## Next Up (in order)
1. Commit M0 (message below in log). Update PLANNING.md: Open Decision #2 → RESOLVED, LovyanGFX.
2. M1: parallax render skeleton — compose in internal-SRAM buffer(s), blit to framebuffer; measure fps with 3 layers + 10 sprites. GO/NO-GO ≥30 fps.
3. M2 prep: input module (single-touch drag joystick, XPT2046 backend).

## Known Issues / Risks
- HW-2 (open risk, now with data): full-screen fill = 29–30 ms ≈ entire 33 ms frame budget. Naive full-frame redraw cannot hit 30 fps. M1 MUST use compose-in-SRAM + partial blits per CLAUDE.md mitigations.
- HW-3 (resolved note): touch = XPT2046 on SPI shared with TF slot (SD CS=10); touch CS = GPIO0 strapping pin. Documented in board_config.h.
- M0-3 (RESOLVED 2026-07-12): LovyanGFX Bus_RGB/Panel_RGB need explicit includes — lgfx/v1/platforms/esp32s3/{Panel_RGB,Bus_RGB}.hpp — not pulled in by LovyanGFX.hpp.
- M0-2 (note): platform pinned espressif32@6.5.0 (Arduino core 2.0.14, Elecrow-validated); LovyanGFX ^1.1.16 works.
- HW-1 (RESOLVED 2026-07-11): board = CrowPanel 4.3" Basic, RGB parallel. Amazon "SPI" listing was wrong.

## Decisions Made
- 2026-07-12: Open Decision #2 RESOLVED — Graphics lib = LovyanGFX. Confirmed working on hardware at M0. esp_lcd native remains fallback if M1 fps gate fails.
- 2026-07-11: Endless structure for v1 via procedurally chained hand-authored chunks; fixed levels later = ordered chunk list.
- 2026-07-11: Game = "Cave Escape", Pitfall-style hazard-avoidance side-scroller. v1 scope locked; vines/ladders/crouch deferred.
- 2026-07-11: Input = single-touch drag joystick (anchor + drag, flick-up jump), abstracted for future BLE gamepad.
- 2026-07-11: PlatformIO + Arduino framework + C++; git (no manual version file).
- 2026-07-11: Docs = CLAUDE.md / PLANNING.md / PROGRESS.md; session monitoring = this file.
- 2026-07-11: PC emulator deferred (PLANNING #3).

## Session Log (newest first)
### 2026-07-12 — Session 2
- Done: M0 COMPLETE. platformio.ini (espressif32@6.5.0, qio_qspi, 4MB), board_config.h (pin map + NV3047 timings verified vs Elecrow wiki, module DIS06043H), main.cpp smoke test. Built, flashed, verified: RGBW cycle correct, PSRAM 2,095,023 B detected, heap stable.
- Fixed en route: files must live in src/ + include/ (PlatformIO layout); LovyanGFX esp32s3 RGB headers must be included explicitly (M0-3).
- Data: fillScreen 29–30 ms → HW-2 confirmed as the central M1 risk; full-frame redraws are off the table.
- Decision: LovyanGFX adopted (Open Decision #2 closed).
- Commit: feat(m0): PlatformIO skeleton + CrowPanel 4.3 board config + display smoke test

### 2026-07-11 — Session 1 (cont. 2)
- Done: repo initialized and pushed to GitHub.

### 2026-07-11 — Session 1 (cont.)
- Done: hardware confirmed (S3 N4R2, RGB parallel, NV3047, resistive touch, 4MB/2MB). HW-1 closed; HW-2/HW-3 opened.

### 2026-07-11 — Session 1
- Done: scaffolding (CLAUDE.md, PLANNING.md, PROGRESS.md); toolchain chosen; reference article scoped to 2.5D approach.

## Changelog (human-readable milestones)
- v0.1.0 — 2026-07-12 — M0: toolchain proven on hardware. Display + PSRAM verified.
- v0.0.0 — 2026-07-11 — Project scaffolded. No code.
