# Monome Aleph Development Environment Setup

This guide will help you set up a development environment for the Monome Aleph firmware.

## System Requirements

- macOS (tested on macOS 15.3, Apple Silicon)
- Docker Desktop
- At least 10GB free disk space

## Overview

The Monome Aleph uses two processors:
1. **AVR32 (Controller)** - Handles UI, encoders, switches, USB, I2C, SD card, CV input
2. **Blackfin BF533 (DSP)** - Handles audio I/O, DSP processing, CV output

Each processor requires its own toolchain to compile firmware.

## Setup Options

### Option 1: Docker-based Development (Recommended for Apple Silicon)

Docker provides a consistent Linux environment that's compatible with the older toolchains.

#### 1. Build the Docker image

```bash
docker-compose build
```

#### 2. Start the development container

```bash
docker-compose run --rm aleph-dev
```

#### 3. Inside the container, set up the AVR32 toolchain

```bash
cd /toolchain/avr32-toolchain
make install-cross PREFIX=/opt/avr32-tools
export PATH=/opt/avr32-tools/bin:$PATH
```

#### 4. Set up the Blackfin toolchain

```bash
# Download Blackfin toolchain (2012R2-RC2)
cd /tmp
wget http://sourceforge.net/projects/adi-toolchain/files/2012R2/2012R2-RC2/i386/blackfin-toolchain-elf-gcc-4.3-2012R2-RC2.i386.tar.bz2
tar -xjf blackfin-toolchain-elf-gcc-4.3-2012R2-RC2.i386.tar.bz2 -C /
export PATH=/opt/uClinux/bfin-elf/bin:$PATH
```

#### 5. Test the build

```bash
# Build a controller app (BEES)
cd /aleph/apps/bees
make

# Build a DSP module (e.g., lines)
cd /aleph/modules/lines
make
```

### Option 2: Native macOS Build (Advanced, Intel Macs only)

This option is only recommended for Intel Macs. Apple Silicon users should use Docker.

#### 1. Install Homebrew dependencies

```bash
brew install mpfr gmp libmpc texinfo
```

#### 2. Build AVR32 toolchain

```bash
cd toolchain/avr32-toolchain
make install-cross
```

Add to your `.zshrc` or `.bash_profile`:
```bash
export PATH=$HOME/avr32-tools-XXXXXX/bin:$PATH
```

#### 3. For Blackfin on macOS, use Docker

```bash
docker run --rm -ti -v ~/Documents/01\ Projects/Aleph/aleph:/projects/aleph pf0camino/cross-bfin-elf "/bin/bash"
```

## Building Firmware

### Controller Firmware (AVR32)

The main controller application is BEES:

```bash
cd aleph/apps/bees
make
```

This produces `aleph-bees.hex` which can be loaded via the bootloader.

### DSP Modules (Blackfin)

Example - building the "lines" module:

```bash
cd aleph/modules/lines
make
```

This produces a `.ldr` file that the controller loads onto the DSP.

## Loading Firmware

1. **Enter bootloader mode**: Hold the MODE switch (square button, upper right) while powering on
2. **Load AVR32 application**: Place `.hex` file in `/hex` folder on SD card
3. **DSP modules**: Place `.ldr` files in `/data` folder on SD card

## Debugging

Connect via USB and use a serial terminal:

```bash
# macOS
sudo stty -F /dev/ttyACM0 115200
minicom -D /dev/ttyACM0

# Or use screen
screen /dev/ttyACM0 115200
```

## Project Structure

```
aleph/
├── apps/           # AVR32 applications
│   └── bees/      # Main routing/management app
├── modules/       # Blackfin DSP modules
│   ├── lines/    # Example module
│   └── mix/      # Mixer module
├── dsp/           # Common DSP functions (32-bit fixed-point)
├── common/        # Shared code (SPI protocol, etc.)
├── avr32_lib/     # AVR32-specific libraries
└── bfin_lib/      # Blackfin-specific libraries
```

## Troubleshooting

### AVR32 Toolchain Build Fails
- Make sure you have all Homebrew dependencies installed
- Check that the avr32-patches were extracted correctly
- On Apple Silicon, use Docker instead

### Blackfin Toolchain Issues
- Use the Docker image: `pf0camino/cross-bfin-elf`
- Or install via the official SourceForge download

### "Host not found" errors during build
- The old Atmel distribution server is offline
- Use the patches from `toolchain/avr32-patches.tar`

## Resources

- [Official Aleph Documentation](http://www.monome.org/docs/aleph)
- [Aleph GitHub Repository](https://github.com/monome/aleph)
- [Community Forum](https://llllllll.co/)

## Quick Start Workflow

```bash
# 1. Start Docker environment
docker-compose run --rm aleph-dev

# 2. Build BEES controller app
cd /aleph/apps/bees && make

# 3. Build a DSP module
cd /aleph/modules/lines && make

# 4. Copy files to SD card
# - Copy .hex to /hex folder
# - Copy .ldr to /data folder

# 5. Load on hardware
# - Boot into bootloader (hold MODE while powering on)
# - Select the .hex file to flash
```
