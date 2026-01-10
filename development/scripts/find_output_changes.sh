#!/bin/bash
# Find all operators that changed output counts between 0.7.1 and 0.8.x
# This is CRITICAL for scene migration - operators that gained outputs break connections

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "======================================================================"
echo "CRITICAL: Finding Operators That Gained Outputs (0.7.1 -> 0.8.x)"
echo "======================================================================"
echo ""
echo "This script identifies operators that gained outputs between versions."
echo "Per Yann Copier: SCREEN and BIGNUM are examples - need to find ALL."
echo ""

# Get the repository root
REPO_ROOT=$(git rev-parse --show-toplevel)
cd "$REPO_ROOT"

# Save current branch/commit
current_ref=$(git rev-parse --abbrev-ref HEAD)
if [ "$current_ref" = "HEAD" ]; then
  # Detached HEAD, save commit
  current_ref=$(git rev-parse HEAD)
fi

echo "Current location: $current_ref"
echo ""

# Ask user for 0.7.1 reference
echo "Enter the git reference for 0.7.1 (tag, branch, or commit):"
echo "  (Try: git tag | grep 0.7 to find it)"
read -p "0.7.1 ref: " ref_07

if [ -z "$ref_07" ]; then
  echo "Error: No reference provided for 0.7.1"
  exit 1
fi

# Use 'dev' branch for 0.8.x
ref_08="dev"

echo ""
echo "Will compare:"
echo "  0.7.1: $ref_07"
echo "  0.8.x: $ref_08"
echo ""
read -p "Press Enter to continue..."

# Create temp directory for results
mkdir -p /tmp/bees_output_analysis

# Function to extract output counts
extract_outputs() {
  local version=$1
  local outfile=$2
  
  echo "Scanning $version operators..."
  
  for file in apps/bees/src/ops/op_*.c; do
    if [ -f "$file" ]; then
      opname=$(basename "$file" .c | sed 's/op_//')
      
      # Try multiple patterns to find numOutputs
      outputs=$(grep -h "numOutputs.*=" "$file" | head -1 | grep -o '[0-9]\+' || echo "")
      
      if [ -z "$outputs" ]; then
        outputs="?"  # Unknown - need manual check
      fi
      
      echo "$opname:$outputs" >> "$outfile"
    fi
  done
  
  sort "$outfile" -o "$outfile"
}

# Extract from 0.7.1
echo ""
echo "Checking out 0.7.1..."
git checkout "$ref_07" >/dev/null 2>&1

extract_outputs "0.7.1" "/tmp/bees_output_analysis/outputs_v07.txt"

# Extract from 0.8.x
echo "Checking out 0.8.x..."
git checkout "$ref_08" >/dev/null 2>&1

extract_outputs "0.8.x" "/tmp/bees_output_analysis/outputs_v08.txt"

# Restore original location
echo "Restoring original location..."
git checkout "$current_ref" >/dev/null 2>&1

# Compare results
echo ""
echo "======================================================================"
echo "RESULTS: Operators with Output Count Changes"
echo "======================================================================"
echo ""

# Join the files and compare
join -t: /tmp/bees_output_analysis/outputs_v07.txt /tmp/bees_output_analysis/outputs_v08.txt | \
  awk -F: -v RED="$RED" -v GREEN="$GREEN" -v YELLOW="$YELLOW" -v NC="$NC" '
  BEGIN {
    changed = 0
    unknown = 0
  }
  {
    op = $1
    v07 = $2
    v08 = $3
    
    # Handle unknowns
    if (v07 == "?" || v08 == "?") {
      printf "%s%-20s  v0.7: %3s  v0.8: %3s  UNKNOWN - MANUAL CHECK NEEDED%s\n", YELLOW, op, v07, v08, NC
      unknown++
    }
    # Show changes
    else if (v07 != v08) {
      diff = v08 - v07
      if (diff > 0) {
        printf "%s%-20s  v0.7: %3s  v0.8: %3s  (+%d outputs) *** BREAKS SCENES ***%s\n", RED, op, v07, v08, diff, NC
      } else {
        printf "%s%-20s  v0.7: %3s  v0.8: %3s  (%d outputs)%s\n", GREEN, op, v07, v08, diff, NC
      }
      changed++
    }
  }
  END {
    print ""
    print "======================================================================"
    printf "TOTAL CHANGED: %d operators\n", changed
    printf "NEEDS MANUAL CHECK: %d operators\n", unknown
    print "======================================================================"
  }'

# Also show operators that only exist in one version
echo ""
echo "Operators only in 0.7.1:"
comm -23 /tmp/bees_output_analysis/outputs_v07.txt /tmp/bees_output_analysis/outputs_v08.txt | \
  awk -F: '{print "  - " $1}'

echo ""
echo "Operators only in 0.8.x (NEW):"
comm -13 /tmp/bees_output_analysis/outputs_v07.txt /tmp/bees_output_analysis/outputs_v08.txt | \
  awk -F: '{print "  - " $1}'

echo ""
echo "======================================================================"
echo "Raw data saved to:"
echo "  /tmp/bees_output_analysis/outputs_v07.txt"
echo "  /tmp/bees_output_analysis/outputs_v08.txt"
echo "======================================================================"
echo ""
echo "Next steps:"
echo "1. Document ALL operators with +N outputs in OPERATOR_OUTPUT_CHANGES.h"
echo "2. Manually check any '?' entries in the operator source files"
echo "3. Update scene migration code to handle output index shifts"
echo ""
