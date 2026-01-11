# Development Stability Strategy

**Created**: 2026-01-11  
**Purpose**: Establish stable, predictable development workflow for Aleph firmware development

---

## The Problem

Over the last two days, the build system has been unstable due to:

1. **Docker Volume Mount Issues**: macOS arm64 emulating amd64 containers causes file read failures
2. **libavr32 Instability**: Submodule changes getting overwritten during branch switches
3. **Branch Proliferation**: Multiple feature branches causing confusion about "working" state
4. **Uncommitted Changes**: Critical build fixes living as uncommitted changes that get lost

---

## Root Cause: Docker Volume Mount Failure

### Current Symptoms

```bash
# Files appear to exist but are empty when read from Docker
docker run --rm -v "$(pwd):/aleph" aleph-builder bash -c "file /aleph/apps/bees/Makefile"
# Output: Makefile: empty  (despite being 1582 bytes on host)
```

### Why This Happens

- **Platform Mismatch**: macOS arm64 host running amd64 Docker container
- **osxfs Limitation**: Docker Desktop's osxfs filesystem driver has issues with cross-platform file reads
- **Docker Version**: Your Docker 20.10.23 is older and has known volume mount bugs on macOS

### Permanent Solution Options

#### Option 1: Update Docker Desktop (RECOMMENDED)

```bash
# Current: Docker 20.10.23 (has known bugs)
# Target:  Docker 24.x or later (improved arm64/amd64 compatibility)

# Steps:
# 1. Download latest Docker Desktop for Mac from docker.com
# 2. Install and restart
# 3. Test: docker run --rm -v "$(pwd):/test" alpine cat /test/README.md
```

#### Option 2: Use Native arm64 Toolchain (IDEAL)

Build a native arm64 Docker image with AVR32 toolchain:

```dockerfile
FROM --platform=linux/arm64 ubuntu:22.04
# Install AVR32 toolchain compiled for arm64
# This eliminates emulation overhead and volume mount issues
```

**Benefits**:

- No emulation = faster builds
- No volume mount issues
- Better long-term stability

**Drawbacks**:

- Requires rebuilding toolchain Docker image (1-2 hours initial setup)
- Need to verify AVR32 toolchain works on arm64

#### Option 3: Use Build Server / Linux Machine

Run builds on x86_64 Linux machine where Docker works properly:

- Remote server
- Linux VM
- GitHub Actions for CI builds

---

## Git Workflow Strategy

### Problem: Too Many Branches, Unclear State

**Current branches**:

- `main` - stable merged work
- `dev` - development work
- `cdc-dev` - CDC feature
- `beekeep-m1` - Beekeep feature
- `feature/scene-migration-0.7.1-to-0.8.x` - current work

**Issues**:

- Unclear which branch has working build
- libavr32 state differs across branches
- Merging causes conflicts and build breakage

### Proposed: Simplified Branch Strategy

```
main (stable, released code)
  └── develop (active development, always buildable)
      ├── feature/cdc-support
      ├── feature/scene-migration
      ├── feature/beekeep-updates
      └── [other features]
```

#### Branch Rules

**`main` branch**:

- Only merge complete, tested features
- Always builds successfully
- Represents "release-ready" state
- Tag releases: v0.8.3, v0.8.4, etc.

**`develop` branch** (NEW):

- Default branch for development work
- Always keeps working build state
- libavr32 pinned to known-good commit
- All feature branches created from here
- All features merge back here first

**`feature/*` branches**:

- Created from `develop`
- Short-lived (days to weeks, not months)
- Merged back to `develop` when complete
- Deleted after merge

### Migration Steps

```bash
# 1. Create develop branch from current working state
cd /path/to/aleph
git checkout feature/scene-migration-0.7.1-to-0.8.x
git checkout -b develop

# 2. Ensure libavr32 is stable
cd libavr32
git checkout aleph-cdc-compat
git pull origin aleph-cdc-compat
cd ..
git add libavr32
git commit -m "Pin libavr32 to aleph-cdc-compat for stable builds"

# 3. Push develop
git push -u origin develop

# 4. Update .gitmodules to track aleph-cdc-compat
# (Already done, but verify)
cat .gitmodules  # Should show: branch = aleph-cdc-compat
```

---

## libavr32 Submodule Stability

### Problem: Changes Getting Overwritten

**What happens now**:

1. You make fixes to libavr32 files
2. Changes stay as uncommitted modifications
3. `git checkout` or submodule update wipes them
4. Build breaks

### Solution: Commit Everything to aleph-cdc-compat

**Never leave libavr32 changes uncommitted!**

```bash
# After making ANY change to libavr32:
cd libavr32
git status  # Check what changed
git add .
git commit -m "Fix: [describe what you fixed]"
git push origin aleph-cdc-compat

# Return to parent repo and update submodule reference
cd ..
git add libavr32
git commit -m "Update libavr32 to include [fix description]"
```

### Submodule Best Practices

1. **Always work on aleph-cdc-compat branch**

   ```bash
   cd libavr32
   git checkout aleph-cdc-compat
   ```

2. **Never use `git submodule update --remote`**

   - This pulls latest from upstream, losing your changes
   - Only use: `git submodule update --init` (gets pinned commit)

3. **Verify before every build**

   ```bash
   cd libavr32
   git log -1 --oneline  # Check you're on expected commit
   git status            # Should be clean
   ```

4. **Document all libavr32 changes**
   - Keep BUILDING_BEES_GUIDE.md updated
   - Note which files were modified and why

---

## Recommended Immediate Actions

### Priority 1: Fix Docker (Choose One)

- [ ] **Option A**: Update Docker Desktop to 24.x+
- [ ] **Option B**: Build arm64-native Docker image
- [ ] **Option C**: Set up Linux build environment

### Priority 2: Stabilize Git Workflow

```bash
# 1. Stash current changes
git stash push -m "2026-01-11 WIP: param debug logging"

# 2. Create stable develop branch
git checkout -b develop

# 3. Verify libavr32 state
cd libavr32
git checkout aleph-cdc-compat
git log -1
cd ..

# 4. Apply only committed, tested changes
git stash list
# Selectively apply what you need

# 5. Test build before committing
# (Once Docker is fixed)

# 6. Commit and push
git add -A
git commit -m "Establish develop branch with stable build configuration"
git push -u origin develop
```

### Priority 3: Update Documentation

- [ ] Update BUILDING_BEES_GUIDE.md with Docker fix
- [ ] Document new branch strategy in GIT_STRATEGY.md
- [ ] Create DEVELOPMENT_CHECKLIST.md for daily workflow

---

## Daily Development Workflow (Future)

### Starting Work

```bash
# 1. Switch to develop
git checkout develop
git pull origin develop

# 2. Verify libavr32
cd libavr32 && git status && git log -1 --oneline && cd ..

# 3. Create feature branch
git checkout -b feature/my-new-feature

# 4. Work and commit regularly
```

### During Work

```bash
# If you modify libavr32:
cd libavr32
git add .
git commit -m "Fix: ..."
git push origin aleph-cdc-compat
cd ..
git add libavr32
git commit -m "Update libavr32: ..."

# If you modify app code:
git add apps/bees/src/whatever.c
git commit -m "Add: ..."
```

### Completing Feature

```bash
# 1. Test build
# 2. Test on hardware
# 3. Merge to develop
git checkout develop
git merge --no-ff feature/my-new-feature
git push origin develop

# 4. Delete feature branch
git branch -d feature/my-new-feature
git push origin --delete feature/my-new-feature
```

### Releasing

```bash
# When develop is stable and tested:
git checkout main
git merge --no-ff develop
git tag -a v0.8.4 -m "Release 0.8.4: [features]"
git push origin main --tags
```

---

## Build System Stability Checklist

Before declaring build system stable:

- [ ] Docker reads files correctly (no "empty" files)
- [ ] Build completes without errors
- [ ] Build is reproducible (clean checkout builds successfully)
- [ ] libavr32 submodule stays on aleph-cdc-compat
- [ ] No uncommitted changes required for build
- [ ] Documentation matches actual working process
- [ ] Can switch between branches without breaking build

---

## Emergency Recovery

If build breaks again:

```bash
# 1. Don't panic - we have working hex file
ls -lh apps/bees/aleph-bees.hex  # Jan 10 23:32 build

# 2. Check what changed
git status
git diff

# 3. Check libavr32
cd libavr32
git status
git log -5 --oneline

# 4. Reset to known-good state
git checkout aleph-cdc-compat
cd ..

# 5. Consult BUILDING_BEES_GUIDE.md

# 6. Ask: "What was different when it last worked?"
```

---

## Success Criteria

Development system is stable when:

1. ✅ Any team member can clone and build successfully
2. ✅ Build process takes < 5 minutes
3. ✅ Switching branches doesn't break builds
4. ✅ libavr32 changes persist correctly
5. ✅ Clear documentation for all processes
6. ✅ No "magic" uncommitted changes required

**Current Status**: ❌ Docker volume mount issue blocking all builds

**Next Step**: Fix Docker to unblock development
