# Aleph Firmware Build Guide

**Repository**: `btrailor/aleph`  
**Active Branch**: `develop`  
**Builder Image**: `aleph-builder:latest` (Docker, Ubuntu 24.04 base)  
**Backup Location**: `NAS:/volume1/docker/aleph-builder-backup.tar.gz` (457MB)

---

## Quick Reference

| Component | Architecture | Toolchain Location | Source |
|-----------|-------------|-------------------|--------|
| AVR32 (controller) | `linux/amd64` | `~/avr32-toolchain-linux/bin/` | avr32-gnu-toolchain 3.4.3.820 |
| Blackfin BF533 (DSP) | `i386` | `/opt/uClinux/bfin-elf/bin/` | ADI 2014R1-RC2 |
| Base Image | `amd64` | — | Ubuntu 24.04 |

---

## Prerequisites

### 1. Docker Runtime

The builder image is `linux/amd64`. On macOS Apple Silicon, Docker must emulate x86_64:

```bash
# Verify Docker is running
docker version

# On macOS arm64, ensure Rosetta 2 is installed
softwareupdate --install-rosetta --agree-to-license
```

> **Historical Note**: Docker 20.10.23 had an osxfs volume mount bug causing files to appear empty inside containers. This was resolved by Docker 29.x. Your current Docker Desktop is 29.4.0 — this issue should be resolved.

### 2. Import the Builder Image

The image is backed up on the NAS. Import it:

```bash
# From NAS (requires SSH or mounted volume)
scp Brett.Gershon@192.168.1.199:/volume1/docker/aleph-builder-backup.tar.gz ./
docker load -i aleph-builder-backup.tar.gz

# Verify
docker images | grep aleph-builder
# aleph-builder   latest    [ID]    457MB
```

### 3. Verify libavr32 Submodule

```bash
cd libavr32
git log -1 --oneline
# Should show: 02469a2 Fix aleph build compatibility (or newer on aleph-cdc-compat)
git status
# Should be clean
cd ..
```

---

## Building BEES Firmware (AVR32 Controller)

### Full Build

```bash
cd ~/aleph-repo

docker run --rm -v "$(pwd):/host" -w /tmp aleph-builder bash -c \
  "cp -r /host /tmp/aleph && cd /tmp/aleph/apps/bees && make && \
   cp aleph-bees.hex aleph-bees.elf /host/apps/bees/"
```

**Why copy to `/tmp`?** macOS Docker has permissions issues writing dependency files (`.d`) to mounted volumes. Building in `/tmp` and copying outputs back avoids this.

### Verify Output

```bash
ls -lh apps/bees/aleph-bees.hex
# Expected: ~480-500KB

ls -lh apps/bees/aleph-bees.elf
# Expected: ~980KB
```

### Build Individual Components

```bash
# Just the bees app (inside container)
cd /aleph/apps/bees
make

# Clean rebuild
make clean && make
```

---

## Building DSP Modules (Blackfin)

The Blackfin toolchain is also in the same container:

```bash
docker run --rm -v "$(pwd):/host" -w /tmp aleph-builder bash -c \
  "cp -r /host /tmp/aleph && cd /tmp/aleph/modules/lines && make && \
   cp lines.ldr /host/modules/lines/"
```

Available modules:
- `modules/lines/` — Delay/loop processor
- `modules/mix/` — Mixer
- `modules/waves/` — Wavetable synth
- `modules/fmsynth/` — FM synthesizer
- `modules/pitch_shift/` — Pitch shifter (newer)

---

## Image Maintenance

### Where the Image Lives

| Location | Path | Purpose |
|----------|------|---------|
| **Primary Backup** | `NAS:/volume1/docker/aleph-builder-backup.tar.gz` | Archive |
| **Local** | Docker daemon image cache | Active use |

### Backup the Image

After any image modification:

```bash
# Export current image
docker save aleph-builder:latest | gzip > aleph-builder-backup.tar.gz

# Copy to NAS
scp aleph-builder-backup.tar.gz Brett.Gershon@192.168.1.199:/volume1/docker/
```

### Image Contents Summary

The `aleph-builder` image contains:
- Ubuntu 24.04 base
- GCC, build-essential, gcc-multilib
- AVR32 toolchain (3.4.3.820, x86_64): `~/avr32-toolchain-linux/bin/`
- AVR32 headers (6.2.0.742)
- Blackfin toolchain (2014R1-RC2, i386): `/opt/uClinux/bfin-elf/bin/`
- git, wget, ripgrep, fd-find
- LuaRocks with luafilesystem, lpack, md5

Built: 2026-01-12  
Architecture: `linux/amd64`

---

## Troubleshooting

### Files Empty Inside Container

**Symptom**: `file Makefile` shows "empty" despite existing on host.

**Cause**: Docker volume mount bug (older Docker versions on macOS arm64).

**Fix**: Update Docker Desktop to 29.x+. The workaround (copy to `/tmp`) remains valid.

### libavr32 Changes Lost

**Symptom**: Build breaks after `git checkout` or `git submodule update`.

**Cause**: Uncommitted libavr32 changes get wiped.

**Fix**: Always commit libavr32 changes to `aleph-cdc-compat` branch:

```bash
cd libavr32
git add .
git commit -m "Fix: [description]"
git push origin aleph-cdc-compat
cd ..
git add libavr32
git commit -m "Update libavr32: [description]"
```

### Build Fails with Missing Headers

**Symptom**: `avr32/io.h` or similar not found.

**Fix**: Verify AVR32 headers were copied correctly:

```bash
cd libavr32
ls avr32/include/avr32/
# Should show: compiler.h, gpio.h, etc.
```

### Blackfin Build Failures

**Symptom**: `bfin-elf-gcc` not found or segfaults.

**Cause**: The Blackfin toolchain is 32-bit (i386) running on amd64. It requires 32-bit compatibility libraries.

**Fix**: Ensure `lib32z1` and `gcc-multilib` are installed in the container (they are in the current image).

---

## Reference: Build Commands

```bash
# Start interactive container for debugging
docker run --rm -it -v "$(pwd):/host" aleph-builder bash
cd /host/apps/bees
make

# Build with verbose output
make V=1

# Build specific target
make aleph-bees.hex

# Flash to Aleph (requires avrdude + hardware)
# See utils/avr32_boot/ documentation
```

---

## Historical Context

- **2025-11**: Phase 0 environment established. Dual-container setup (ARM64 for AVR32, x86_64 for Blackfin).
- **2026-01-10**: Docker 20.10.23 volume mount bug discovered on macOS arm64. Files read as empty.
- **2026-01-11**: Stability recovery documented. `develop` branch established as stable baseline. Image built.
- **2026-01-12**: `aleph-builder` image created with both toolchains in single container. Exported as backup.
- **2026-05-27**: Docker Desktop updated to 29.4.0. Volume mount bug should be resolved.

---

## Next Steps After Building

1. **Test on hardware**: Flash `aleph-bees.hex` to Aleph via bootloader
2. **Verify CDC grid**: Connect modern monome grid (VID 0x0483), check serial output
3. **Test scene loading**: Load v0.8.x scenes, verify operator ID validation
4. **Implement converter**: Finish `scene_convert_from_0_7_1()` if not yet complete
