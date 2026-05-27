#!/usr/bin/env bash
# scripts/test-sim.sh
# Aleph Simulator — local test runner
#
# Builds the headless beekeep simulator, generates fixtures (if needed),
# runs the test harness, and exits with 0 on pass or non-zero on failure.
#
# Usage:
#   ./scripts/test-sim.sh               # normal run
#   ./scripts/test-sim.sh -v            # verbose assertions
#   ./scripts/test-sim.sh --json-out /tmp/results.json
#   ./scripts/test-sim.sh --skip-build  # run harness with existing binaries
#
# Prerequisites (macOS):
#   brew install jansson pkg-config
# Prerequisites (Linux):
#   sudo apt-get install libjansson-dev pkg-config

set -euo pipefail

# --------------------------------------------------------------------------
# Colour helpers
# --------------------------------------------------------------------------
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'  # reset

info()    { echo -e "${CYAN}[test-sim]${NC} $*"; }
success() { echo -e "${GREEN}[test-sim] ✓${NC} $*"; }
warn()    { echo -e "${YELLOW}[test-sim] !${NC} $*"; }
error()   { echo -e "${RED}[test-sim] ✗${NC} $*" >&2; }

# --------------------------------------------------------------------------
# Defaults
# --------------------------------------------------------------------------
SKIP_BUILD=0
VERBOSE_FLAG=""
JSON_FLAG=""
JSON_OUT=""

# --------------------------------------------------------------------------
# Parse arguments
# --------------------------------------------------------------------------
while [[ $# -gt 0 ]]; do
    case "$1" in
        -v|--verbose)
            VERBOSE_FLAG="-v"
            shift ;;
        --json-out)
            JSON_OUT="$2"
            JSON_FLAG="--json-out $2"
            shift 2 ;;
        --skip-build)
            SKIP_BUILD=1
            shift ;;
        -h|--help)
            echo "Usage: $0 [-v] [--json-out PATH] [--skip-build]"
            exit 0 ;;
        *)
            error "Unknown argument: $1"
            exit 1 ;;
    esac
done

# --------------------------------------------------------------------------
# Locate the beekeep directory relative to this script
# --------------------------------------------------------------------------
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
BEEKEEP_DIR="${REPO_ROOT}/utils/beekeep"
FIXTURES_DIR="${BEEKEEP_DIR}/test/fixtures"

if [[ ! -d "${BEEKEEP_DIR}" ]]; then
    error "Cannot find utils/beekeep at ${BEEKEEP_DIR}"
    exit 1
fi

info "Repository root : ${REPO_ROOT}"
info "beekeep dir     : ${BEEKEEP_DIR}"
info "Fixtures dir    : ${FIXTURES_DIR}"

# --------------------------------------------------------------------------
# Step 1: Build headless simulator
# --------------------------------------------------------------------------
if [[ "${SKIP_BUILD}" -eq 0 ]]; then
    info "Step 1/4: Building headless simulator..."
    pushd "${BEEKEEP_DIR}" > /dev/null
    if ! make HEADLESS=1 MOCK_BFIN=1 2>&1; then
        error "Headless build failed."
        popd > /dev/null
        exit 1
    fi
    success "Headless build complete."
    popd > /dev/null
else
    warn "Skipping build (--skip-build)"
fi

# --------------------------------------------------------------------------
# Step 2: Build + run fixture generator (if fixtures are missing)
# --------------------------------------------------------------------------
if [[ ! -f "${FIXTURES_DIR}/empty.scn" ]]; then
    info "Step 2/4: Generating test fixtures (empty.scn not found)..."
    pushd "${BEEKEEP_DIR}" > /dev/null
    if ! make HEADLESS=1 MOCK_BFIN=1 _make_fixtures 2>&1; then
        error "Fixture generator build failed."
        popd > /dev/null
        exit 1
    fi
    if ! ./make_fixtures 2>&1; then
        error "Fixture generation failed."
        popd > /dev/null
        exit 1
    fi
    success "Fixtures generated."
    popd > /dev/null
else
    info "Step 2/4: Fixtures already present — skipping generation."
fi

# --------------------------------------------------------------------------
# Step 3: Build test harness
# --------------------------------------------------------------------------
if [[ "${SKIP_BUILD}" -eq 0 ]]; then
    info "Step 3/4: Building test harness..."
    pushd "${BEEKEEP_DIR}" > /dev/null
    if ! make HEADLESS=1 MOCK_BFIN=1 _test_harness 2>&1; then
        error "Test harness build failed."
        popd > /dev/null
        exit 1
    fi
    success "Test harness built."
    popd > /dev/null
else
    warn "Skipping harness build (--skip-build)"
fi

# Verify binary exists
if [[ ! -x "${BEEKEEP_DIR}/beekeep-test" ]]; then
    error "beekeep-test binary not found at ${BEEKEEP_DIR}/beekeep-test"
    error "Run without --skip-build to compile it."
    exit 1
fi

# --------------------------------------------------------------------------
# Step 4: Run test harness
# --------------------------------------------------------------------------
info "Step 4/4: Running test harness..."
echo ""

pushd "${BEEKEEP_DIR}" > /dev/null

# Build full harness command
HARNESS_CMD="./beekeep-test test/fixtures ${VERBOSE_FLAG}"
if [[ -n "${JSON_OUT}" ]]; then
    HARNESS_CMD="${HARNESS_CMD} --json-out ${JSON_OUT}"
fi

set +e
eval "${HARNESS_CMD}"
HARNESS_EXIT=$?
set -e

popd > /dev/null

# --------------------------------------------------------------------------
# Results
# --------------------------------------------------------------------------
echo ""
if [[ "${HARNESS_EXIT}" -eq 0 ]]; then
    success "All tests passed."
else
    error "One or more tests FAILED (exit code ${HARNESS_EXIT})."
fi

if [[ -n "${JSON_OUT}" && -f "${BEEKEEP_DIR}/${JSON_OUT}" ]]; then
    info "JSON results: ${BEEKEEP_DIR}/${JSON_OUT}"
elif [[ -n "${JSON_OUT}" && -f "${JSON_OUT}" ]]; then
    info "JSON results: ${JSON_OUT}"
fi

exit "${HARNESS_EXIT}"
