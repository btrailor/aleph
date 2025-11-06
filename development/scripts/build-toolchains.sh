#!/bin/bash
# Script to build both AVR32 and Blackfin toolchains inside the Docker container
# Run this inside the aleph-dev Docker container

set -e

echo "=== Building Aleph Toolchains ==="
echo ""

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}Step 1: Building AVR32 Toolchain${NC}"
echo "This will take 30-60 minutes..."
echo ""

cd /toolchain/avr32-toolchain

# Check if patches exist, if not extract from tar
if [ ! -d "avr32-patches" ]; then
    if [ -f "../avr32-patches.tar" ]; then
        echo "Extracting AVR32 patches..."
        cp ../avr32-patches.tar downloads/avr32-patches.tar.gz
        tar -xf downloads/avr32-patches.tar.gz
    else
        echo "Error: avr32-patches.tar not found"
        exit 1
    fi
fi

# Build AVR32 toolchain
make install-cross PREFIX=/opt/avr32-tools

echo -e "${GREEN}✓ AVR32 toolchain built successfully${NC}"
echo ""

echo -e "${YELLOW}Step 2: Installing Blackfin Toolchain${NC}"
echo ""

# Download and install Blackfin toolchain if not already installed
if [ ! -d "/opt/uClinux" ]; then
    cd /tmp
    echo "Downloading Blackfin toolchain..."
    wget -q --show-progress http://sourceforge.net/projects/adi-toolchain/files/2012R2/2012R2-RC2/i386/blackfin-toolchain-elf-gcc-4.3-2012R2-RC2.i386.tar.bz2

    echo "Installing Blackfin toolchain..."
    tar -xjf blackfin-toolchain-elf-gcc-4.3-2012R2-RC2.i386.tar.bz2 -C /

    echo -e "${GREEN}✓ Blackfin toolchain installed successfully${NC}"
else
    echo -e "${GREEN}✓ Blackfin toolchain already installed${NC}"
fi

echo ""
echo -e "${GREEN}=== All toolchains ready! ===${NC}"
echo ""
echo "Add these to your PATH:"
echo "  export PATH=/opt/avr32-tools/bin:/opt/uClinux/bfin-elf/bin:\$PATH"
echo ""
echo "Or add this to your .bashrc in the container:"
echo "  echo 'export PATH=/opt/avr32-tools/bin:/opt/uClinux/bfin-elf/bin:\$PATH' >> ~/.bashrc"
echo ""
echo "Test the toolchains:"
echo "  avr32-gcc --version"
echo "  bfin-elf-gcc --version"
