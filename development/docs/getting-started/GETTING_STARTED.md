# Getting Started with Aleph Development

**Last Updated**: January 11, 2026

This guide will get you from zero to building BEES firmware and understanding the development workflow.

---

## Table of Contents

1. [Understanding the Git Workflow](#understanding-the-git-workflow)
2. [Quick Start](#quick-start)
3. [Development Environment](#development-environment)
4. [Building Firmware](#building-firmware)
5. [Troubleshooting](#troubleshooting)

---

## Understanding the Git Workflow

### Upstream Repository (monome/aleph)

**Two branches you need to know about**:

- **`dev` branch** → BEES 0.8.x (2018) - **USE THIS** ✅

  - Dynamic memory management (PR #267)
  - CV output fixes (Issue #294)
  - Stable preset system
  - **This is the current working version**

- **`master` branch** → BEES 0.7.1 (2014) - **DON'T USE** ❌
  - Old version with known issues
  - Missing critical fixes
  - Only useful for historical reference

### Your Fork (btrailor/aleph)

**Your active branches**:

- **`develop` branch** → Your primary development branch ⭐

  - Created from feature/scene-migration-0.7.1-to-0.8.x
  - Contains all your work: CDC support, scene migration, build fixes
  - Tracks `fork/develop` (your remote)
  - **This is where you do all new work**

- **`main` branch** → Scene migration analysis

  - Contains operator mapping analysis
  - Scene conversion framework
  - Ahead 145 commits from aleph-broken/main

- **`dev` branch** → Tracks upstream origin/dev
  - For pulling updates from monome/aleph
  - Don't work directly on this

### Git Remotes

```bash
origin      → https://github.com/monome/aleph.git (upstream)
fork        → https://github.com/btrailor/aleph.git (your fork)
aleph-broken → git@github.com:btrailor/aleph-broken.git (backup)
```

### Typical Git Workflow

```bash
# 1. Make sure you're on develop
git checkout develop

# 2. Pull any changes from your fork
git pull fork develop

# 3. Do your work, commit changes
git add .
git commit -m "Description of changes"

# 4. Push to your fork
git push fork develop

# 5. To sync with upstream (occasionally)
git fetch origin
git merge origin/dev  # Merge upstream changes into develop
```

### Why This Structure?

- **`develop`** = Your stable working branch with all features integrated
- **`dev`** (upstream) = Monome's official 0.8.x release
- You build on `develop`, which started from 0.8.x + your enhancements

---

## Quick Start

### Prerequisites

- Docker Desktop 24.x+ (or Colima)
- macOS (M1/M2/M3 tested) or Linux
- Git
- At least 8GB free disk space

### 5-Minute Build

```bash
# 1. Clone and checkout your develop branch
git clone https://github.com/btrailor/aleph.git
cd aleph
git checkout develop

# 2. Verify libavr32 submodule
cd libavr32 && git log -1 && cd ..
# Should show: 02469a2 Fix aleph build compatibility

# 3. Build firmware using Docker
docker run --rm -v "$(pwd):/host" -w /tmp aleph-builder bash -c \
  "cp -r /host /tmp/aleph && cd /tmp/aleph/apps/bees && make && \
   cp aleph-bees.hex aleph-bees.elf /host/apps/bees/"
```

**Output**: `apps/bees/aleph-bees.hex` (ready to flash to hardware)

### ⚠️ Critical Build Issue (January 2026)

**Docker 20.10.23 on macOS has a volume mount bug**:

- Files appear empty when read through mounted volumes
- **Solution**: Update to Docker 24.x+ or use Linux
- **Workaround**: Build on Linux VM or use build from Jan 10

See [../building/BUILDING_BEES_GUIDE.md](../building/BUILDING_BEES_GUIDE.md) for complete details.

---

## Development Environment

### Current Setup (January 2026)

Your `develop` branch includes:

- ✅ CDC USB support for modern monome grids (2021+)
- ✅ Scene migration framework (v0.7.1 → v0.8.x)
- ✅ Parameter debug logging
- ✅ Build system fixes
- ✅ libavr32 pinned to aleph-cdc-compat branch

### Docker Image

The `aleph-builder` Docker image includes:

- AVR32 GNU toolchain
- USB host stack
- Build dependencies
- Compatible with both Intel and ARM Macs

**Build the image** (one-time, ~15 minutes):

```bash
cd development/docker
docker build -t aleph-builder .
```

### libavr32 Submodule Management

**Critical**: The libavr32 submodule must stay on `aleph-cdc-compat`:

```bash
# Check current state
cd libavr32
git status
git log -1 --oneline

# Should show:
# HEAD detached at 02469a2
# 02469a2 Fix aleph build compatibility
```

**If libavr32 gets reset** (after git operations):

```bash
git checkout aleph-cdc-compat
cd ..
git add libavr32
git commit -m "Fix: restore libavr32 to aleph-cdc-compat"
```

**Why this matters**: The aleph-cdc-compat branch contains critical fixes:

- CDC USB driver compatibility
- print_funcs.h include fixes
- Modern monome grid support

See [../workflow/DEVELOPMENT_STABILITY_STRATEGY.md](../workflow/DEVELOPMENT_STABILITY_STRATEGY.md) for the full story.

---

## Building Firmware

### Build BEES Controller (AVR32)

**Using Docker** (recommended):

```bash
docker run --rm -v "$(pwd):/host" -w /tmp aleph-builder bash -c \
  "cp -r /host /tmp/aleph && cd /tmp/aleph/apps/bees && make && \
   cp aleph-bees.hex /host/apps/bees/"
```

**Why copy to /tmp?** macOS Docker has permission issues writing to mounted volumes.

**Output**:

- `apps/bees/aleph-bees.hex` (~480KB)
- Ready to flash to Aleph hardware

### Build DSP Modules (Blackfin)

**Using Blackfin Docker image**:

```bash
docker run --rm --platform linux/amd64 \
  -v "$(pwd):/projects/aleph" \
  pf0camino/cross-bfin-elf bash -c \
  "cd /projects/aleph/modules/lines && make"
```

**Note**: DSP module builds are currently experimental.

### Verify Build Output

```bash
# Check hex file was created
ls -lh apps/bees/aleph-bees.hex

# Should show ~480-500KB file

# Check file is not empty (Docker bug check)
head -10 apps/bees/aleph-bees.hex
# Should show Intel HEX format data, not empty
```

---

## Troubleshooting

### Docker Volume Mount Issues

**Symptom**: Build succeeds but hex file is 0 bytes or `head` shows empty

**Cause**: Docker 20.10.23 volume mount bug on macOS arm64

**Solutions**:

1. Update to Docker Desktop 24.x+
2. Use Linux (VM, remote server, or GitHub Actions)
3. Build native arm64 image with AVR32 toolchain

### Colima File System Cache Issues

**Symptom**: Compiler reports "file shorter than expected"

**Cause**: Colima sshfs mount has stale cache

**Fix**:

```bash
colima stop && sleep 3 && colima start
```

### libavr32 Reset

**Symptom**: Build fails with missing symbols or header errors

**Cause**: libavr32 submodule reverted to wrong commit

**Fix**:

```bash
cd libavr32
git checkout aleph-cdc-compat
cd ..
git add libavr32
```

### Common Build Errors

**Error**: `usb_protocol_ftdi.h: No such file`

- **Fix**: libavr32 on wrong branch (see above)

**Error**: `MODULE_NAME_LEN undeclared`

- **Fix**: Restart Colima to clear file cache

**Error**: `/host: permission denied`

- **Fix**: That's why we copy to /tmp first (see build command)

---

## Daily Development Workflow

### Starting a Work Session

```bash
# 1. Checkout develop branch
cd /path/to/aleph
git checkout develop

# 2. Pull latest from your fork
git pull fork develop

# 3. Check libavr32 state
cd libavr32 && git log -1 && cd ..
# Verify it's on 02469a2 or later

# 4. Make your changes
# Edit files with your favorite editor

# 5. Build to test
docker run --rm -v "$(pwd):/host" -w /tmp aleph-builder bash -c \
  "cp -r /host /tmp/aleph && cd /tmp/aleph/apps/bees && make && \
   cp aleph-bees.hex /host/apps/bees/"

# 6. Commit changes
git add .
git commit -m "Description of changes"

# 7. Push to fork
git push fork develop
```

### Quick Build Cycle

For faster iteration during development:

```bash
# Incremental build (faster)
docker run --rm -v "$(pwd):/host" -w /tmp aleph-builder bash -c \
  "cp -r /host /tmp/aleph && cd /tmp/aleph/apps/bees && make"
```

### Before Committing

Always check:

- ✅ Code compiles without errors
- ✅ libavr32 still on aleph-cdc-compat
- ✅ No accidentally added build artifacts
- ✅ Commit message is descriptive

---

## Resources

### Documentation in This Repository

- **Building**: [../building/BUILDING_BEES_GUIDE.md](../building/BUILDING_BEES_GUIDE.md)
- **Workflow**: [../workflow/DEVELOPMENT_CHECKLIST.md](../workflow/DEVELOPMENT_CHECKLIST.md)
- **Git Strategy**: [../workflow/GIT_STRATEGY.md](../workflow/GIT_STRATEGY.md)
- **Stability**: [../workflow/DEVELOPMENT_STABILITY_STRATEGY.md](../workflow/DEVELOPMENT_STABILITY_STRATEGY.md)

### Your Current Work

- **Scene Migration**: [../scene-migration/SCENE_MIGRATION_STRATEGY.md](../scene-migration/SCENE_MIGRATION_STRATEGY.md)
- **CDC Support**: [../history/CDC_MERGE_COMPLETE_2026-01-11.md](../history/CDC_MERGE_COMPLETE_2026-01-11.md)

### External Resources

- **Forum**: https://llllllll.co (lines community)
- **Official Docs**: http://monome.org/docs/aleph
- **Upstream Repo**: https://github.com/monome/aleph

---

## Next Steps

After your first successful build:

1. **Read the code** - Start with `apps/bees/src/app_bees.c`
2. **Try on hardware** - Flash the hex file to your Aleph
3. **Test CDC grids** - Connect a modern monome grid
4. **Implement scene converter** - Complete the v0.7.1 migration work
5. **Fix Docker** - Update to 24.x+ for reliable builds

See [../workflow/DEVELOPMENT_CHECKLIST.md](../workflow/DEVELOPMENT_CHECKLIST.md) for daily workflow guidance.

---

_Last Updated: January 11, 2026_  
_Branch: develop_  
_Version: 0.8.3 with CDC support_

**Two Toolchains Required**:

1. **AVR32 (for Controller)**

   - Compiles BEES application
   - Runs on AVR32 processor
   - Toolchain: `avr32-gcc` 4.4.7
   - Build time: ~30-60 minutes (one-time)

2. **Blackfin (for DSP)**
   - Compiles audio modules
   - Runs on Blackfin BF533 processor
   - Toolchain: via Docker (`pf0camino/cross-bfin-elf`)
   - Available via helper scripts

### Directory Structure

```
/Users/brettgershon/Documents/01 Projects/Aleph/
├── aleph/                    # Main aleph repository (dev branch)
│   ├── apps/
│   │   └── bees/            # BEES controller application
│   ├── modules/             # DSP modules (Blackfin)
│   ├── common/              # Shared code
│   ├── dsp/                 # DSP library
│   └── docs/                # Documentation
│
├── toolchain/               # Toolchain source code
│   ├── avr32-toolchain/    # AVR32 GCC toolchain builder
│   └── avr32-patches.tar   # Required patches
│
├── Dockerfile               # Development environment
├── start-bees-dev.sh       # Quick startup script
└── PHASE_0_EXECUTION_PLAN.md  # What we're doing
```

---

## Building Components

### Build BEES (AVR32)

```bash
cd /aleph/apps/bees
make clean
make

# For release build (optimized)
make clean
make R=1
```

**Output**: `aleph-bees.hex` (firmware file)

### Build a DSP Module (Blackfin)

Exit the aleph-dev container and use the helper script on your Mac:

```bash
# On your Mac (not in Docker)
./build-dsp-module.sh lines
```

Or manually with the Blackfin Docker image:

```bash
docker run --rm --platform linux/amd64 --entrypoint="" \
  -v "$(pwd)/aleph:/projects/aleph" \
  pf0camino/cross-bfin-elf bash -c \
  "cd /projects/aleph/modules/lines && \
   export PATH=/opt/uClinux/bfin-elf/bin:\$PATH && \
   make"
```

**Note**: Module linking has known issues - focus on BEES build first.

---

## Development Workflow

### Typical Development Session

```bash
# 1. Start environment
./start-bees-dev.sh

# 2. Inside container, set PATH
export PATH=/opt/avr32-tools/bin:$PATH

# 3. Make code changes (edit files on Mac, they're mounted)
# Use your favorite editor outside Docker

# 4. Build
cd /aleph/apps/bees
make clean && make

# 5. Check for errors
# Fix any compilation issues

# 6. Deploy (when you have hardware)
# Copy .hex to SD card

# 7. Exit
exit
```

### Faster Builds

For incremental builds, skip `make clean`:

```bash
# Only rebuild changed files
make
```

---

## Common Tasks

### Check Current BEES Version

```bash
cd /aleph/apps/bees
cat version.mk
```

Current: **0.8.1** (maj=0, min=8, rev=1)

### List Available Operators

```bash
cd /aleph/apps/bees/src/ops
ls -1 op_*.c
```

### Find Where Something is Defined

```bash
# Search for a function or symbol
cd /aleph
grep -r "functionName" apps/bees/src/
```

### Check Build Size

```bash
cd /aleph/apps/bees
make
avr32-size aleph-bees.elf
```

---

## Troubleshooting

### "avr32-gcc: command not found"

**Solution**: Set PATH inside container

```bash
export PATH=/opt/avr32-tools/bin:$PATH
```

Or add to your shell startup (inside container):

```bash
echo 'export PATH=/opt/avr32-tools/bin:$PATH' >> ~/.bashrc
source ~/.bashrc
```

### "No rule to make target..."

**Solution**: You might be in the wrong directory

```bash
# Make sure you're in the right place
cd /aleph/apps/bees
pwd  # Should show: /aleph/apps/bees
```

### Build Hangs or Fails

**Solution**: Clean and rebuild

```bash
make clean
make
```

### Docker Container Exits Immediately

**Solution**: Use the startup script or `-it` flags

```bash
# Correct
./start-bees-dev.sh

# Or manually
docker run --rm -it -v "$(pwd)/aleph:/aleph" aleph-dev
```

---

## Phase 0 Goals

You're currently in **Phase 0: Establish 0.8.x Baseline**

### What We're Doing:

1. ✅ Set up build environment
2. 🔄 Build BEES 0.8.1 from source
3. ⏳ Build DSP modules
4. ⏳ Document current state
5. ⏳ Create test suite
6. ⏳ Verify all 0.8.x improvements

### Expected Timeline:

- **Week 1**: Build environment & compilation
- **Week 2**: Code review & documentation
- **Week 3**: Testing infrastructure
- **Week 4**: Regression tests & summary

---

## Next Steps

### After Your First Build:

1. **Read the Code**

   - Start with [apps/bees/src/app_bees.c](aleph/apps/bees/src/app_bees.c)
   - Understand main loop
   - Review operator system

2. **Try Building Modules**

   - Use `build-dsp-module.sh`
   - Document which ones work
   - Note any errors

3. **Explore 0.8.x Features**

   - Look for PR #267 changes (memory management)
   - Find CV output fixes
   - Study preset system

4. **Check Documentation**
   - Read [apps/bees/README-OP-DOCUMENTATION.md](aleph/apps/bees/README-OP-DOCUMENTATION.md)
   - Review operator guide
   - Study scene format

---

## Resources

### Documentation

- **Phase 0 Plan**: [PHASE_0_EXECUTION_PLAN.md](PHASE_0_EXECUTION_PLAN.md)
- **Development Pathway**: [aleph-bees-1.0-development-pathway.md](aleph-bees-1.0-development-pathway.md)
- **0.8.x Analysis**: [bees-0.8-analysis.md](bees-0.8-analysis.md)
- **This Guide**: [GETTING_STARTED.md](GETTING_STARTED.md)

### Community

- **Forum**: https://llllllll.co (lines)
- **Repository**: https://github.com/monome/aleph
- **Docs**: http://monome.org/docs/aleph

### Tools

- **Docker Image**: `aleph-dev` (AVR32 toolchain)
- **Blackfin Image**: `pf0camino/cross-bfin-elf`
- **Helper Scripts**: `start-bees-dev.sh`, `build-dsp-module.sh`

---

## Development Tips

### Make Builds Faster

```bash
# Use multiple cores
make -j4
```

### Save Terminal Output

```bash
make 2>&1 | tee build.log
```

### Quick Operator Check

```bash
# Count how many operators exist
ls apps/bees/src/ops/op_*.c | wc -l
```

### Find Memory Usage

```bash
avr32-nm --size-sort aleph-bees.elf | tail -20
```

---

## Success Indicators

You're on track if:

- ✅ BEES builds without errors
- ✅ You can navigate the codebase
- ✅ You understand the directory structure
- ✅ You know where to find things
- ✅ Docker workflow is smooth

---

## Help & Support

**Stuck?** Check these first:

1. [PHASE_0_EXECUTION_PLAN.md](PHASE_0_EXECUTION_PLAN.md) - Detailed steps
2. [TROUBLESHOOTING.md](aleph/README.md) - Common issues
3. GitHub issues - Known problems
4. lines forum - Community help

**Questions about**:

- Build system → Check Makefiles
- Code structure → Read READMEs in each directory
- Operators → See `apps/bees/README-OP-DOCUMENTATION.md`
- Modules → Look in `modules/*/README` files

---

## Current Status

**Phase**: 0 (Baseline Verification)
**Week**: 1
**Day**: 3
**Status**: Ready to build BEES! 🚀

**Next Action**: Run `./start-bees-dev.sh` and build the toolchain

---

_Getting Started Guide v1.0_
_Created: November 2025_
_For: BEES 1.0 Development Project_
_Updated: Daily during active development_
