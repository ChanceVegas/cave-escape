# PROGRESS.md — Single Source of Truth Across Sessions

Every session: read this first, update it last. If it isn't logged here, the next session doesn't know it happened.

## Current State
- Phase: M3b HW-VERIFIED ✅ (2026-07-15, tag M3B-R1)
- Builds: yes (espressif32@6.5.0; RAM 6.7%, Flash 60.7%)
- Runs on hardware: yes — camera ratchet follow, world-space collision, camera-driven parallax
- Measured FPS: 25.2–25.3 locked; render avg 34.5 ms (M3a compose cost ~0 vs M2b, verified)
- Heap: flat at 291,176 across extended run (no-alloc rule holding)

## Next Up (in order)
1. M3c — static spikes (authored in chunk data, AABB overlap = death).
   Measure render ms after; PERF gate: if headroom < ~2 ms, RLE compositor
   (documented reserve) BEFORE M3d.
2. M3d creature = stretch, may slip to M4.
3. M5 (queued): visible touch-feedback dot (anchor + drag-offset overlay at
   touch point; feedback only, NO input-logic change).

## Known Issues / Risks
- PERF-5 (watch, data 07-15): M3B-R1 render avg 35.4 ms idle / 34.7 ms moving
  (+~0.9 ms vs M3a idle) against 39.6 ms frame period -> ~4 ms headroom.
  fps floor 25 still met, heap flat (recycling clean). Decision recorded: NO
  renderer re-architecture on speculation; measure after M3c, pull RLE
  compositor reserve only if headroom < ~2 ms or M3d overruns.
- LEVEL-1 (open, minor): randomSeed(micros()) at boot -> near-deterministic
  chunk sequence across boots. Fine for testing; add touch-entropy at M4/M5.
- M3A-1 (RESOLVED 2026-07-15 by M3b): checkpoint respawn replaced camera-relative respawn; chunk starts guarantee floor. Was: pit respawn = camera.x + PLAYER_SPAWN_X with NO terrain
  check — could theoretically respawn over a pit (not with current test terrain:
  respawn point is always ≥130 px behind any pit the player can die in). Proper
  checkpoint respawn arrives at M3b; do not band-aid this.
- BUILD-1 (RESOLVED 2026-07-14): user allowlisted PlatformIO registry domains;
  container `pio run` now works. M3a compiled in-container before hw test.
- FEEL-2 (open, M3b design input): with run-up, ALL pits 60/70/80 px cleared
  without issue (M3A-R1) — vs 60 px "tight" at M2b from a short floor. Effective
  clearance is speed-dependent; ~86 px ceiling assumes full run-up. Chunk
  authoring rule: wide pits require preceding flat run-up.
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
### 2026-07-15 — Session 7 (cont. 2) — INPUT-4 gate result: not reproducible
- User ran deliberate mid-drag flick jumps: working, no adverse behavior.
- INPUT-4 rework CANCELLED per gate (no rework without reproducible failure).
  Flick jump retained. Visible touch dot demoted to M5 as feedback-only
  overlay. Original complaint likely an artifact of cramped M2b test terrain.
- Commit: docs(input): cancel INPUT-4 rework (not reproducible), dot to M5

### 2026-07-15 — Session 7 (cont.) — M3b hardware verification: PASSED
- M3B-R1 on device: endless terrain, no seam artifacts, good chunk variety,
  death -> checkpoint respawn correct with camera snap-back, dist monotonic
  through death (plateau, no dip), fps 25.2-25.4, heap flat 290,544 after
  ~460 B first-seconds settle. M3A-1 retirement confirmed on hardware.
- PERF-5 opened: render 34.7-35.4 ms, ~4 ms headroom. RLE reserve stays the
  contingency; no preemptive rework (user raised resource concern; resolved
  as "measure at M3c, lever if needed").
- INPUT-4 challenged: user reports "input was fine" in play — contradicts the
  jump-while-moving complaint that motivated INPUT-4. Gate added: reproduce
  (10x mid-drag flick fail count) before any input rework.
- Commit: docs(m3b): hw verification passed, open PERF-5, gate INPUT-4 on repro

### 2026-07-15 — Session 7 (M3b: chunk system)
- DECISION AMENDED (camera, 07-14): ratchet gets ONE exception — camera::snapTo
  (checkpoint) on respawn only. Death can leave the checkpoint behind the
  camera's left edge; Pitfall-style snap-back. snapTo collapses interp state
  (prev=curr=drawX) so there's no one-frame sweep.
- Done (M3B-R1): src/level.h/.cpp — chunk format {width, solids[]} chunk-relative,
  4-chunk flash library (flat / pit60 / ledge / pit80-with-320px-run-up per
  FEEL-2), ring of active chunks (LEVEL_MAX_ACTIVE 6), fixed solids pool
  (LEVEL_SOLIDS_POOL 32) re-registered via physics::setSolids on window change,
  recycle keyed on CHECKPOINT (not camera) so respawn terrain always exists,
  prefetch LEVEL_PREFETCH_PX 160 past view; checkpoint = last chunk boundary
  crossed; entities::respawn -> checkpoint + PLAYER_SPAWN_X (retires M3A-1);
  distance score = monotonic max camera X (bug caught in-session: raw camera.x
  would dip at respawn snap), printed in serial stats ("dist"); main.cpp test
  terrain DELETED, tick order input -> entities -> camera -> level.
- Authoring rules encoded in level.cpp header comment: every chunk starts with
  >=96 px flat floor at y=240; wide pits carry in-chunk run-up.
- Compiled in container (pio run SUCCESS). Hardware verification = next step.
- Known minor: randomSeed(micros()) at boot is low-entropy — chunk sequence may
  repeat across boots. Cosmetic for now; revisit if it bothers testing (LEVEL-1).
- Commit: feat(m3b): chunk system — procedural chaining, checkpoint respawn, distance score

### 2026-07-14 — Session 6 (M3a verification: gates passed)
- Repo forensics: m3-world existed on remote but at main's tip — Session 5 patch
  was never applied (git am step missed). Patch re-applied; renderer.cpp hunk was
  corrupted in transit and reconstructed from hunk header counts (verified by build).
- BUILD-1 closed: registry allowlist works; `pio run` SUCCESS in container.
- Stale-flash incident: device showed M2b-r1 banner → traced to the un-applied
  patch, not a flash failure. Transfer/verify protocol caught it as designed.
- M3a HW-VERIFIED (M3A-R1): camera follow smooth, ratchet holds, ledges A/B
  reachable, respawn correct (incl. pit 3 / M3A-1 scenario), fps 25.2–25.3,
  render 34.5 ms, heap flat 291,176.
- FEEL-2 opened: all pits (60/70/80) clearable WITH run-up; clearance is
  speed-dependent. M3b chunk-authoring constraint recorded.
- User requested on-screen joypad → multi-zone rejected (single-touch panel).
  Root complaint: jump-while-moving unreliable = diagonal flick at 25 Hz
  sampling (INPUT-2 territory). Design agreed → INPUT-4 (see Next Up):
  fixed visible stick + position-based jump, overlay (no viewport shrink).
  Reserved-strip variant considered and dropped (would force re-authoring
  terrain Y for 208 px playfield before M3b).
- M3A-1 addendum: repeated falls off world's end (x>1680, not a real pit) —
  respawn always landed on floor, NO death loop. Terrain-edge "impossible pit"
  is expected throwaway-terrain behavior; resolves at M3b.
- Commit: docs(m3a): hw verification, close BUILD-1, open FEEL-2, spec INPUT-4

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