# NukeX v4 — Changelog

## v4.0.0.6 — 2026-04-20

### Added
- **Alignment diagnostics exposed on PJSR.** Two new read-only process parameters — `nFramesProcessed` and `nFramesFailedAlignment` — are populated from the stacking result after `executeGlobal()`. Scripted callers can read them directly (`P.nFramesFailedAlignment`) instead of regex-parsing the Process Console log. The E2E harness (`tools/validate_e2e.js`) now fails a case if any frame failed alignment, rather than relying on wall-time and execute-ok alone.
- **Informative per-frame alignment log.** The Process Console used to emit `aligned (200 stars)` for every frame whether alignment succeeded or failed — a real UX trap that contributed to the NGC7635 alignment-failure regression going unnoticed before the v4.0.0.5 investigation. It now emits the actual outcome, inlier count, and sub-pixel RMS, e.g. `aligned: ok (stars=200, inliers=83, rms=0.088 px)` or `aligned: FAILED (stars=200, inliers=0, rms=0.000 px)`.

### Build / tooling
- **`make sign` and `make package`** now work out of the box: `tools/release.sh` drives the full mechanical post-build pipeline (sign `.so`, stage into `repository/bin/`, create date-stamped tarball, compute SHA-1, patch `updates.xri` with new `fileName`/`sha1`/`releaseDate`, re-sign the XRI). The previous `cmake/PackageAndSign.cmake` was never included and had three latent bugs (wrong target name, wrong tarball filename, no SHA-1 update) that would have surfaced the first time anyone typed `make package`.
- **`make e2e` / `make e2e-regen` hardened.** Invocation moved to `tools/run_e2e.sh` after the initial CMake inlining hit a `$$`-escape bug where `$NUKEX_E2E_REGEN` expanded to `$$` (shell PID) at configure time, corrupting the manifest path. The driver now also wipes the manifest's `output_root` before invoking PI as a belt-and-braces against any stale FITS triggering an overwrite prompt.
- **Silent-overwrite of harness FITS outputs.** `ImageWindow.saveAs` was being called with `verifyOverwrite=true`, which pops a modal "replace?" dialog in headless `--automation-mode`. Changed to `false`.

## v4.0.0.5 — 2026-04-20

### Fixed
- **Alignment: replaced the positional nearest-neighbour star matcher** with Groth (1986) / Valdes (1995) triangle similarity matching (K=100, canonical vertex ordering, sorted-r1 window index, vote-based correspondences). On real multi-hour imaging sessions with cumulative tracking drift >> 5 px, the old matcher reported "aligned (N stars)" but produced 0 usable matches in nearly all frames — the E2E baseline on NGC7635 L/Lights (65 frames, 3.5 hr) dropped from 61/65 failed alignments to **0/65**.
- **Phase B: Ceres solver crash on pathological voxels.** `StudentTNLL::Evaluate` and `ContaminationNLL::Evaluate` now return `false` instead of letting a non-finite cost/gradient reach `ceres/line_search.cc:705`'s CHECK-fail (which aborted the process after ~26 min on a full stack).

### Performance
- **Phase B per-voxel Ceres fitting now parallelised across CPU cores** (OpenMP `parallel for` with dynamic-256 scheduling). Each voxel's fit is independent: `ModelSelector::select` allocates its fitters as stack locals, so there is no shared mutable state to synchronise.
  - **Measured: 9.35× Phase B speedup** on NGC7635 L/Lights (32.86 min → 3.51 min). Total stack wall-time 34 min → 4.6 min. Ship bar was 1.5×.

### Observability
- **`NukeXProgress` emits three sideband progress channels** so headless harnesses (and curious users watching a long stack) can see liveness even during silent Ceres compute:
  1. `std::cerr` line per progress event (reaches shell via `2>&1 | tee`).
  2. Append log at `/tmp/nukex_progress.log` (`tail -f` from any terminal).
  3. 10-second heartbeat watchdog thread writes `/tmp/nukex_heartbeat.txt` with elapsed seconds + last phase/detail, proving liveness even when no callbacks fire.

### Testing
- **E2E validation harness** (`tools/validate_e2e.js` + `make e2e`). Runs the full pipeline against a manifest of test stacks, verifies execute-ok, wall-time budgets, 0-alignment-failure, and bitwise regression via FNV-1a hash of raw pixel samples (not file SHA — avoids the FITS write-time-stamp problem). Per-case golden hashes in `test/fixtures/golden/`. Dropdown sweep (`GHS`, `MTF`, `ArcSinh`) verified to produce distinct stretched outputs.
- **Baseline fixture:** `test/fixtures/phaseB_baseline_ms.txt` captures the authoritative Phase B floor (**1,971,756 ms** on NGC7635, pre-OpenMP) that B6 measures the post-optimisation run against.
- New `test_alignment_diag.cpp` (tag `[.diag]`, opt-in) loads real NGC7635 frames and sweeps matcher parameters for future debugging. Existing homography tests updated: 5×4 grid and collinear-point layouts replaced with deterministic-random positions (grids had too many congruent triangles for triangle-matching descriptors to discriminate).

### Build / plumbing
- `tools/capture_baseline.js` rewritten for PI `--automation-mode` quirks discovered while getting headless harnesses to work: `File.environmentVariable` does not see shell env (use `jsArguments`), `Console.writeln` does not reach shell stdout (wrap in `Console.beginLog()`/`endLog()`), enum values sharing an integer value collide on the prototype (use integer literals). See memory `reference_pjsr_automation_quirks.md`.
- `docs/superpowers/plans/2026-04-19-phase7-perf-findings.md` documents the flamegraph + selected optimisation + actual 9.35× speedup.
- `.gitignore` covers `Testing/`, `repository/bin/`, `build_profile/`, `.claude/`.

## v4.0.0.4 — 2026-04-20

### Added
- **Stretch pipeline wired to output.** New `NukeX_stretched` window opens alongside `NukeX_stacked` and `NukeX_noise`. Previously the module stacked but never applied the configured stretch.
- **Metadata-driven Auto selection.** `primaryStretch` defaults to `Auto`; the module reads FITS `FILTER`/`BAYERPAT`/`NAXIS3` from the first light, classifies into one of LRGB-mono / LRGB-color / Bayer-RGB / Narrowband, and picks the Phase-5 champion curve (VeraLux today across all classes). The classification is logged to the Console.
- **Finishing stretch slot.** New `finishingStretch` parameter; only `None` is enrolled this release. SAS / OTS / Photometric are deferred to later phases per their own dedicated brainstorms.

### Changed
- **Schema break:** the flat 10-entry `stretchType` enum and `autoStretch` bool are replaced by `primaryStretch` (Auto + 7 curves: VeraLux, GHS, MTF, ArcSinh, Log, Lupton, CLAHE) and `finishingStretch` (None only). Saved projects referencing the old IDs revert to defaults.

### Deferred to v4.0.0.5
- Phase B Ceres-fitting perf optimization (>=1.5x target on real workload).
- PJSR E2E validation harness (`make e2e`) with bitwise regression across a 4-stack corpus.

### Build
- Added optional `NUKEX_PROFILING=ON` CMake flag that adds `-fno-omit-frame-pointer` to support perf flamegraph capture for the v4.0.0.5 Phase B work.
- Added `#include <chrono>` Phase-B wall-time instrumentation in the stacker; logs `Phase B: <n> ms` via the progress observer.
