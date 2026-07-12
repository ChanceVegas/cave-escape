# PROGRESS.md — Single Source of Truth Across Sessions

Every session: read this first, update it last. If it isn't logged here, the next session doesn't know it happened.

## Current State
- Phase: pre-M0 (scaffolding only, no code yet)
- Builds: n/a
- Runs on hardware: n/a
- Measured FPS: n/a

## Next Up (in order)
1. Init git repo, first commit of the three docs, push to GitHub.
2. M0: PlatformIO project + board_config.h (pins from Elecrow wiki/schematic), screen-fill smoke test, flash.
3. USER decisions before M2: game concept (PLANNING #4) and input scheme (PLANNING #5).

## Known Issues / Risks
- HW-1 (RESOLVED 2026-07-11): board confirmed as CrowPanel 4.3" Basic, RGB parallel. Amazon listing's "SPI" was wrong.
- HW-2 (open risk): quad PSRAM bandwidth shared between panel refresh and drawing. Measure at M1; mitigations listed in CLAUDE.md.
- HW-3 (open risk): resistive single-touch limits control schemes. Decision pending (PLANNING #5).

## Decisions Made
- 2026-07-11: Endless structure for v1, built from hand-authored chunks chained procedurally — fixed levels become an ordered chunk list later. Planning phase COMPLETE; all blockers cleared for M0.
- 2026-07-11: Game = "Cave Escape", Pitfall-style hazard-avoidance side-scroller. v1 scope locked in PLANNING.md; vines/ladders/crouch deferred.
- 2026-07-11: Input = single-touch drag joystick (anchor + horizontal drag, flick-up jump). Abstracted so BLE gamepad can be added later. Roblox-style dual-touch ruled out — resistive panel is single-point.
- 2026-07-11: PlatformIO + Arduino framework + C++. Git for version control (no manual version file).
- 2026-07-11: Docs consolidated to 3 files (CLAUDE.md / PLANNING.md / PROGRESS.md); session monitoring = this file.
- 2026-07-11: PC emulator deferred (see PLANNING.md Open Decisions #3).

## Session Log (newest first)
### 2026-07-11 — Session 1 (cont.)
- Done: hardware confirmed (S3 N4R2, RGB parallel, NV3047, resistive touch, 4MB/2MB). CLAUDE.md and PLANNING.md updated. HW-1 closed; HW-2/HW-3 opened.
- Next: git init + M0 smoke test.

### 2026-07-11 — Session 1
- Done: project scaffolding created (CLAUDE.md, PLANNING.md, PROGRESS.md); toolchain chosen; reference article reviewed and scoped to 2.5D approach.
- Blocked: hardware confirmation (HW-1).
- Next: user confirms board → git init → M0.

## Changelog (human-readable milestones)
- v0.0.0 — 2026-07-11 — Project scaffolded. No code.
