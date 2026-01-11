# Building BEES Firmware - Complete Guide

**Last Updated**: 2026-01-11  
**Author**: Brett Gershon  
**Purpose**: Definitive guide for building BEES firmware with persistent, working build system

---

## ⚠️ CRITICAL ISSUE (2026-01-11)

**Docker Volume Mount Failure on macOS arm64**

Docker 20.10.23 on macOS cannot read files through volume mounts when emulating amd64:

- Files show correct size but are empty when read from container
- Affects ALL builds - not specific to our code
- Root cause: osxfs driver bug in older Docker versions

**Solutions**:

1. **Update Docker Desktop** to 24.x+ (recommended)
2. **Build native arm64 Docker image** with AVR32 toolchain
3. **Use Linux machine** for builds (VM, remote server, or GitHub Actions)

See [DEVELOPMENT_STABILITY_STRATEGY.md](DEVELOPMENT_STABILITY_STRATEGY.md) for details.

**Workaround**: Use Jan 10 23:32 build (`apps/bees/aleph-bees.hex`) until Docker is updated.

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

### Issue 8: USB Driver Link Errors (uhi_ftdi_install, uhi_cdc_enable, etc.)

**Symptom**: Linker errors like:

```
undefined reference to `uhi_ftdi_install'
undefined reference to `uhi_cdc_enable'
undefined reference to `uhi_ftdi_uninstall'
```

**Root Cause**: USB driver source files not included in makefile.

**Solution**: Verify `apps/aleph_avr32_src.mk` contains:

```makefile
$(LIB_AVR32)src/usb/hid/hid.c \
$(LIB_AVR32)src/usb/hid/uhi_hid.c \
$(LIB_AVR32)src/usb/ftdi/ftdi.c \
$(LIB_AVR32)src/usb/ftdi/uhi_ftdi.c \
$(LIB_AVR32)src/usb/cdc/cdc.c \
$(LIB_AVR32)src/usb/cdc/uhi_cdc.c \
$(LIB_AVR32)src/usb/midi/uhi_midi.c \
$(LIB_AVR32)src/usb/midi/midi.c \
```

**Also verify** CDC class is in include path:

```makefile
common/services/usb/class/cdc                      \
```

### Issue 9: USB Circular Dependency Errors

**Symptom**: Compile errors like:

```
error: conflicting types for 'uhi_hid_install'
warning: "UHI_HID" redefined
```

**Root Cause**: Circular include dependency between `conf_usb_host.h` and UHI headers.

**Solution**: Verify `libavr32/conf/aleph/conf_usb_host.h` structure:

1. UHI callback macros defined BEFORE includes:

```c
// ftdi functions
#define UHI_FTDI_CHANGE(dev, b_plug) ftdi_change(dev, b_plug)

// cdc functions
#define UHI_CDC_CHANGE(dev, b_plug) cdc_change(dev, b_plug)

// midi functions
#define UHI_MIDI_CHANGE(dev, b_plug) midi_change(dev, b_plug)

// hid functions
#define UHI_HID_CHANGE(dev, b_plug) hid_change(dev, b_plug)
```

2. UHI headers included at END of file:

```c
#include "uhi_ftdi.h"
#include "uhi_cdc.h"
#include "uhi_hid.h"
#include "uhi_midi.h"

// USB_HOST_UHI must be defined AFTER includes
#define USB_HOST_UHI        UHI_FTDI , UHI_CDC , UHI_HID , UHI_MIDI

#endif // _CONF_USB_HOST_H_
```

**Why this order?**

- UHI headers include `conf_usb_host.h` (creating circular dependency)
- Include guards prevent infinite loop
- But UHI macros must be defined by headers BEFORE being used in USB_HOST_UHI
- Moving USB_HOST_UHI definition after includes ensures macros exist

### Issue 10: Missing USB Callback Declarations

**Symptom**: Implicit declaration errors:

```
error: implicit declaration of function 'usb_mode_change'
error: implicit declaration of function 'usb_vbus_error'
error: implicit declaration of function 'usb_sof'
```

**Root Cause**: USB hardware layer needs callback function declarations.

**Solution**: Verify these files include `"usb.h"`:

1. `libavr32/asf/avr32/drivers/usbb/usbb_host.c`:

```c
#include "conf_usb_host.h"
#include "usb.h"  // ← Must be here
#include "sysclk.h"
```

2. `libavr32/asf/common/services/usb/uhc/uhc.c`:

```c
#include "conf_usb_host.h"
#include "usb.h"  // ← Must be here
#include "usb_protocol.h"
```

### Issue 11: CDC Implementation Missing

**Symptom**: Link errors for `cdc_setup`, `cdc_disconnect`, `cdc_read`:

```
undefined reference to `cdc_setup'
undefined reference to `cdc_disconnect'
```

**Root Cause**: CDC implementation files not cherry-picked from upstream.

**Solution**: Ensure libavr32 includes CDC implementation (commit bc43976):

```bash
cd libavr32
ls src/usb/cdc/
# Should show: cdc.c  cdc.h  uhi_cdc.c  uhi_cdc.h

# If missing:
git cherry-pick bc43976
```

### Issue 12: ADC Link Errors

**Symptom**:

```
undefined reference to `adc_convert'
undefined reference to `init_adc'
```

**Root Cause**: ADC source file not in makefile.

**Solution**: Verify `apps/aleph_avr32_src.mk` contains:

```makefile
$(LIB_AVR32)src/adc.c \
```

---

## USB Support Architecture

### Four USB Interfaces

The aleph build requires ALL four USB host interfaces:

1. **FTDI** - Legacy USB-to-serial adapters
2. **CDC** - Modern monome grids (USB-C devices)
3. **HID** - Human interface devices (keyboards, etc.)
4. **MIDI** - MIDI controllers

**Critical**: All four must be in the build or linking will fail.

### File Structure

Each interface needs two files:

```
libavr32/src/usb/
├── ftdi/
│   ├── ftdi.c          # High-level FTDI driver
│   ├── ftdi.h
│   ├── uhi_ftdi.c      # USB Host Interface implementation
│   └── uhi_ftdi.h
├── cdc/
│   ├── cdc.c           # High-level CDC driver
│   ├── cdc.h
│   ├── uhi_cdc.c       # USB Host Interface implementation
│   └── uhi_cdc.h
├── hid/
│   ├── hid.c
│   ├── hid.h
│   ├── uhi_hid.c
│   └── uhi_hid.h
└── midi/
    ├── midi.c
    ├── midi.h
    ├── uhi_midi.c
    └── uhi_midi.h
```

### Makefile Requirements

**Three places need updates for each USB interface:**

1. **Source files** in `apps/aleph_avr32_src.mk`:

```makefile
CSRCS += \
  $(LIB_AVR32)src/usb/ftdi/ftdi.c \
  $(LIB_AVR32)src/usb/ftdi/uhi_ftdi.c \
  $(LIB_AVR32)src/usb/cdc/cdc.c \
  $(LIB_AVR32)src/usb/cdc/uhi_cdc.c \
  # etc...
```

2. **Include paths** in `apps/aleph_avr32_src.mk`:

```makefile
INC_PATH += \
  $(LIB_AVR32)src/usb/ftdi \
  $(LIB_AVR32)src/usb/cdc \
  common/services/usb/class/cdc \  # ASF CDC class
  # etc...
```

3. **USB_HOST_UHI** in `libavr32/conf/aleph/conf_usb_host.h`:

```c
#define USB_HOST_UHI  UHI_FTDI , UHI_CDC , UHI_HID , UHI_MIDI
```

**Order matters**: FTDI, CDC, HID, MIDI (matches working commit 66ec7de).

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

### Build System Updates - Session 2: Full USB Support

**Problem Solved**: Added complete USB support (FTDI, CDC, HID, MIDI) with circular dependency resolution.

#### libavr32 Changes (commit 5744d73 on aleph-cdc-compat)

- **New files**: Cherry-picked CDC implementation from commit bc43976
  - `src/usb/cdc/cdc.c` - CDC driver implementation
  - `src/usb/cdc/cdc.h` - CDC driver interface
  - `src/usb/cdc/uhi_cdc.c` - USB Host Interface for CDC
  - `src/usb/cdc/uhi_cdc.h` - UHI CDC header
- `src/usb/cdc/cdc.c`: Added `cdc_disconnect()` function for cleanup
- `conf/aleph/conf_usb_host.h`:
  - Moved `USB_HOST_UHI` definition AFTER UHI header includes (fixes circular dependency)
  - Driver order: UHI_FTDI, UHI_CDC, UHI_HID, UHI_MIDI
- `asf/avr32/drivers/usbb/usbb_host.c`: Added `#include "usb.h"` for USB callbacks
- `asf/common/services/usb/uhc/uhc.c`: Added `#include "usb.h"` for USB callbacks

#### aleph repo Changes

- `apps/aleph_avr32_src.mk`:
  - Added FTDI source files: `$(LIB_AVR32)src/usb/ftdi/ftdi.c`, `uhi_ftdi.c`
  - Added CDC source files: `$(LIB_AVR32)src/usb/cdc/cdc.c`, `uhi_cdc.c`
  - Restored ADC source: `$(LIB_AVR32)src/adc.c` (needed for hardware)
  - Added CDC class to include path: `common/services/usb/class/cdc`
- **Output**: `apps/bees/aleph-bees.hex` (483KB) with full USB support ✅

### Build System Updates - Session 1: Scene Migration

#### libavr32 (aleph-cdc-compat branch, commit 02469a2)

- `src/adc.c`: Commented `#include "print_funcs.h"`
- `src/events.c`: Commented `#include "print_funcs.h"`
- `src/font.c`: Commented include, fixed U32→u32, removed print_dbg call
- `src/i2c.c`: Commented `#include "print_funcs.h"`
- `src/monome_transport.c/h`: Added from avr32/src for CDC support

#### aleph repo

- `apps/bees/src/op.h`: Removed duplicate enums
- `apps/bees/src/scene_convert.c`: Added op.h include, removed print_funcs
- `apps/bees/src/OPERATOR_ID_MAPPING.h`: Added stddef.h
- `apps/bees/src/OPERATOR_OUTPUT_CHANGES.h`: Fixed eOpMidiOutCC typo
- `.gitmodules`: Added branch = aleph-cdc-compat

---

## Scene Conversion & v0.8 Compatibility

### Purpose of scene_convert.c

**File**: `apps/bees/src/scene_convert.c`

**What it does**: Converts scene files from v0.7.1 format to v0.8.x format, enabling legacy scenes to load in new firmware.

**Why needed**: v0.8 introduced:

1. New operators (op_param, etc.)
2. Operator enum order changes
3. Output enum ID changes
4. Binary scene format updates

### How Scene Conversion Works

When loading a scene saved in v0.7.1:

1. **scene.c** detects old version number in scene file header
2. Calls `scene_convert_from_0_7_1()` in **scene_convert.c**
3. Converts operator IDs using `OPERATOR_ID_MAPPING.h` lookup table
4. Converts output IDs using `OPERATOR_OUTPUT_CHANGES.h` mapping
5. Updates scene version number to 0.8.x
6. Scene can now load in v0.8 firmware with correct operator types

### Mapping Files

#### OPERATOR_ID_MAPPING.h

Maps old operator enum values (array indices) to new enum constants:

```c
// Example: op_ckdiv moved from position 10 to enum value eOpCkdiv (9)
static const op_id_t OPERATOR_ID_MAP[] = {
    [0] = eOpInvalid,
    [1] = eOpAdd,
    // ...
    [10] = eOpCkdiv,  // old position 10 → new enum eOpCkdiv
    // ...
};
```

#### OPERATOR_OUTPUT_CHANGES.h

Maps old output enum values to new ones:

```c
// Example: MIDI CC output ID changed
static const output_change_t OUTPUT_CHANGES[] = {
    {
        .oldId = 42,              // old enum value
        .newId = eOpMidiOutCC,    // new enum (capital CC)
        .outputIndex = 0
    },
    // ...
};
```

### Does This Enable All v0.7.1 Scenes to Load in v0.8?

**Yes, theoretically** - The implementation should enable backward compatibility IF:

1. ✅ `scene_convert.c` compiles successfully (FIXED - builds now)
2. ✅ All operators have correct entries in `OPERATOR_ID_MAPPING.h`
3. ✅ All output changes documented in `OPERATOR_OUTPUT_CHANGES.h`
4. ✅ Version detection logic works (`scene.c` checks version)
5. ⚠️ **Testing required**: Real v0.7.1 scenes must be loaded to verify

**What's been done**:

- ✅ Build system fixed - firmware compiles with scene_convert.c
- ✅ Mapping tables created for operator and output ID translation
- ✅ Conversion logic implemented in scene_convert.c

**Remaining work for 100% compatibility**:

- **Verification**: Load actual v0.7.1 scene files on hardware with v0.8 firmware
- **Edge cases**: Test complex scenes with many operators and connections
- **Data validation**: Ensure converted operator parameters don't cause crashes
- **Fallback handling**: Graceful degradation if conversion fails
- **Logging**: Monitor serial output for conversion messages and errors

### Testing Scene Conversion

To verify all scenes load correctly:

1. **Build firmware** with scene_convert.c (✅ DONE - hex file created)
2. **Flash to hardware**: `cp apps/bees/aleph-bees.hex /Volumes/ALEPH/`
3. **Load v0.7.1 scenes** from SD card
4. **Monitor serial output** for:
   - Version mismatch detection
   - Conversion start/completion messages
   - Operator creation success/failure
   - Connection restoration
   - Parameter validation

**Key log patterns to watch**:

```
scene_load: version mismatch detected
scene_convert_from_0_7_1: starting conversion
op_init: creating operator type X (mapped from Y)
scene_load: conversion complete, X operators created
```

**Success criteria**:

- Scene loads without errors
- All operators present with correct types
- Connections between operators work
- Parameters preserved correctly
- Presets recall properly

---

## Colima/Docker File System Cache Issues

### Symptom: "File Shorter Than Expected"

If you see compilation errors like:

```
warning: ../../common/module_common.h is shorter than expected
error: 'MODULE_NAME_LEN' undeclared here (not in a function)
```

And files show 0 lines inside Docker but normal size on host:

```bash
# Inside container - WRONG:
$ wc -l /aleph/common/module_common.h
0 /aleph/common/module_common.h

# On host - correct:
$ wc -l common/module_common.h
18 common/module_common.h
```

### Root Cause

**Colima sshfs mount has stale file system cache**. This happens after:

- Directory renames or moves
- Extended periods without restart
- File operations that don't sync with VM

### Fix: Restart Colima

```bash
colima stop && sleep 3 && colima start
```

Wait 10-15 seconds for full startup, then retry build.

### Verification

Test file readability inside Docker:

```bash
docker run --rm -v "$(pwd):/aleph" aleph-builder:latest \
  bash -c "wc -l /aleph/common/module_common.h"
```

Should show correct line count (not 0).

### Alternative: Use VirtioFS

Switch Colima to VirtioFS (more reliable than sshfs):

```bash
colima delete  # Warning: destroys VM
colima start --mount-type virtiofs --vm-type vz
```

---

## Support

**If build still fails**:

1. Verify submodule: `cd libavr32 && git log -1 --oneline`
2. Should show: `02469a2 Fix aleph build compatibility` (or later on aleph-cdc-compat)
3. If not: `git checkout aleph-cdc-compat`
4. Verify USB source files in makefile (see Issue 8 above)
5. Check this guide was followed

**Last known working**:

- Date: 2026-01-10 (Session 2 - Full USB support)
- aleph: feature/scene-migration-0.7.1-to-0.8.x
- libavr32: 5744d73 (aleph-cdc-compat) with CDC implementation
- Output: apps/bees/aleph-bees.hex (483KB) with FTDI+CDC+HID+MIDI support ✅

---

**KEY TAKEAWAYS**:

1. Always verify libavr32 is on aleph-cdc-compat branch before building
2. All four USB interfaces (FTDI, CDC, HID, MIDI) must be in makefile
3. USB_HOST_UHI must be defined AFTER UHI header includes (circular dependency fix)
