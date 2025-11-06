#!/bin/bash
# Monome Aleph Development Environment Setup Script

set -e

echo "=== Monome Aleph Development Environment Setup ==="
echo ""

# Check if Docker is installed
if ! command -v docker &> /dev/null; then
    echo "Error: Docker is not installed. Please install Docker Desktop first."
    echo "Download from: https://www.docker.com/products/docker-desktop"
    exit 1
fi

echo "✓ Docker is installed"

# Build the Docker image directly
echo "Building Docker image (this may take a while)..."
docker build -t aleph-dev .

echo ""
echo "=== Setup Complete! ==="
echo ""
echo "To start developing:"
echo "  1. Run: docker run --rm -it -v \"\$(pwd)/aleph:/aleph\" -v \"\$(pwd)/toolchain:/toolchain\" aleph-dev"
echo "     Or use the shortcut: make dev"
echo "  2. Inside the container, build the toolchains: /aleph/build-toolchains.sh"
echo ""
echo "Quick reference:"
echo "  - Build BEES: cd /aleph/apps/bees && make"
echo "  - Build a module: cd /aleph/modules/lines && make"
echo ""
echo "For detailed instructions, see DEVELOPMENT_SETUP.md"
