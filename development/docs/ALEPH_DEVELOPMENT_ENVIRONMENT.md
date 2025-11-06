# Aleph Dual-Container Development Environment

**Complete Docker-based development setup for Aleph hardware platform**

## Overview

The Aleph development environment uses a **dual-container architecture** to support both processor architectures in the Aleph hardware:

- **AVR32 Container**: Firmware development (AT32UC3A0512)
- **Blackfin Container**: DSP module development (ADSP-BF533)

This setup provides **real hardware compilation** with official toolchains from both Atmel and Analog Devices.

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                    Host System (macOS)                     │
│  ┌─────────────────────────┐  ┌─────────────────────────────┐ │
│  │    AVR32 Container      │  │    Blackfin Container       │ │
│  │   (ARM64 Ubuntu)        │  │   (x86_64 Ubuntu 20.04)    │ │
│  │                         │  │                             │ │
│  │ ┌─────────────────────┐ │  │ ┌─────────────────────────┐ │ │
│  │ │  AVR32 Toolchain    │ │  │ │  Blackfin Toolchain     │ │ │
│  │ │  GCC 4.4.7          │ │  │ │  ADI 2014R1-RC2         │ │ │
│  │ │  Binutils 2.23.1    │ │  │ │  GCC 4.3.5              │ │ │
│  │ │  ASF Framework      │ │  │ │  Binutils 2.21          │ │ │
│  │ └─────────────────────┘ │  │ └─────────────────────────┘ │ │
│  │                         │  │                             │ │
│  │ Target: AT32UC3A0512    │  │ Target: ADSP-BF533          │ │
│  │ Output: .elf/.hex/.bin  │  │ Output: .elf/.ldr           │ │
│  └─────────────────────────┘  └─────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

## Container Details

### AVR32 Container (`aleph-avr32`)

- **ID**: `16c993596a63`
- **Platform**: ARM64 (linux/arm64)
- **Base**: Ubuntu (latest)
- **Purpose**: Aleph firmware development
- **Toolchain**: Custom-built from jsnyder/avr32-toolchain
- **Location**: `/root/avr32-tools-6711e09/bin/`

#### Key Components:

- **AVR32 GCC 4.4.7**: Cross-compiler for AT32UC3A0512
- **ASF Framework**: Complete Atmel Software Framework
- **BEES Source**: Aleph's primary application firmware
- **Custom Headers**: Minimal C library implementation

### Blackfin Container (`aleph-blackfin`)

- **Platform**: x86_64 (linux/amd64)
- **Base**: Ubuntu 20.04
- **Purpose**: DSP module development
- **Toolchain**: Official ADI Blackfin 2014R1-RC2
- **Location**: `/opt/uClinux/bfin-elf/bin/`

#### Key Components:

- **Blackfin GCC 4.3.5**: Official ADI cross-compiler for BF533
- **DSP Libraries**: Hardware-optimized math libraries
- **LDR Tools**: Boot loader generation for BF533
- **Memory Management**: L1/L2 memory region support

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

```bash
# Enter Blackfin container
docker exec -it aleph-blackfin bash

# Set up environment
export PATH=/opt/uClinux/bfin-elf/bin:$PATH

# Build DSP module (example: mix)
cd /workspace/aleph/modules/mix
make clean
make

# Results: mix (ELF), mix.ldr (hardware loader)
```

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
# Start containers
docker start aleph-avr32 aleph-blackfin

# Stop containers
docker stop aleph-avr32 aleph-blackfin

# View container status
docker ps -a | grep aleph

# Clean up (removes containers)
docker rm -f aleph-avr32 aleph-blackfin
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

This dual-container setup provides a **complete, professional-grade development environment** for both Aleph processor architectures with real hardware compilation capabilities.
