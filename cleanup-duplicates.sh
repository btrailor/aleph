#!/bin/bash
# Aleph Project Cleanup Script
# Removes duplicate files created by macOS/filesystem issues

set -e

echo "=== Aleph Project Cleanup ==="
echo ""

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Counters
COUNT_REMOVED=0
COUNT_SKIPPED=0

echo "Finding duplicate files (with ' 2.', ' 3.', etc. in name)..."
echo ""

# Find all duplicate files
DUPLICATES=$(find . -name "* 2.*" -o -name "* 3.*" -o -name "* 4.*" -o -name "* 5.*" -o -name "* 6.*" -o -name "* 7.*" -o -name "* 8.*" -o -name "* 9.*")

if [ -z "$DUPLICATES" ]; then
    echo -e "${GREEN}No duplicate files found!${NC}"
    exit 0
fi

TOTAL=$(echo "$DUPLICATES" | wc -l | tr -d ' ')
echo "Found $TOTAL duplicate files"
echo ""

# Categories
DOCS_COUNT=$(echo "$DUPLICATES" | grep -c "development/docs" || echo 0)
OBJ_COUNT=$(echo "$DUPLICATES" | grep -cE "\.(o|d)$" || echo 0)
MAKEFILE_COUNT=$(echo "$DUPLICATES" | grep -c "Makefile" || echo 0)
OTHER_COUNT=$((TOTAL - DOCS_COUNT - OBJ_COUNT - MAKEFILE_COUNT))

echo "Breakdown:"
echo "  Documentation files: $DOCS_COUNT"
echo "  Build artifacts (.o/.d): $OBJ_COUNT"
echo "  Makefiles: $MAKEFILE_COUNT"
echo "  Other files: $OTHER_COUNT"
echo ""

read -p "Proceed with cleanup? (y/n) " -n 1 -r
echo ""
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Cleanup cancelled."
    exit 0
fi

echo ""
echo "Removing duplicate files..."
echo ""

# Process each duplicate
while IFS= read -r file; do
    if [ -f "$file" ]; then
        # Check if it's tracked by git
        if git ls-files --error-unmatch "$file" > /dev/null 2>&1; then
            # It's tracked - need to git rm
            echo "  [git rm] $file"
            git rm -f "$file" > /dev/null 2>&1
            COUNT_REMOVED=$((COUNT_REMOVED + 1))
        else
            # Not tracked - just remove
            echo "  [rm] $file"
            rm -f "$file"
            COUNT_REMOVED=$((COUNT_REMOVED + 1))
        fi
    else
        COUNT_SKIPPED=$((COUNT_SKIPPED + 1))
    fi
done <<< "$DUPLICATES"

echo ""
echo -e "${GREEN}=== Cleanup Complete ===${NC}"
echo "  Removed: $COUNT_REMOVED files"
echo "  Skipped: $COUNT_SKIPPED files (already deleted)"
echo ""

# Check for duplicate Makefiles at root
echo "Checking for duplicate Makefiles at root..."
if [ -f "./Makefile 2" ]; then
    echo "  Found: Makefile 2"
    git rm -f "./Makefile 2" 2>/dev/null || rm -f "./Makefile 2"
fi
if [ -f "./Makefile 2.aleph" ]; then
    echo "  Found: Makefile 2.aleph"
    git rm -f "./Makefile 2.aleph" 2>/dev/null || rm -f "./Makefile 2.aleph"
fi

echo ""
echo -e "${YELLOW}Note: Run 'git status' to review changes${NC}"
echo "Run 'git add -A && git commit -m \"Clean up duplicate files\"' to commit"
