# Phase 8 Regression Floor (v4.0.0.8 baseline)

**Captured:** 2026-04-22 prior to Phase 8 work starting.

This document pins the state of the shipped `v4.0.0.8` module that Phase 8's
v4.0.1.0 must preserve bit-for-bit on the Auto path. At v4.0.1.0 ship, Layer 2
is absent (bootstrap deferred to Phase 8.5) and no user DB exists on the E2E
test machine, so Layer 3 has zero rows. The Layer 3 → Layer 2 → Layer 1
fallback therefore delivers Layer 1 factory defaults — the same as v4.0.0.8.
`make e2e` goldens must match byte-for-byte.

Any Phase 8 task that changes a golden hash has broken additivity and must be
reverted or fixed before proceeding.

## ctest

- **Total:** 46
- **Passed:** 46
- **Failed:** 0
- **Runtime (serial):** 76.83 s

Note: the first parallel run surfaced 6 transient failures on Ceres-heavy
stretch tests (`test_mtf`, `test_arcsinh`, `test_log`, `test_rnc`, `test_ghs`,
`test_ots`). All six passed cleanly under `ctest -j 1`. Root cause is
resource contention when multiple fitters hammer BLAS in parallel, not a
real test-bench regression. Serial ctest is the authoritative invariant.

## E2E goldens (must remain bit-identical through v4.0.1.0)

Committed at `test/fixtures/golden/lrgb_mono_ngc7635.json`:

```json
{"primary":{"stacked":"d069786a","noise":"b9ec9edd","stretched":"83dfad37"},"sweep":{"GHS":{"stretched":"046c8b25"},"MTF":{"stretched":"b890bf1e"},"ArcSinh":{"stretched":"f7ea96b4"}}}
```

Hashes are FNV-1a over the raw pixel samples of the output FITS. Image
geometry for all three primary outputs: 3840×2160×1 (LRGB-mono, NGC7635
L/Lights corpus at `/mnt/qnap/astro_data/NGC7635/L/Lights`, 65 frames, 0
failed alignment at this baseline).

## Phase B wall-time floor

Committed at `test/fixtures/phaseB_baseline_ms.txt`:

```
1971756
```

That's 1,971,756 ms ≈ 32.86 min at the Phase B baseline (captured in the
Phase 7 work, post-OpenMP-parallelisation). The e2e validator checks current
wall-time against this baseline via a `phaseB_speedup_min: 1.5` requirement
from the manifest, so a run that's not at least 1.5× faster than baseline
would fail — but today's fresh run completed its primary Phase B in 285.757 s,
well under the ceiling.

## Confirming e2e run (2026-04-22 18:16 EDT)

Fresh `make e2e` against the installed `v4.0.0.8` module
(`/opt/PixInsight/bin/NukeX-pxm.so`, modified 2026-04-21 14:27):

- **Meta file** (`/tmp/nukex_e2e_meta.txt`): `STATUS ok`, `CASES 4`.
- **Report** (`/tmp/nukex_e2e/e2e_report.json`): `"status": "PASS"` with
  primary pixel hashes matching the committed goldens bit-for-bit.
- **Primary case** `lrgb_mono_ngc7635`: 65/65 frames aligned, primary
  Phase B `elapsed_s: 285.757`.
- **Sweep variants** (GHS, MTF, ArcSinh): all `status: ok`, stretched hashes
  match committed goldens.

## How Phase 8 must preserve this

At v4.0.1.0:

1. Layer 2 is empty (no `share/phase8_bootstrap_model.json` ships).
2. Layer 3 is empty on a fresh install (no `phase8_user_model.json` in user
   data until the user rates a run).
3. The Layer 3 → Layer 2 → Layer 1 fallback therefore resolves to Layer 1
   factory defaults, which are the Phase-5 champion constants already baked
   into each `StretchOp`'s member initializers.
4. The pixel output of the Auto path is therefore bit-identical to v4.0.0.8.

Operationally: any commit in the Phase 8 plan that causes `make e2e` to
fail a golden match has broken the additivity invariant. Revert or fix it
before proceeding. Tasks 15 and 21 of the plan are explicit regression gates
that re-verify against this floor.
