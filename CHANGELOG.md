# NukeX v4 — Changelog

## v4.0.0.8 — 2026-04-21

Robustness + observability release driven by the first real-data stack on
shipped v4.0.0.7 (M27 Bayer-RGB, 2026-04-20).  Two concrete user-reported
gaps closed; pixel output on well-exposed frames is bit-identical to
v4.0.0.7 (E2E goldens unchanged).

### Added
- **Saturation guard in StarDetector.**  Before running
  `find_local_maxima`, the detector measures the fraction of pixels at
  or above `saturation_level` on a 4× decimated sample of the frame.
  If that fraction exceeds `saturation_reject_fraction` (default 0.5),
  the frame is rejected up front with an empty catalog.  Previously, a
  dawn-twilight frame where a majority of pixels were clipped sent the
  O(n²) exclusion-radius filter chasing a plateau of "local maxima" and
  hung `StarDetector::find_local_maxima` for 5+ minutes per frame.
  New unit tests exercise the hang on 1200×1200 (10.4 s → < 50 ms) and
  verify normal frames with a handful of saturated-core stars still
  detect correctly.
- **Distinctive log line for blown-out frames.**  In the stacking
  engine, a pre-flight saturation check replaces the generic
  `aligned: FAILED (stars=0)` line with
  `aligned: SKIPPED (blown out — X.X% pixels at saturation)` so a user
  watching the Process Console can tell a truly unusable frame from a
  detection misfire.
- **Fit-loop heartbeat in Phase B.**  The OpenMP-parallelised Ceres
  distribution-fitting loop (`gpu_executor.cpp:437`) now emits a
  `fitted K/N voxels (Ts)` line every 2 s from thread 0.  Previously
  the Process Console was silent for the entire 3–4 min single-batch
  fit between kernel 2 and kernel 3, which made long stacks look
  frozen.  The emission is rate-limited via an atomic compare-exchange
  on a shared "last report time" so the observer's mutex never lands
  on the hot compute path; unit tests (`test_fit_heartbeat`) lock in
  the thread-0 gate, the interval gate, and the concurrent-done count.

### Implementation notes
- New public `StarDetector::saturation_fraction(image, level)` helper
  backs both the detector's own guard and the stacking-engine log line
  so the fraction is computed from a single definition.
- New `FitHeartbeat` utility class in `src/lib/gpu` keeps the
  rate-limit / emission logic testable without a full Phase B harness.

## v4.0.0.7 — 2026-04-20

Polish pass on top of v4.0.0.6.  No compute-path changes — pixel output
is bit-identical to v4.0.0.6 (E2E golden hashes unchanged).  The
delta is entirely user-facing clarity, scripting ergonomics, and test
hygiene.

### Added
- **WARNING / CRITICAL** messages in the Process Console on low
  alignment success rate:  > 50 % failed fires `** CRITICAL **` with
  a remediation hint; 10–50 % fires `** WARNING **`; ≤ 10 % stays
  quiet (some drift / clouded frames is normal).
- **Auto-selector rationale** now names the FITS values that drove
  the classification:  `Auto: classified as LRGB-mono (FITS
  FILTER='L', BAYERPAT='', NAXIS3=1) -> VeraLux`.
- **Tooltips** on every NukeX interface control: Primary / Finishing
  Stretch dropdowns + labels, Enable GPU checkbox, frame-list tree
  boxes, and all Add / Remove / Clear / Toggle All buttons.
- **Validate()** now refuses to enable the Execute button when
  `cacheDirectory` is unset-but-non-default-and-unwritable, and warns
  at ExecuteGlobal time when fewer than 5 frames are enabled (Phase B's
  Ceres fitters need n ≥ 5 to converge).
- **FITS header provenance** on all output windows (`NukeX_stacked`,
  `NukeX_noise`, `NukeX_stretched`):  `CREATOR`, `NUKEXVER`,
  `NUKEXIMG`, `NFRAMES`, `NFAILALN`, `HISTORY` entries.  The stretched
  window additionally carries `PRIMSTR` / `FINSTR` with the applied
  curve names.

### Changed
- **Filter classifier** now case-folds the `FILTER` keyword before the
  narrowband-name lookup.  Previously depended on the FITS reader's
  uppercase normalisation; now robust to any casing upstream might
  produce.
- **ctest** runs all 45 tests in 94 s with no `-E` exclusion list.
  Heavy exploratory / integration / visual / sweep cases are re-tagged
  as opt-in Catch2 dot-prefixed tags (`[.integration]` etc.) so they
  skip by default but are still callable via the corresponding filter.

### Fixed
- **`tools/run_e2e.sh` and `tools/release.sh`** now use `set -o
  pipefail` so `PixInsight.sh … | tee file` surfaces a PI crash
  instead of silently reporting success.  `run_e2e.sh` also wraps PI
  in `timeout --kill-after=30s ${NUKEX_E2E_TIMEOUT:-3600}`.

### Testing
- Golden-hash E2E regression now covers the dropdown-sweep variants
  (`GHS`, `MTF`, `ArcSinh`) as well as the primary path, so a
  per-curve regression in (say) MTF while VeraLux stays stable is
  caught bitwise.
- New regression tests for the `isfinite` guard in `StudentTNLL::
  Evaluate` and `ContaminationNLL::Evaluate`:  extreme-outlier and
  all-identical-samples inputs that used to CHECK-abort the process
  at `ceres/line_search.cc:705` now exit the fitter cleanly.
- Thirteen mixed-case narrowband filter names (`Ha`, `ha`, `hA`,
  `h-alpha`, `Narrowband`, `nb`, …) lock in the defensive case-fold.

### Plumbing
- Module version macros centralised in `src/module/NukeXVersion.h`
  (previously only in `NukeXModule.cpp`, so any other translation unit
  that wanted them had to hardcode).  Version bumps now require
  editing one place.

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
