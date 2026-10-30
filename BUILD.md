# Building Aleph Firmware

Quick guide for building BEES firmware and DSP modules.

---

## Quick Build (Docker)

### Prerequisites

- Docker Desktop running (`open -a Docker`, then wait for the daemon)
- Git submodules initialized: `git submodule update --init --recursive`

### Toolchain image

**`aleph-builder` is the only toolchain image this project uses**, and it carries
*both* compilers:

| Toolchain | Path in image | Version |
|---|---|---|
| AVR32 | `/root/avr32-toolchain-linux/bin` | GCC 4.4.7 (GNU AVR32 3.4.3.820) |
| Blackfin | `/opt/uClinux/bfin-elf/bin` | GCC 4.3.5 (ADI-2014R1-RC2) |

Neither is on the image's default `PATH` — you must export it. The image is
`linux/amd64`, so it needs `--platform linux/amd64` on Apple Silicon.

```bash
# Verify the image and both toolchains
docker run --rm --platform=linux/amd64 aleph-builder bash -c \
  'export PATH=/root/avr32-toolchain-linux/bin:/opt/uClinux/bfin-elf/bin:$PATH && \
   avr32-gcc --version | head -1 && bfin-elf-gcc --version | head -1'
```

### Build BEES Firmware

```bash
# Build with Docker (build in /tmp for speed, copy artifacts back)
docker run --rm --platform=linux/amd64 \
  -v "$(pwd):/host" -w /tmp \
  aleph-builder bash -c '
    cp -r /host /tmp/aleph && cd /tmp/aleph/apps/bees && \
    export PATH=/root/avr32-toolchain-linux/bin:/opt/uClinux/bfin-elf/bin:$PATH && \
    make && cp aleph-bees.hex aleph-bees.elf /host/apps/bees/'

# Output: apps/bees/aleph-bees.hex
```

### Build DSP Module (example: lines)

Same image, same `PATH` export — the Blackfin compiler is already in it:

```bash
docker run --rm --platform=linux/amd64 \
  -v "$(pwd):/aleph" -w /aleph/modules/lines \
  aleph-builder bash -c \
  "export PATH=/root/avr32-toolchain-linux/bin:/opt/uClinux/bfin-elf/bin:\$PATH && make"

# Output: modules/lines/lines.ldr
```

---

## Build Environment Setup

### Toolchain Image

`aleph-builder` is the single image for both AVR32 and Blackfin builds. It ships
with the toolchains pre-installed — you do **not** need to build or pull an
image.

```bash
# Only if the image is genuinely missing from your machine:
cd development/environment
docker build -t aleph-builder .
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

This project pins libavr32 to a **fork** branch carrying the CDC and
compatibility fixes.

**Important**: the pinned commit lives on `btrailor/libavr32`, branch
`aleph-develop-substrate`. It is **not** on upstream `monome/libavr32` `main`.
Note that `.gitmodules` currently declares the *upstream* URL and `branch = main`,
which does not match where the pin actually lives — a fresh
`git submodule update --init` against that URL will not find commit `162b653`.
Fixing that mismatch is tracked as an open item.

```bash
# Verify submodule state
cd libavr32
git branch          # Should show aleph-develop-substrate
git rev-parse HEAD # Should be 162b653c1462ec50e7132ad59f18d39c40bfa2a2
cd ..

# If submodule is in wrong state
git submodule update --init
```

### Verify a fresh checkout actually builds

The Phase A goal is that anyone, on any machine, can produce a flashable build
from GitHub alone:

```bash
git clone git@github.com:btrailor/aleph.git && cd aleph
git submodule update --init --recursive
cd apps/bees && make
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

The toolchain is inside the image but not on its default `PATH`. Export it:

```bash
docker run --rm --platform=linux/amd64 -v "$(pwd):/aleph" -w /aleph/apps/bees \
  aleph-builder bash -c \
  "export PATH=/root/avr32-toolchain-linux/bin:/opt/uClinux/bfin-elf/bin:\$PATH && make"
```

### "make: Nothing to be done for 'all'"

Stale artifacts. Force a clean rebuild:

```bash
docker run --rm --platform=linux/amd64 -v "$(pwd):/host" -w /tmp \
  aleph-builder bash -c '
    cp -r /host /tmp/aleph && cd /tmp/aleph/apps/bees && \
    export PATH=/root/avr32-toolchain-linux/bin:/opt/uClinux/bfin-elf/bin:$PATH && \
    make clean && make'
```

### "Cannot connect to the Docker daemon"

Docker Desktop isn't running:

```bash
open -a Docker   # then wait for the daemon to come up
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
