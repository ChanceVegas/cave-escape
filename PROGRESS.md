# PROGRESS.md — Single Source of Truth Across Sessions

Every session: read this first, update it last. If it isn't logged here, the next session doesn't know it happened.

## Current State
- Phase: M3a CODE DONE (2026-07-14) — camera scroll on branch m3-world; NOT yet
  compiled or hw-verified (session container could not reach PlatformIO registry).
- Builds: UNVERIFIED for M3a — first `pio run` on target machine is the compile gate.
  (M2 close on main: builds yes, espressif32@6.5.0.)
- Runs on hardware: M2 tip yes; M3a pending.
- Measured FPS: 25.3 locked; render avg 34.5 ms (M2b). M3a adds ~zero compose cost
  (few float ops + off-screen rect skip) — verify, don't assume.
- Heap: stable at 291,184 at M2 (no-alloc rule holding)

## Next Up (in order)
1. M3a hardware verify (protocol per CLAUDE.md rule 8, xfer-tag M3A-R1): build,
   flash, then test — camera follows at CAM_ANCHOR_X=160, back-edge clamp works,
   scroll judder-free, parallax tracks camera, pits 60/70/80 px (FEEL-1 data:
   which are clearable?), ledges A/B reachable, pit respawn lands safely ahead
   of camera, fps ≥ 25 / render ≤ ~35 ms / heap flat.
2. M3b — chunk system (scope in PLANNING.md M3 breakdown).
3. M3c — static spikes. (M3d creature = stretch, may slip to M4.)

## Known Issues / Risks
- M3A-1 (open, temporary): pit respawn = camera.x + PLAYER_SPAWN_X with NO terrain
  check — could theoretically respawn over a pit (not with current test terrain:
  respawn point is always ≥130 px behind any pit the player can die in). Proper
  checkpoint respawn arrives at M3b; do not band-aid this.
- BUILD-1 (open, this session): container couldn't reach PlatformIO registry
  (api.registry.platformio.org / dl.platformio.org not in network allowlist).
  M3a committed compile-UNVERIFIED on branch. User may allowlist those domains
  for future sessions.
- FEEL-1 (open, M3 design input): 60 px test pit was clearable but tight. Causes to
  separate at M3: constants tuning vs ~40 ms worst-case flick latency (25 Hz touch
  sampling, INPUT-2). Do NOT tune jump constants against throwaway terrain.
- PROC-1 (lesson, closed): M2b code sat uncommitted in a session container and was
  nearly rewritten from scratch next session. Rule: session end = commit, even wip(...)
  on a branch. Uncommitted work does not exist.
- PERF-4 REVISED 2026-07-14: render avg 34.5 ms → core-1 headroom for ALL M3 logic
  ≈ 4 ms/frame. RLE compositor remains the pre-planned overdraft response.
- PERF-1 (open, managed): 27.2 fps is only 2.2 above floor; game logic at M2/M3 shares core 1 with compose. If M3 dips below 25: RLE-span compositor is the pre-planned response (PLANNING.md).
- PERF-2 (accepted): core 0 idle-task watchdog disabled (disableCore0WDT) — push task saturates core 0 by design. Trade-off: no hang detection on core 0.
- PERF-3 (open, minor): render is frequency-locked to panel but not PHASE-locked (LovyanGFX exposes no vsync event). A faint static tear line may sit at a boot-random row; reset re-rolls it. Proper fix = esp_lcd vsync callback — documented reason to migrate off LovyanGFX's RGB path if it ever matters.
- PERF-4 (open, managed): core-1 logic budget ≈ 6 ms/frame at M2/M3 (39.6 ms period − 32 compose − 1 updates). Overdraft response: RLE compositor. M2a data point: touch polling at 60 Hz ticks costs nothing measurable (fps steady 25.3 while touching).
- INPUT-1 (watch): possible ghost jumps under aggressive diagonal scrubbing — one uncontrolled sighting; 0/5 in controlled scrubs at M2a; ZERO during M2b gameplay testing incl. deliberate diagonal scrubbing (2026-07-14). Ready fix if it ever recurs: require |dY| > |dX| for the velocity trigger.
- INPUT-2 (design fact): effective touch sampling is ~25 Hz (one real sample per frame; the 2-3 logic ticks per frame read identical coordinates). Jump detection is designed around this. A 120 Hz sampler task is the documented upgrade if finer gestures are ever needed.
- HW-2 (RESOLVED as understood): "PSRAM bandwidth" risk was really shared flash+PSRAM memory-bus saturation. Measured, mitigated (pclk 4 MHz, flash layers, row-skip, dual-core), documented in PLANNING.md.
- M0-3 (RESOLVED): LovyanGFX Bus_RGB/Panel_RGB need explicit esp32s3 platform header includes.
- M0-2 (note): platform pinned espressif32@6.5.0; LovyanGFX ^1.1.16.
- HW-3 (resolved note): touch XPT2046 shares SPI with TF slot; touch CS = GPIO0 strapping pin.

## Decisions Made
- 2026-07-13: Jump detection = RAW y (not EMA-smoothed) with dual trigger — displacement above anchor (slow slides) OR per-sample upward velocity (fast snaps) — plus release-edge displacement check using second-to-last sample (final liftoff sample is garbage-prone). Verified 5/5 jumps, 0/5 false positives on hardware.
- 2026-07-13: Hardware test protocol adopted after losing ~3 test cycles to stale files and a broken test harness: (1) transfer code via clipboard (pbpaste > file), never downloads; (2) grep for a version tag after every transfer; (3) confirm upload SUCCESS; (4) RST + boot-banner check before testing; (5) test-harness prints live at the event fire-site, never polled across ticks.
- 2026-07-12: M1 gate — target revised 30 → 25 fps floor (Option B) instead of RLE compositor now (Option C). Priority: reach playable game; C deferred as documented reserve lever. USER decision.
- 2026-07-12: pclk 8 → 5 → 4 MHz (measured +2.4 then +1.8 fps; panel verified OK by eye at each step).
- 2026-07-12: layers = flash-resident 8-bit indexed + palette, generated by tools/gen_layers.py; custom row compositor with per-row content flags.
- 2026-07-12: renderer = dual-core band pipeline (compose core 1, push core 0, 2 ping-pong SRAM bands, FreeRTOS queues).
- 2026-07-12: render interpolation (prev/curr + alpha) added for parallax.
- 2026-07-12: frame limiter locks render to exact panel period (39,590 us, derived from pclk+timings). Root cause of residual jerk was the ~2 Hz beat between 27.3 fps render and 25.26 Hz scanout — interpolation alone could not fix a display-sampling beat. Verified fixed on hardware.
- 2026-07-12: Graphics lib = LovyanGFX (Open Decision #2 closed at M0).
- 2026-07-11: endless chunk structure; "Cave Escape" concept; single-touch drag joystick; PlatformIO+Arduino+C++; 3-doc system; PC emulator deferred.

## Session Log (newest first)
### 2026-07-14 — Session 5 (M3 breakdown + M3a: camera scroll)
- M3 broken into M3a/b/c(+d stretch) — user approved; user ruled pits COUNT as one
  of the 2 hazard types → M3 hazard scope = pits + static spikes; creature = stretch.
- DECISION: camera is player-driven with one-way forward ratchet (never retreats),
  not auto-scroll. Rationale: Pitfall-style agency per PLANNING; ratchet bounds
  chunk recycling at M3b. Back-edge clamp replaces M2b screen-edge clamp.
- Done (branch m3-world): src/camera.h/.cpp (ratchet follow, prev/curr interp);
  entities → world space (camera clamp, world→screen in composeBand with off-screen
  rect skip, respawn relative to camera — see M3A-1); parallax now camera-driven
  (self-scroll + SCROLL_SPEED_PX_S deleted, update() removed); renderer beginRender
  order = camera FIRST; config: +CAM_ANCHOR_X 160; main: 3.5-screen test terrain
  (pits 60/70/80 px for FEEL-1 data, two reachable 40 px ledges), tick order
  input → entities → camera, tag M3A-R1.
- Static checks only: no stale refs (SCROLL_SPEED/parallax::update/worldX gone);
  compile NOT verified (BUILD-1). Hardware verification = next session or user.
- Commit: feat(m3a): camera scroll — ratchet follow, world-space entities, camera-driven parallax

### 2026-07-14 — Session 4 (M2b: player physics + repo recovery)
- Done: physics.h/.cpp (gravity, axis-separated AABB vs static solids, tunnel-proof
  by MAX_FALL clamp), entities.h/.cpp (player: input consumption per tick, prev/curr
  render interpolation, pit respawn), renderer wired to entities::composeBand (M1 test
  sprites deleted), config.h player constants (apex 50 px / 0.33 s derivation
  documented), M2b test terrain in main.cpp (floor + 60 px pit + head-bonk ledge).
- Hardware-verified per protocol (xfer-tag M2B-R1): gravity/landing clean, full-range
  move + edge clamp, jump apex correct, airborne flicks ignored, pit respawn without
  interp streak, head-bonk correct AND unreachable ledge confirmed unreachable,
  pit clearable at full stick (tight — logged FEEL-1), no ghost jumps (INPUT-1),
  fps 25.3 / render 34.5 ms avg / heap flat 291,184.
- Repo recovery en route: stale doc copies attached to session came from OUTSIDE the
  repo (repo docs were current all along); duplicate docs in include/ removed (were
  byte-identical, but duplicated truth invites divergence); wrong-branch push and a
  GitHub-merge/local-merge history divergence untangled (pull --rebase). Rule going
  forward: solo project merges happen locally, not via GitHub UI, OR always pull main
  first. .pio/ build artifacts found committed (15 MB elf) — purged this session via
  .gitignore + git rm -r --cached.
- Process failure logged honestly (PROC-1): this M2b code was written in a prior
  session and never committed; it survived only by container luck and was one answer
  away from being rewritten. Session end = commit, always.
- Commit: feat(m2b): player physics + collision + placeholder render, hardware-verified

### 2026-07-13 — Session 3 (M2a: input)
- Done: input module (src/input.h/.cpp) — drag-joystick state machine on XPT2046 via LovyanGFX touch (hw config in display.cpp, gestures in input.cpp, backend-abstract per PLANNING #5). moveX verified full-range both directions with correct polarity and quiet dead zone; jump verified 5/5 (fast + slow flicks), 0/5 false positives on horizontal scrubs.
- Bug found & fixed IN THE TEST HARNESS, not the gesture code: jumpPressed is a one-tick edge, but the main.cpp harness polled it after the 2-3-tick batch — only ~1-in-3 jumps survived to print. All earlier "flaky jump" results (4/6, 3/6, 0/6) were this measurement bug plus stale-file flashes. Raw-trace instrumentation (T0/S/T1 sample logging) is what isolated it.
- Process failure logged honestly: repeated file-transfer failures (downloads landing wrong/stale, heredoc paste mangled by zsh) cost ~3 flash cycles debugging ghosts. Fixed with the hardware test protocol (see Decisions).
- Repo layout repaired: config.h returned to include/ after drifting into src/.
- Commit: feat(m2a): input module — drag joystick + flick jump, hardware-verified

### 2026-07-12 — Session 2 (M1)
- fps ledger, every step measured on hardware: 11.4 (baseline PSRAM layers) → 13.8 (pclk 5 MHz) → 17.5 (flash layers + row compositor) → 21.7 (row-skip flags) → 25.4 (dual-core pipeline) → 27.2 (pclk 4 MHz) → 25.3 locked (frame limiter; trades 2 fps for judder-free scroll).
- Root cause identified: shared flash/PSRAM memory bus saturation (compose reads ∥ push writes ∥ panel scanout). Dual-core stage times rose when parallel — bus contention, not sync overhead.
- Prediction miss logged: flash layers predicted <10 ms compose, measured 31 (cache thrash + per-pixel loop cost underestimated). Subsequent estimates corrected.
- M1 gate: user chose Option B (25 fps floor, proceed to M2) over Option C (RLE compositor now).
- Close-out: instrumentation stripped; TARGET_FPS 30→25 in config.h; parallax render interpolation added; PLANNING.md budget + milestones + module map updated.
- Branch: work on m1-perf; merge to main at close.

### 2026-07-12 — Session 2 (M0)
- M0 COMPLETE: skeleton built/flashed/verified; RGBW cycle correct; PSRAM 2 MB detected. LovyanGFX adopted. fillScreen 30 ms datum foreshadowed M1 fight.

### 2026-07-11 — Session 1
- Scaffolding, toolchain, hardware confirmed (S3 N4R2, RGB parallel, NV3047), concept + input scheme decided, repo initialized and pushed.

## Changelog (human-readable milestones)
- v0.3.0 — 2026-07-14 — M2: playable player. Drag-joystick move + flick jump, gravity,
  AABB collision, pit respawn, on hardware at 25.3 fps.
- v0.2.0 — 2026-07-12 — M1: render skeleton on hardware. 25.3 fps locked to panel, judder-free, 3 parallax layers + 10 sprites, dual-core pipeline. Target revised to 25 fps floor.
- v0.1.0 — 2026-07-12 — M0: toolchain proven on hardware.
- v0.0.0 — 2026-07-11 — Project scaffolded. No code.