# CLAUDE.md — ESP32 2.5D Side-Scroller

Read this file and PROGRESS.md at the start of EVERY session. Do not write code before doing so.

## Project
2.5D side-scrolling game for an Elecrow 4.3" ESP32 HMI display (480x272 touch).
Reference technique source: https://medium.com/@davidmonterocrespo24/how-i-built-the-first-3d-racing-game-for-esp32-s3-because-someone-said-ai-could-do-it-better-50236a02286f
(That project is pseudo-3D racing; ours is parallax side-scrolling. Borrow its *process* — PC-first iteration, painter's-algorithm layering, hand-tuned constants — not its projection math.)

## Hardware — ✅ CONFIRMED 2026-07-11
Elecrow CrowPanel 4.3" Basic (ESP32-S3-WROOM-1-N4R2):
- MCU: ESP32-S3, dual-core LX7 @ 240 MHz, 512KB SRAM
- Flash: 4MB (asset budget is tight — keep sprites lean, RGB565)
- PSRAM: 2MB QUAD SPI (not octal — bandwidth is the scarce resource)
- Display: 480x272 TFT, **16-bit RGB parallel bus**, driver IC NV3047
- Touch: **resistive, SINGLE touch point** (major input-design constraint)
- Extras: TF card slot, speaker interface, battery interface

Rendering implications (binding until M1 measurements say otherwise):
- Framebuffer (255KB) lives in PSRAM; esp_lcd RGB driver DMAs it to panel continuously.
- Panel refresh + game drawing SHARE quad-PSRAM bandwidth. Mitigations: bounce
  buffers in internal SRAM, compose scanline/region in SRAM then memcpy to PSRAM,
  restart-on-vsync. Never draw pixel-by-pixel directly into the PSRAM framebuffer.
- Single touch point: no simultaneous move+jump via touch. Input scheme is an
  open decision in PLANNING.md — do not code controls before it's made.

## Toolchain
- PlatformIO, Arduino framework, C++.
- Build: `pio run`
- Flash: `pio run -t upload`
- Serial: `pio device monitor -b 115200`
- Board def and pin map go in `platformio.ini` + `include/board_config.h` once hardware is confirmed.

## Coding Rules
1. Modular: one system per file pair (`.h/.cpp`). No god files.
2. No dynamic allocation (`new`/`malloc`/`String`) inside the game loop. Allocate at init.
3. All colors RGB565. All sprite assets compiled in as `const uint16_t` arrays in PROGMEM/flash.
4. Fixed-timestep update, decoupled from render. Target 30 fps minimum; measure, don't assume.
5. Tunable gameplay constants live ONLY in `include/config.h`, each with a comment stating units and feel intent.
6. Float is fine on S3 (has FPU); avoid `double` everywhere.
7. Every module gets a one-line purpose comment at top and is listed in PLANNING.md's module map. If you add a module, update PLANNING.md in the same commit.

## Session Workflow (mandatory)
1. SESSION START = CLONE. Clone https://github.com/ChanceVegas/cave-escape.git and
   read CLAUDE.md + PROGRESS.md + PLANNING.md FROM THE CLONE. The repo is the only
   source of truth. Never trust files attached to a chat, memory summaries, or a
   leftover container working tree without diffing them against the repo first —
   stale attachments cost a full session of confusion once (see PROGRESS 07-14).
   If repo state contradicts the session's inputs, STOP and raise it.
2. Work ONE task from "Next Up" in PROGRESS.md. Small scope.
3. Before ending: update PROGRESS.md (state, session log entry, known issues) and
   commit it IN THE SAME COMMIT as the feature work — doc drift is a logged failure.
4. SESSION END = COMMIT. All work gets committed before the session closes, even
   incomplete work as `wip(scope): ...` on a branch. Uncommitted work does not
   exist (PROC-1: hardware-verified code was nearly lost and rewritten).
5. Commit format: `type(scope): summary` — types: feat, fix, refactor, docs, perf,
   chore, wip. One logical change per commit. Never commit non-compiling code to
   main (wip commits live on branches only). Never `git add -A` blindly — check
   `git status` first; build artifacts (.pio/) are gitignored and must stay out.
6. Merges happen LOCALLY, not via the GitHub web UI (mixing the two forked history
   once). Branch per milestone (m3-world, ...); merge to main at milestone close.
7. If you discover a design problem, do NOT silently work around it. Log it in
   PROGRESS.md "Known Issues" and raise it with the user.
8. Code transfer to the target machine follows the hardware test protocol in
   PROGRESS.md Decisions (pbpaste, xfer-tag grep, upload SUCCESS, RST + banner).

## Repo Map
- Remote: https://github.com/ChanceVegas/cave-escape.git (public)
- CLAUDE.md — this file (rules, hardware, toolchain)
- PLANNING.md — architecture, module map, milestones, open decisions
- PROGRESS.md — current state, session log, known issues, changelog
- src/, include/ — code; tools/ — asset generators
- .gitignore — .pio/ and .vscode/ stay out of the repo. These docs live at repo
  root ONLY — never duplicate them into src/ or include/ (duplicated truth diverges)