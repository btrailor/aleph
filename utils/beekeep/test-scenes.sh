#!/usr/bin/env bash
# test-scenes.sh — Bulk scene loading diagnostic
#
# Usage: ./test-scenes.sh [scenes-dir]
# Default: ../release/scenes-0.7.1
#
# For each .scn file, attempts to load it via the beekeep test harness
# and reports whether conversion succeeds.

SCENES_DIR="${1:-../release/scenes-0.7.1}"
BEEKEEP="./beekeep-0.8.3"
PASSED=0
FAILED=0
TOTAL=0

echo "# Scene Bulk Load Test"
echo "# Directory: $SCENES_DIR"
echo "#"

if [ ! -d "$SCENES_DIR" ]; then
    echo "BAIL OUT: Directory not found: $SCENES_DIR"
    exit 1
fi

# Build beekeep first if needed
if [ ! -x "$BEEKEEP" ]; then
    echo "# Building beekeep..."
    make HEADLESS=1 || { echo "Build failed"; exit 1; }
fi

for scene in "$SCENES_DIR"/*.scn; do
    # Skip backup files (" 2.scn")
    if [[ "$(basename "$scene")" == *" 2.scn" ]]; then
        continue
    fi
    
    TOTAL=$((TOTAL + 1))
    name=$(basename "$scene")
    
    # Try to load the scene by passing it as an argument to beekeep
    # The headless binary loads the first scene argument if provided
    output=$("$BEEKEEP" "$scene" 2>&1 | tail -20)
    
    if echo "$output" | grep -q "ERROR"; then
        echo "not ok $TOTAL - $name (load error)"
        echo "# ---"
        echo "$output" | sed 's/^/# /'
        echo "# ---"
        FAILED=$((FAILED + 1))
    elif echo "$output" | grep -q "module descriptor not found"; then
        # This is expected for most scenes — DSP modules aren't mocked
        echo "ok $TOTAL - $name (scene loads, DSP missing — expected)"
        PASSED=$((PASSED + 1))
    elif echo "$output" | grep -q "num ops"; then
        # Extract op count from output
        ops=$(echo "$output" | grep "num ops" | tail -1 | sed 's/.*num ops //')
        echo "ok $TOTAL - $name ($ops ops)"
        PASSED=$((PASSED + 1))
    else
        echo "ok $TOTAL - $name (loaded)"
        PASSED=$((PASSED + 1))
    fi
done

echo ""
echo "# Results: $TOTAL total, $PASSED passed, $FAILED failed"

if [ $FAILED -eq 0 ]; then
    echo "# All scenes loadable (DSP module warnings are expected)"
else
    echo "# $FAILED scene(s) have load errors"
fi

exit $FAILED
