#!/bin/bash
# Quick test script for the Aleph development environment

echo "=== Monome Aleph Development Environment Test ==="
echo ""

# Check if Docker is running
if ! docker info > /dev/null 2>&1; then
    echo "❌ Docker is not running. Please start Docker/Colima first."
    exit 1
fi
echo "✅ Docker is running"

# Check if image exists
if ! docker images | grep -q "aleph-dev"; then
    echo "❌ aleph-dev image not found. Run 'make setup' first."
    exit 1
fi
echo "✅ Docker image 'aleph-dev' exists"

# Test container startup
echo "Testing container startup..."
if docker run --rm -v "$(pwd)/aleph:/aleph" -v "$(pwd)/toolchain:/toolchain" aleph-dev bash -c "echo Container started successfully" > /dev/null 2>&1; then
    echo "✅ Container starts successfully"
else
    echo "❌ Container failed to start"
    exit 1
fi

# Test GCC availability
echo "Testing GCC..."
GCC_VERSION=$(docker run --rm aleph-dev gcc --version | head -1)
echo "✅ $GCC_VERSION"

# Test volume mounts
echo "Testing volume mounts..."
ALEPH_FILES=$(docker run --rm -v "$(pwd)/aleph:/aleph" aleph-dev bash -c "ls /aleph | wc -l")
TOOLCHAIN_FILES=$(docker run --rm -v "$(pwd)/toolchain:/toolchain" aleph-dev bash -c "ls /toolchain | wc -l")

if [ "$ALEPH_FILES" -gt 0 ]; then
    echo "✅ Aleph source mounted ($ALEPH_FILES items)"
else
    echo "❌ Aleph source mount failed"
    exit 1
fi

if [ "$TOOLCHAIN_FILES" -gt 0 ]; then
    echo "✅ Toolchain source mounted ($TOOLCHAIN_FILES items)"
else
    echo "❌ Toolchain mount failed"
    exit 1
fi

# Check for AVR32 toolchain files
echo "Checking AVR32 toolchain files..."
if docker run --rm -v "$(pwd)/toolchain:/toolchain" aleph-dev bash -c "[ -f /toolchain/avr32-toolchain/Makefile ]"; then
    echo "✅ AVR32 toolchain Makefile found"
else
    echo "❌ AVR32 toolchain Makefile not found"
    exit 1
fi

# Check for patches
if docker run --rm -v "$(pwd)/toolchain:/toolchain" aleph-dev bash -c "[ -d /toolchain/avr32-toolchain/avr32-patches ]"; then
    echo "✅ AVR32 patches extracted"
else
    echo "❌ AVR32 patches not found"
    exit 1
fi

echo ""
echo "=== All Tests Passed! ==="
echo ""
echo "Your development environment is ready."
echo ""
echo "Next steps:"
echo "  1. Run: make dev"
echo "  2. Inside container: /aleph/build-toolchains.sh"
echo "  3. Wait for toolchains to build (~30-60 minutes)"
echo "  4. Start developing!"
echo ""
echo "Quick commands:"
echo "  make dev         - Start development shell"
echo "  make build-bees  - Build BEES controller app"
echo "  make build-lines - Build lines DSP module"
