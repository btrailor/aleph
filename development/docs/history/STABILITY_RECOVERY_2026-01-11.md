# Development Stability Recovery - 2026-01-11

## Summary

After two days of build system instability, we've diagnosed the root cause and established a stable development strategy going forward.

---

## Root Cause Identified

**Docker Volume Mount Failure**

```
Symptom: Files appear in ls but read as empty from Docker container
Cause:   Docker 20.10.23 osxfs bug on macOS arm64 emulating amd64
Impact:  Complete build system failure - cannot read Makefiles or source files
```

**Evidence**:

```bash
$ docker run --rm -v "$(pwd):/aleph" aleph-builder ls -la /aleph/apps/bees/Makefile
-rwxr-xr-x 1 501 dialout 1582 Jan 10 16:04 Makefile

$ docker run --rm -v "$(pwd):/aleph" aleph-builder file /aleph/apps/bees/Makefile
Makefile: empty

$ docker run --rm -v "$(pwd):/aleph" aleph-builder head -5 /aleph/apps/bees/Makefile
(no output - file reads as empty)
```

This is NOT our code or configuration - it's a Docker bug that affects ALL file reads.

---

## Solutions

### Immediate (Choose One)

1. **Update Docker Desktop to 24.x+** (Recommended)

   - Fixes volume mount bugs
   - Improves arm64/amd64 compatibility
   - 30-minute task

2. **Build arm64-native Docker image**

   - Eliminates emulation entirely
   - Requires rebuilding toolchain container
   - 1-2 hour initial setup

3. **Use Linux build machine**
   - Remote server, VM, or GitHub Actions
   - x86_64 Linux doesn't have this issue
   - Most reliable long-term

### Workaround Until Fixed

Use the working build from Jan 10 23:32:

```bash
ls -lh apps/bees/aleph-bees.hex
# -rw-r--r-- 1 brettgershon staff 483K Jan 10 23:32 aleph-bees.hex
```

This build works but lacks the debug logging we added today.

---

## Development Strategy Changes

### New Branch Structure

```
main (stable releases)
  └── develop (active development, always buildable)
      ├── feature/cdc-support
      ├── feature/scene-migration
      └── [other short-lived features]
```

**Benefits**:

- Clear "working" baseline (develop)
- Features isolated and short-lived
- Easy to recover from mistakes
- libavr32 stability across branches

### libavr32 Submodule Rules

**CRITICAL**: Never leave libavr32 changes uncommitted

```bash
# After ANY change to libavr32:
cd libavr32
git add .
git commit -m "Fix: [what you fixed]"
git push origin aleph-cdc-compat
cd ..
git add libavr32
git commit -m "Update libavr32: [description]"
```

**Why this matters**:

- Uncommitted changes get wiped by `git checkout`
- We lost working build state multiple times this way
- Submodule must track aleph-cdc-compat branch

---

## Current State

### What's Working

- ✅ libavr32 on stable commit (02469a2)
- ✅ .gitmodules tracks aleph-cdc-compat
- ✅ Working hex file from Jan 10 available
- ✅ Comprehensive documentation created:
  - `DEVELOPMENT_STABILITY_STRATEGY.md` - Full analysis and solutions
  - `DEVELOPMENT_CHECKLIST.md` - Daily workflow reference
  - `GIT_STRATEGY.md` - Updated branch strategy
  - `BUILDING_BEES_GUIDE.md` - Updated with Docker issue

### What's Blocked

- ❌ Cannot build new firmware (Docker bug)
- ❌ Cannot test param debug logging
- ❌ Cannot verify scene conversion code

### What's Staged

- Modified param.c (debug logging added)
- Modified apps/aleph_avr32_src.mk (TWI driver removed)
- Updated documentation

---

## Next Steps

### Option A: Update Docker (Recommended)

```bash
# 1. Download latest Docker Desktop for Mac
open https://www.docker.com/products/docker-desktop

# 2. Install and restart

# 3. Verify fix
docker --version  # Should be 24.x or later
docker run --rm -v "$(pwd):/test" alpine cat /test/README.md
# Should output README content (not empty)

# 4. Rebuild firmware
docker run --rm -v "$(pwd):/host" -w /tmp aleph-builder bash -c \
  "cp -r /host /tmp/aleph && cd /tmp/aleph/apps/bees && make && \
   cp aleph-bees.hex /host/apps/bees/"

# 5. Test debug logging on hardware
```

### Option B: Focus on Code (Without Building)

```bash
# 1. Commit current changes
git add -A
git commit -m "Add param debug logging and update documentation"

# 2. Implement scene conversion logic
# Edit apps/bees/src/scene_convert.c
# - Implement scene_convert_from_0_7_1()
# - Use OPERATOR_ID_MAPPING.h
# - Use OPERATOR_OUTPUT_CHANGES.h

# 3. Integrate into scene.c
# - Call conversion before unpickling

# 4. Test once Docker is fixed
```

### Option C: Set Up Linux Build Environment

```bash
# 1. Spin up Ubuntu VM or use remote server
# 2. Install Docker
# 3. Clone repo
# 4. Build normally (x86_64 Docker doesn't have this bug)
```

---

## Documentation Created

All strategies and workflows documented in:

1. **DEVELOPMENT_STABILITY_STRATEGY.md**

   - Root cause analysis
   - All solution options
   - Git workflow redesign
   - Daily practices

2. **DEVELOPMENT_CHECKLIST.md**

   - Quick reference for daily work
   - Before/during/after workflows
   - Common issues and solutions
   - Emergency recovery procedures

3. **GIT_STRATEGY.md** (Updated)

   - New branch structure
   - Feature development workflow
   - Release process
   - libavr32 submodule management

4. **BUILDING_BEES_GUIDE.md** (Updated)
   - Docker issue documented
   - Workaround noted
   - Link to stability strategy

---

## Lessons Learned

1. **Infrastructure first**: Can't develop without working build
2. **Commit everything**: Uncommitted changes WILL be lost
3. **Submodules are tricky**: Need strict discipline
4. **Document when working**: Memory fades, docs persist
5. **Stable baseline**: One "always works" branch is critical

---

## Decision Point

**What do you want to do?**

1. Update Docker now (30 min) then continue development
2. Focus on implementing scene conversion code (can't test yet)
3. Set up Linux build environment (most reliable)
4. Use workaround and work on non-build tasks

Let me know and I'll help you proceed.
