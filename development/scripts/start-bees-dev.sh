#!/bin/bash
# Aleph BEES Development Environment Startup Script

set -e

echo "========================================="
echo "  Monome Aleph - BEES 1.0 Development"
echo "  Starting Docker Environment"
echo "========================================="
echo ""

# Check if Docker is running
if ! docker info > /dev/null 2>&1; then
    echo "❌ Error: Docker is not running"
    echo "Please start Docker/Colima first"
    exit 1
fi

echo "✅ Docker is running"
echo ""

# Check if aleph-dev image exists
if ! docker images | grep -q "aleph-dev"; then
    echo "⚠️  aleph-dev image not found. Building..."
    docker build -t aleph-dev .
    echo "✅ Image built successfully"
else
    echo "✅ aleph-dev image found"
fi

echo ""
echo "Starting development container..."
echo "You will be dropped into /aleph directory"
echo ""
echo "Quick commands:"
echo "  - Build AVR32 toolchain: cd /toolchain/avr32-toolchain && make install-cross PREFIX=/opt/avr32-tools"
echo "  - Add to PATH: export PATH=/opt/avr32-tools/bin:\$PATH"
echo "  - Build BEES: cd /aleph/apps/bees && make"
echo "  - Build DSP module: cd /aleph/modules/lines && ..."
echo ""
echo "Press ENTER to continue..."
read

# Start container with mounted volumes
docker run --rm -it \
  -v "$(pwd)/aleph:/aleph" \
  -v "$(pwd)/toolchain:/toolchain" \
  -w /aleph \
  aleph-dev
