# Building BEES Firmware - Complete Guide

**Last Updated**: 2026-01-10  
**Author**: Brett Gershon  
**Purpose**: Definitive guide for building BEES firmware with persistent, working build system

---

## Table of Contents

1. [Critical Understanding](#critical-understanding)
2. [Quick Start](#quick-start)
3. [Prerequisites](#prerequisites)
4. [Build Process](#build-process)
5. [libavr32 Submodule Management](#libavr32-submodule-management)
6. [Common Issues & Solutions](#common-issues--solutions)
7. [Verification](#verification)

---

## Critical Understanding

### Why the Build Breaks

**The Problem**: libavr32 is a Git submodule that keeps reverting to upstream state.

- **libavr32** = Git submodule from https://github.com/monome/libavr32
- We need CUSTOM changes for aleph compatibility (CDC, print_funcs fixes)
- Standard `git submodule update` REVERTS our changes
- Solution: Track our custom branch `aleph-cdc-compat`

### The Fix (Already Applied)

1. ✅ Created `aleph-cdc-compat` branch in libavr32 with our patches
2. ✅ Updated `.gitmodules` to track this branch
3. ✅ Committed libavr32 state to parent repo
4. ✅ Documented build process (this file)

**Going forward**: libavr32 will stay on `aleph-cdc-compat` branch across all your branches.

---

## Quick Start

### One-Command Build (macOS M1/M2/M3)

```bash
cd /path/to/aleph

docker run --rm -v "\$(pwd):/host" -w /tmp aleph-builder bash -c \
  "cp -r /host /tmp/aleph && cd /tmp/aleph/apps/bees && make && \
   cp aleph-bees.hex aleph-bees.elf /host/apps/bees/"
```

**Why copy to /tmp?** macOS Docker has permissions issues writing dependency files to mounted volumes.

**Output**: `apps/bees/aleph-bees.hex` (ready to flash)

---

## Prerequisites

### 1. Docker Image

```bash
docker images | grep aleph-builder
# Should show: aleph-builder   latest    582e5c8141a1
```

### 2. Submodule: libavr32 on aleph-cdc-compat

**VERIFY EVERY TIME BEFORE BUILDING**:

```bash
cd libavr32
git branch  # Should show: * (HEAD detached at 02469a2)
git log -1 --oneline  # Should show: 02469a2 Fix aleph build compatibility
git status  # Should show: nothing to commit, working tree clean
```

**If wrong**, see [libavr32 Submodule Management](#libavr32-submodule-management).

### 3. Required Files

```bash
ls apps/bees/aleph-bees.lds        # Linker script
ls libavr32/src/monome_transport.c # CDC support
```

---

## Build Process

### Method 1: Full Build (Recommended)

```bash
# From aleph root
docker run --rm -v "\$(pwd):/host" -w /tmp aleph-builder bash -c \
  "cp -r /host /tmp/aleph && cd /tmp/aleph/apps/bees && make && \
   cp aleph-bees.* /host/apps/bees/"
```

Time: ~2-3 minutes

### Method 2: Interactive (for debugging)

```bash
docker run --rm -it -v "\$(pwd):/host" -w /tmp aleph-builder bash

# Inside container:
cp -r /host /tmp/aleph
cd /tmp/aleph/apps/bees
make

# Test single file
make ../../apps/bees/src/scene_convert.o

# Copy back
cp aleph-bees.* /host/apps/bees/
```

### Method 3: Quick Syntax Check

```bash
# Check if scene_convert.c compiles
docker run --rm -v "\$(pwd):/host" -w /tmp aleph-builder bash -c \
  "cp -r /host /tmp/aleph && cd /tmp/aleph/apps/bees && \
   make ../../apps/bees/src/scene_convert.o"
```

---

## libavr32 Submodule Management

### Understanding the Custom Branch

**Upstream libavr32** (monome/libavr32) is for Ansible/Teletype/Trilogy.  
**Our branch** (aleph-cdc-compat) has patches for aleph:

1. Commented out `print_funcs.h` includes (conflicts with aleph)
2. Added `monome_transport.c/h` for CDC grid support
3. Fixed type issues (U32 → u32)
4. Removed incompatible debug calls

### Verifying Submodule State

**Run this BEFORE every build**:

```bash
cd libavr32
git status
```

**Expected output:**
```
HEAD detached at 02469a2
nothing to commit, working tree clean
```

**If you see:**
- `modified:` files → You have uncommitted changes
- Different commit → Wrong version checked out
- `working tree clean` ✅ → Good to build

### Fixing Submodule Issues

#### Issue: libavr32 has uncommitted changes

```bash
cd libavr32
git status  # Shows modified files

# See what changed
git diff

# Option A: Discard if wrong
git checkout .
git clean -fd

# Option B: Commit to aleph-cdc-compat
git checkout aleph-cdc-compat
git add -A
git commit -m "Fix: description"

# Update parent to track new commit
cd ..
git add libavr32
git commit -m "Update libavr32 to latest aleph-cdc-compat"
```

#### Issue: Wrong commit checked out

```bash
cd libavr32

# Check what's there
git log -1 --oneline

# Go to correct commit
git checkout 02469a2

# Or checkout the branch
git checkout aleph-cdc-compat

# Parent should already track this, but verify
cd ..
git status  # Should NOT show "modified: libavr32"
```

#### Issue: Parent repo shows "modified: libavr32"

```bash
# From aleph root
git status

# If shows "modified: libavr32 (new commits)"
cd libavr32
git log -1 --oneline  # Note the commit hash

# If it's 02469a2 or later aleph-cdc-compat commit, update parent:
cd ..
git add libavr32
git commit -m "Update libavr32 to aleph-cdc-compat@COMMITHASH"

# If it's wrong commit, go back to 02469a2
cd libavr32
git checkout 02469a2
```

### After Switching Branches

When you `git checkout` a different aleph branch:

```bash
# After checkout
git submodule update --init

# THEN verify libavr32
cd libavr32
git status  # Should be on 02469a2
git checkout aleph-cdc-compat  # If detached

# If it reverted to wrong commit:
git checkout 02469a2
```

---

## Common Issues & Solutions

### Issue 1: "No rule to make target 'clean'"

**Don't use `make deploy` or `make clean`**. Just use `make`.

### Issue 2: "Operation not permitted" on .d files

macOS Docker permission issue. Use Method 1 (build in /tmp).

### Issue 3: "print_funcs.h" errors

Your libavr32 is on wrong branch/commit.

```bash
cd libavr32
git checkout 02469a2
```

### Issue 4: Duplicate enum 'eOpCkdiv'

Fixed in `apps/bees/src/op.h`. If you see this, your branch is wrong.

### Issue 5: Missing aleph-bees.lds

```bash
git checkout HEAD -- apps/bees/aleph-bees.lds
```

### Issue 6: "eOpMidiOutCc undeclared"

Fixed in OPERATOR_OUTPUT_CHANGES.h (capital CC). Branch is wrong.

### Issue 7: Build worked before, broken now

**Most likely**: libavr32 submodule reverted.

```bash
cd libavr32
git status
# If not on 02469a2:
git checkout 02469a2
```

---

## Verification

### Successful Build

1. No errors in output
2. Files created:
   ```bash
   ls -lh apps/bees/aleph-bees.hex  # ~471KB
   ```

3. Hex file is valid:
   ```bash
   head -1 apps/bees/aleph-bees.hex
   # Should start with: :020000040000FA
   ```

### Size Check

```bash
docker run --rm -v "\$(pwd):/host" aleph-builder \
  avr32-size /host/apps/bees/aleph-bees.elf
```

Expected:
```
text       data        bss        dec        hex    filename
0x28dec    0x1cd4    0x403185c  67486492  405c31c  aleph-bees.elf
```

---

## Quick Reference

### Before Every Build
```bash
cd libavr32 && git status && cd ..
# Verify: HEAD detached at 02469a2, working tree clean
```

### Build Command
```bash
docker run --rm -v "\$(pwd):/host" -w /tmp aleph-builder bash -c \
  "cp -r /host /tmp/aleph && cd /tmp/aleph/apps/bees && make && \
   cp aleph-bees.hex /host/apps/bees/"
```

### If Build Fails
1. Check libavr32: `cd libavr32 && git status`
2. Fix if needed: `git checkout 02469a2`
3. Retry build

### After Branch Switch
```bash
git checkout your-branch
git submodule update --init
cd libavr32 && git checkout 02469a2 && cd ..
```

---

## Development Workflow

1. **Check submodule** (every time):
   ```bash
   cd libavr32 && git status && cd ..
   ```

2. **Edit code** on host (macOS):
   ```bash
   code apps/bees/src/scene_convert.c
   ```

3. **Build**:
   ```bash
   # Use quick start command above
   ```

4. **Flash**:
   ```bash
   cp apps/bees/aleph-bees.hex /Volumes/ALEPH/hex/
   ```

---

## What Changed (2026-01-10)

### libavr32 (aleph-cdc-compat branch, commit 02469a2)
- `src/adc.c`: Commented `#include "print_funcs.h"`
- `src/events.c`: Commented `#include "print_funcs.h"`
- `src/font.c`: Commented include, fixed U32→u32, removed print_dbg call
- `src/i2c.c`: Commented `#include "print_funcs.h"`
- `src/monome_transport.c/h`: Added from avr32/src for CDC support

### aleph repo
- `apps/aleph_avr32_src.mk`: Removed adc.c (incompatible)
- `apps/bees/src/op.h`: Removed duplicate enums
- `apps/bees/src/scene_convert.c`: Added op.h include, removed print_funcs
- `apps/bees/src/OPERATOR_ID_MAPPING.h`: Added stddef.h
- `apps/bees/src/OPERATOR_OUTPUT_CHANGES.h`: Fixed eOpMidiOutCC typo
- `.gitmodules`: Added branch = aleph-cdc-compat

---

## Support

**If build still fails**:

1. Verify submodule: `cd libavr32 && git log -1 --oneline`
2. Should show: `02469a2 Fix aleph build compatibility`
3. If not: `git checkout 02469a2`
4. Check this guide was followed

**Last known working**:
- Date: 2026-01-10
- aleph: feature/scene-migration-0.7.1-to-0.8.x  
- libavr32: 02469a2 (aleph-cdc-compat)
- scene_convert.c compiles successfully

---

**KEY TAKEAWAY**: Always verify libavr32 is on commit 02469a2 before building!

