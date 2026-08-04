# Building Aleph Firmware

Quick guide for building BEES firmware and DSP modules.

---

## Quick Build (Docker)

### Prerequisites

- Docker installed
- Git submodules initialized: `git submodule update --init --recursive`

### Build BEES Firmware

```bash
# Build with Docker
docker run --rm --platform=linux/amd64 \
  -v "$(pwd):/aleph" -w /aleph/apps/bees \
  aleph-builder make

# Output: apps/bees/aleph-bees.hex
```

### Build DSP Module (example: lines)

```bash
# Build with Docker
docker run --rm --platform=linux/amd64 \
  -v "$(pwd):/aleph" -w /aleph/modules/lines \
  pf0camino/cross-bfin-elf bash -c \
  "export PATH=/opt/uClinux/bfin-elf/bin:\$PATH && make"

# Output: modules/lines/lines.ldr
```

---

## Build Environment Setup

### AVR32 Toolchain (for BEES)

```bash
# Build Docker image with AVR32 tools
cd development/environment
docker build -t aleph-builder .
```

### Blackfin Toolchain (for DSP modules)

```bash
# Use pre-built Docker image
docker pull pf0camino/cross-bfin-elf
```

---

## Native Build (Advanced)

If you have the toolchains installed locally:

```bash
# BEES firmware
cd apps/bees
make

# DSP module
cd modules/lines
make
```

Required toolchains:

- **AVR32**: avr32-gcc (for controller firmware)
- **Blackfin**: bfin-elf-gcc (for DSP modules)

---

## Installation to SD Card

After building:

```bash
# Install to SD card (mounted at /path/to/ALEPH)
./install.sh /path/to/ALEPH

# Or install specific apps/modules
./install.sh -a bees -m lines,waves /path/to/ALEPH
```

---

## Submodule: libavr32

This project uses a custom branch of libavr32 with CDC and compatibility fixes.

**Important**: The submodule is configured to track `aleph-cdc-compat` branch.

```bash
# Verify submodule state
cd libavr32
git branch  # Should show aleph-cdc-compat
cd ..

# If submodule is in wrong state
git submodule update --init
```

---

## Firmware Versions

Firmware version is tracked in `apps/bees/version.mk`:

```makefile
maj = 0
min = 8
rev = 3
```

Current version: **0.8.3**

---

## Output Files

### BEES Firmware

- **apps/bees/aleph-bees.hex** - Flash to aleph controller
- **apps/bees/aleph-bees.elf** - Debug symbols

### DSP Modules

- **modules/MODULE_NAME/MODULE_NAME.ldr** - Loadable DSP module
- **modules/MODULE_NAME/MODULE_NAME.dsc** - Module descriptor
- **modules/MODULE_NAME/MODULE_NAME.lab** - Module label

---

## Troubleshooting

### "libavr32 submodule not found"

```bash
git submodule update --init --recursive
```

### "avr32-gcc: command not found"

```bash
# Use Docker build method instead
docker run --rm -v "$(pwd):/aleph" -w /aleph/apps/bees aleph-builder make
```

### "File not found" errors during build

Check that you're using correct Docker platform:

```bash
docker run --platform=linux/amd64 ...
```

---

## More Information

For detailed build documentation, troubleshooting, and development setup, see the `develop` branch:

- `development/docs/building/BUILDING_BEES_GUIDE.md` - Comprehensive build guide
- `development/docs/getting-started/GETTING_STARTED.md` - Development setup
- `development/docs/workflow/` - Git workflow and daily development practices

---

**Last Updated**: January 11, 2026  
**Firmware Version**: 0.8.3
