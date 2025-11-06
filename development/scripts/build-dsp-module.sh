#!/bin/bash
# Helper script to build Blackfin DSP modules using Docker

# Usage: ./build-dsp-module.sh <module-name>
# Example: ./build-dsp-module.sh lines

set -e

if [ -z "$1" ]; then
    echo "Usage: $0 <module-name>"
    echo ""
    echo "Available modules:"
    ls -1 aleph/modules/ | grep -v "^\." | grep -v "README" | grep -v "UNLICENSE"
    exit 1
fi

MODULE="$1"
MODULE_PATH="aleph/modules/$MODULE"

if [ ! -d "$MODULE_PATH" ]; then
    echo "Error: Module '$MODULE' not found at $MODULE_PATH"
    echo ""
    echo "Available modules:"
    ls -1 aleph/modules/ | grep -v "^\." | grep -v "README" | grep -v "UNLICENSE"
    exit 1
fi

echo "=== Building Blackfin DSP Module: $MODULE ==="
echo ""

# Check if Docker is available
if ! command -v docker &> /dev/null; then
    echo "Error: Docker is not installed or not in PATH"
    exit 1
fi

# Check if the Blackfin Docker image is available
if ! docker images | grep -q "pf0camino/cross-bfin-elf"; then
    echo "Blackfin toolchain Docker image not found. Pulling..."
    docker pull pf0camino/cross-bfin-elf
fi

echo "Building module: $MODULE"
echo "Working directory: $MODULE_PATH"
echo ""

# Build the module
docker run --rm --platform linux/amd64 --entrypoint="" \
    -v "$(pwd)/aleph:/projects/aleph" \
    pf0camino/cross-bfin-elf bash -c \
    "cd /projects/aleph/modules/$MODULE && \
     export PATH=/opt/uClinux/bfin-elf/bin:\$PATH && \
     make 2>&1"

BUILD_STATUS=$?

echo ""
if [ $BUILD_STATUS -eq 0 ]; then
    echo "✅ Build succeeded!"
    echo ""
    echo "Output files:"
    ls -lh "$MODULE_PATH"/*.ldr "$MODULE_PATH"/*.o 2>/dev/null || echo "  (check $MODULE_PATH for build artifacts)"
else
    echo "❌ Build failed with exit code $BUILD_STATUS"
    echo ""
    echo "Common issues:"
    echo "  - Check that all source files are present"
    echo "  - Verify the Makefile is correct"
    echo "  - See BLACKFIN_TEST_RESULTS.md for known issues"
fi

exit $BUILD_STATUS
