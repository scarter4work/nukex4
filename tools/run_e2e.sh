#!/usr/bin/env bash
#
# NukeX v4 — E2E harness driver.
#
# Usage:
#   tools/run_e2e.sh              # verify against committed goldens
#   tools/run_e2e.sh regen        # refresh goldens from current binary
#
# Invoked by `make e2e`.  Kept in its own script because CMake's COMMAND
# substitution does not handle nested `$$`/bash-c quoting cleanly.

set -eu

REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
MANIFEST="${REPO}/test/fixtures/e2e_manifest.json"
BUILD_DIR="${NUKEX_BUILD_DIR:-${REPO}/build}"
E2E_LOG="${BUILD_DIR}/e2e.log"

REGEN_ARG=""
if [ "${1:-}" = "regen" ] || [ "${NUKEX_E2E_REGEN:-}" = "1" ]; then
    REGEN_ARG=",regen=1"
    echo "NukeX E2E: regen mode — goldens in test/fixtures/golden/ WILL be rewritten."
fi

rm -f /tmp/nukex_e2e_meta.txt /tmp/nukex_e2e_console.log

/opt/PixInsight/bin/PixInsight.sh --automation-mode --force-exit \
    "-r=${REPO}/tools/validate_e2e.js,manifest=${MANIFEST}${REGEN_ARG}" \
    2>&1 | tee "${E2E_LOG}"

echo ""
echo "========================================================================"
if [ -f /tmp/nukex_e2e_meta.txt ]; then
    cat /tmp/nukex_e2e_meta.txt
    STATUS="$(grep '^STATUS ' /tmp/nukex_e2e_meta.txt | awk '{print $2}')"
else
    echo "STATUS fail"
    echo "REASON harness did not write meta file"
    STATUS="fail"
fi
echo "========================================================================"

test "${STATUS}" = "ok"
