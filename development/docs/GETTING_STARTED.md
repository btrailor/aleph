# Getting Started with BEES 1.0 Development

**Welcome to the Aleph BEES 1.0 development project!**

This guide will get you from zero to building BEES firmware in ~60 minutes.

---

## What We're Building

**Goal**: BEES 1.0 - A production-ready release of the Aleph control software

**Foundation**: BEES 0.8.1 (from `dev` branch)
**Timeline**: 8-9 months to 1.0 release
**Current Phase**: Phase 0 - Baseline Verification

---

## Quick Start (5 minutes)

### 1. Start Development Environment

```bash
./start-bees-dev.sh
```

This launches a Docker container with all build dependencies.

### 2. Build the AVR32 Toolchain (~30-60 minutes, one-time)

Inside the Docker container:

```bash
cd /toolchain/avr32-toolchain
make install-cross PREFIX=/opt/avr32-tools
```

**Note**: This takes 30-60 minutes. Go get coffee ☕

### 3. Set Up PATH

```bash
export PATH=/opt/avr32-tools/bin:$PATH

# Verify it worked
avr32-gcc --version
```

You should see: `avr32-gcc (GCC) 4.4.7`

### 4. Build BEES!

```bash
cd /aleph/apps/bees
make clean
make
```

**Expected output**: `aleph-bees.hex` file created

### 5. Celebrate 🎉

You just built BEES 0.8.1 from source!

---

## Detailed Setup Guide

### Prerequisites

**On your Mac** (already done):
- ✅ Docker/Colima installed
- ✅ Repository cloned
- ✅ Docker image built (`aleph-dev`)

### Understanding the Environment

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

*Getting Started Guide v1.0*
*Created: November 2025*
*For: BEES 1.0 Development Project*
*Updated: Daily during active development*
