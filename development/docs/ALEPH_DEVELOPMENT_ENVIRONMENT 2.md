# Aleph Dual-Container Development Environment

**Complete Docker-based development setup for Aleph hardware platform**

## Overview

The Aleph development environment supports both processor architectures in the Aleph hardware:

- **AVR32 Development**: Firmware development (AT32UC3A0512)
- **Blackfin Development**: DSP module development (ADSP-BF533)

**Current Setup**: **Unified Container Architecture** using `aleph-builder` image with both toolchains.

This provides **real hardware compilation** with official toolchains from both Atmel and Analog Devices in a single container.

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                     ALEPH DEVELOPMENT ENVIRONMENT               │
├─────────────────────────────────────────────────────────────────┤
│                        Host System (macOS)                      │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────────┐ │
│  │              UNIFIED CONTAINER (aleph-builder)             │ │
│  │                  Platform: linux/amd64                     │ │
│  │                                                             │ │
│  │  ┌─────────────────┐              ┌─────────────────┐       │ │
│  │  │   AVR32 BUILD   │              │ BLACKFIN BUILD  │       │ │
│  │  │                 │              │                 │       │ │
│  │  │ • GCC 4.4.7     │              │ • GCC 4.3.5     │       │ │
│  │  │ • ASF Framework │              │ • BF533 Target  │       │ │
│  │  │ • AT32UC3A0512  │              │ • LDR Tools     │       │ │
│  │  │ • Build Scripts │              │ • DSP Libraries │       │ │
│  │  └─────────────────┘              └─────────────────┘       │ │
│  └─────────────────────────────────────────────────────────────┘ │
│           │                                 │                   │
│           ▼                                 ▼                   │
│  ┌─────────────────┐              ┌─────────────────┐           │
│  │  FIRMWARE       │              │  DSP MODULES    │           │
│  │  OUTPUT         │              │  OUTPUT         │           │
│  │                 │              │                 │           │
│  │ • aleph-bees.hex│              │ • 12/13 .ldr    │           │
│  │ • build-output/ │              │ • Auto-built    │           │
│  │ • Optimized     │              │ • 92% success   │           │
│  └─────────────────┘              └─────────────────┘           │
│           │                                 │                   │
│           └─────────────┬───────────────────┘                   │
│                         ▼                                       │
│              ┌─────────────────┐                                │
│              │   SD CARD       │                                │
│              │   BUNDLER       │                                │
│              │                 │                                │
│              │ • Complete image│                                │
│              │ • 14 Scenes     │                                │
│              │ • DSP Modules   │                                │
│              │ • Latest FW     │                                │
│              └─────────────────┘                                │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

## Container Details

### Unified Container (`aleph-builder`)

- **Image**: `aleph-builder:latest`
- **Platform**: x86_64 (linux/amd64) with ARM64 support
- **Base**: Ubuntu 20.04
- **Purpose**: Complete Aleph development (firmware + DSP modules)

#### AVR32 Toolchain Components:

- **AVR32 GCC**: Cross-compiler for AT32UC3A0512 firmware
- **ASF Framework**: Complete Atmel Software Framework
- **BEES Build System**: Optimized firmware compilation
- **Location**: Available in PATH within container

#### Blackfin Toolchain Components:

- **Blackfin GCC 4.3.5**: Official ADI cross-compiler for BF533
- **DSP Libraries**: Hardware-optimized math libraries
- **LDR Tools**: Boot loader generation for BF533
- **Memory Management**: L1/L2 memory region support
- **Location**: `/opt/uClinux/bfin-elf/bin/`

## Build Scripts

### Automated Build Tools

- **`./build-bees.sh`**: Complete AVR32 firmware build
- **`./build-dsp-modules.sh`**: Build all Blackfin DSP modules
- **`./utils/sdcard-bundler/bundle-sdcard.sh`**: Create complete SD card image

### Manual Build Commands

```bash
# Build firmware
docker run --rm -v "$(pwd):/aleph" aleph-builder bash -c "cd /aleph/apps/bees && make"

# Build single DSP module
docker run --rm -v "$(pwd):/aleph" aleph-builder bash -c "cd /aleph/modules/waves && make"
```

## Development Status

### Current Success Metrics

- **Firmware Building**: ✅ 100% Success (aleph-bees.hex generated)
- **DSP Module Building**: ✅ 92% Success (12/13 modules built)
  - **Successful**: acid, analyser, dacs, dsyn, fmsynth, grains, lines, monosynth, tape, varilines, voder, waves
  - **Failed**: mix (linker script issue)
- **SD Card Bundling**: ✅ 100% Success (firmware + 14 scenes + modules)
- **Container Environment**: ✅ Unified architecture implemented

### Technical Debt

- **High Priority**: Fix mix.lds linker script for 100% DSP build success
- **Medium Priority**: Document scaler_fix.c compiler warning (already noted)
- **Low Priority**: Optimize container startup time for development workflow

## Setup Instructions

### Prerequisites

- Docker Desktop with multi-platform support
- At least 8GB RAM and 20GB disk space
- Working Aleph source code directory

### 1. Create AVR32 Container

```bash
# Create ARM64 container for AVR32 development
docker run -d --name aleph-avr32 \
  --platform linux/arm64 \
  -v "/path/to/aleph:/workspace" \
  ubuntu:latest sleep infinity

# Install dependencies
docker exec -it aleph-avr32 apt-get update
docker exec -it aleph-avr32 apt-get install -y \
  build-essential curl git make autoconf automake \
  flex bison libgmp3-dev libmpfr-dev libmpc-dev \
  texinfo libusb-dev
```

### 2. Build AVR32 Toolchain

```bash
# Clone and build AVR32 toolchain
docker exec -it aleph-avr32 bash -c '
  cd /toolchain
  git clone https://github.com/jsnyder/avr32-toolchain.git
  cd avr32-toolchain
  make install-cross PREFIX=/root/avr32-tools-6711e09
'
```

### 3. Create Blackfin Container

```bash
# Create x86_64 container for Blackfin development
docker run -d --name aleph-blackfin \
  --platform linux/amd64 \
  -v "/path/to/aleph:/workspace" \
  ubuntu:20.04 sleep infinity

# Install dependencies
docker exec -it aleph-blackfin apt-get update
docker exec -it aleph-blackfin apt-get install -y \
  build-essential curl wget tar bzip2 make
```

### 4. Install Official Blackfin Toolchain

```bash
# Download and install ADI Blackfin toolchain
docker exec -it aleph-blackfin bash -c '
  cd /tmp
  curl -L -O "https://sourceforge.net/projects/adi-toolchain/files/2014R1/2014R1-RC2/x86_64/blackfin-toolchain-2014R1-RC2.x86_64.tar.bz2/download"
  tar -xjf download -C /
  /opt/uClinux/bfin-elf/bin/bfin-elf-gcc --version
'
```

## Usage

### AVR32 Firmware Development

```bash
# Enter AVR32 container
docker exec -it aleph-avr32 bash

# Set up environment
export PATH=/root/avr32-tools-6711e09/bin:$PATH

# Build BEES firmware
cd /workspace/aleph/apps/bees
make clean
make

# Results: aleph-bees.elf, aleph-bees.hex, aleph-bees.bin
```

### Blackfin DSP Development

**Current Setup**: Our `aleph-builder` image includes both AVR32 and Blackfin toolchains.

#### Build Individual DSP Module

```bash
# Build single DSP module
cd /path/to/aleph/project
docker run --rm -v "$(pwd):/aleph" aleph-builder bash -c "cd /aleph/modules/waves && make"

# Results: waves.ldr (hardware loader file)
```

#### Build All DSP Modules

Use the automated build script:

```bash
# Build all DSP modules at once
./build-dsp-modules.sh

# Results:
# - All .ldr files generated in respective module directories
# - Build log: dsp-build.log
# - Summary report of successes/failures
```

#### Manual Build Process

```bash
# Individual module build with detailed output
docker run --rm -v "$(pwd):/aleph" aleph-builder bash -c "
  cd /aleph/modules/[MODULE_NAME]
  make clean  # Optional: clean previous build
  make        # Build the module
"

# Check results
ls -la modules/[MODULE_NAME]/*.ldr  # Generated loader file
```

#### Module Build Status

Current build success rate: **12/13 modules (92%)**

✅ **Successfully building**: acid, analyser, dacs, dsyn, fmsynth, grains, lines, monosynth, tape, varilines, voder, waves  
❌ **Build issues**: mix (linker script error)

## File Structure

### Shared Workspace

```
/workspace/aleph/
├── apps/
│   └── bees/              # Main firmware (AVR32)
├── modules/
│   ├── mix/               # Audio mixer (Blackfin)
│   ├── lines/             # Delay lines (Blackfin)
│   ├── waves/             # Oscillators (Blackfin)
│   └── fmsynth/           # FM synthesis (Blackfin)
├── dsp/                   # DSP library functions
├── common/                # Shared headers
└── libavr32/              # AVR32 libraries
```

### Container-Specific Paths

#### AVR32 Container:

- **Toolchain**: `/root/avr32-tools-6711e09/bin/`
- **ASF Framework**: `/workspace/aleph/libavr32/asf/`
- **Build Output**: `/workspace/aleph/apps/bees/`

#### Blackfin Container:

- **Toolchain**: `/opt/uClinux/bfin-elf/bin/`
- **Build Output**: `/workspace/aleph/modules/*/`
- **Libraries**: `/opt/uClinux/bfin-elf/bfin-elf/lib/`

## Troubleshooting

### Common Issues

#### 1. Container Architecture Mismatch

**Problem**: Blackfin toolchain won't run on ARM64  
**Solution**: Ensure Blackfin container uses `--platform linux/amd64`

#### 2. Linker Script Errors

**Problem**: "no memory region specified for loadable section"  
**Solution**: Copy complete linker script from lines module:

```bash
cp /workspace/aleph/modules/lines/lines.lds /workspace/aleph/modules/TARGET/TARGET.lds
```

#### 3. Missing ASF Framework

**Problem**: AVR32 build fails with missing headers  
**Solution**: Install complete ASF framework in AVR32 container

#### 4. Path Issues

**Problem**: Toolchain not found  
**Solution**: Always export PATH before building:

```bash
# AVR32
export PATH=/root/avr32-tools-6711e09/bin:$PATH

# Blackfin
export PATH=/opt/uClinux/bfin-elf/bin:$PATH
```

### Container Management

```bash
# View running aleph-builder containers
docker ps | grep aleph-builder

# View all aleph-builder containers
docker ps -a | grep aleph-builder

# Run builds using aleph-builder (containers are ephemeral)
docker run --rm -v "$(pwd)/aleph:/aleph" aleph-builder bash -c "cd /aleph/apps/bees && make"
docker run --rm -v "$(pwd)/aleph:/aleph" aleph-builder bash -c "cd /aleph/modules/waves && make"

# Interactive session for debugging
docker run --rm -it -v "$(pwd)/aleph:/aleph" aleph-builder bash

# Container cleanup (if needed)
docker system prune -f
```

## Performance Notes

- **AVR32 builds**: ~2-5 minutes for full firmware
- **Blackfin builds**: ~30-60 seconds per DSP module
- **Container startup**: ~5-10 seconds
- **Toolchain verification**: ~2-3 seconds

## Maintenance

### Updates

- **AVR32 toolchain**: Rebuild from latest jsnyder/avr32-toolchain
- **Blackfin toolchain**: ADI 2014R1 is the final supported version
- **Source code**: Update from official Aleph repositories

### Backup

- **Container export**: `docker export container_name > backup.tar`
- **Workspace**: Regular backup of `/workspace/aleph/` directory
- **Build outputs**: Archive generated `.elf` and `.ldr` files

---

This unified container setup provides a **complete, professional-grade development environment** for both Aleph processor architectures with real hardware compilation capabilities using a single `aleph-builder` image.
