# Aleph Development Repository Structure

**Updated**: 2026-01-11 - Revised for stability

## Repository Purpose

This repository tracks BEES firmware development, including:

- Firmware source code (apps/bees, modules, dsp, etc.)
- libavr32 hardware abstraction layer (submodule)
- Development environment (Docker) and documentation
- Analysis and planning documents
- Build scripts and configurations

## Git Branch Strategy (REVISED)

### Main Branches

```
main          - Stable, tested releases (v0.8.3, v0.8.4, etc.)
  └── develop - Active development, always buildable
      ├── feature/cdc-support
      ├── feature/scene-migration
      └── [other features]
```

**`main` branch**:

- Only merge complete, tested features
- Always builds successfully
- Tagged releases: v0.8.3, v0.8.4, etc.
- Represents "production ready" code

**`develop` branch** (NEW - recommended):

- Default branch for active development
- Always maintains buildable state
- libavr32 pinned to known-good commit (aleph-cdc-compat)
- All feature branches created from here
- All features merge here first, then to main when stable

**`feature/*` branches**:

- Created from `develop`
- Short-lived (days to weeks)
- Merged back to `develop` when complete
- Deleted after successful merge

### Legacy Branches (Being Phased Out)

- `dev` - Old development branch
- `cdc-dev` - CDC work (merge to feature/cdc-support)
- `beekeep-m1` - Beekeep work
- `feature/scene-migration-0.7.1-to-0.8.x` - Current work

### Remote Configuration

- `origin` - Your GitHub fork (brettgershon/aleph)
- `upstream` or `monome` - Official monome repository

## Workflow

### Starting New Feature

```bash
# 1. Start from develop
git checkout develop
git pull origin develop

# 2. Verify libavr32 is stable
cd libavr32
git checkout aleph-cdc-compat
git log -1  # Should show: 02469a2 or newer
cd ..

# 3. Create feature branch
git checkout -b feature/my-new-feature

# 4. Work and commit regularly
```

### During Development

**Commit often**:

```bash
git add [files]
git commit -m "[Area]: [what changed]"
git push origin feature/my-new-feature
```

**If modifying libavr32**:

```bash
cd libavr32
git add .
git commit -m "Fix: [description]"
git push origin aleph-cdc-compat
cd ..
git add libavr32
git commit -m "Update libavr32: [description]"
```

### Completing Feature

```bash
# 1. Ensure all changes committed
git status  # Should be clean

# 2. Test build and on hardware

# 3. Merge to develop
git checkout develop
git pull origin develop
git merge --no-ff feature/my-new-feature
git push origin develop

# 4. Delete feature branch
git branch -d feature/my-new-feature
git push origin --delete feature/my-new-feature
```

### Releasing

```bash
# When develop is stable and ready:
git checkout main
git pull origin main
git merge --no-ff develop
git tag -a v0.8.4 -m "Release 0.8.4: [feature summary]"
git push origin main --tags

# Create GitHub release with:
# - Tag: v0.8.4
# - Binary: apps/bees/aleph-bees.hex
# - Changelog
```

## libavr32 Submodule Management

### Critical Rules

1. **Always work on `aleph-cdc-compat` branch**
2. **Commit changes immediately** - never leave uncommitted
3. **Never use `git submodule update --remote`** (overwrites your changes)
4. **Document all changes** in commit messages

### Setup (One-Time)

```bash
cd libavr32
git checkout -b aleph-cdc-compat
git remote add origin https://github.com/[your-fork]/libavr32.git
git push -u origin aleph-cdc-compat
```

### Daily Use

```bash
# Before building
cd libavr32
git checkout aleph-cdc-compat
git status  # Should be clean
cd ..

# After modifying libavr32 files
cd libavr32
git add .
git commit -m "Fix: ..."
git push origin aleph-cdc-compat
cd ..
git add libavr32  # Updates submodule pointer in parent repo
git commit -m "Update libavr32 submodule"
```

### Checking Submodule State

```bash
# What commit is pinned?
git ls-tree HEAD libavr32

# Is submodule in expected state?
cd libavr32
git log -1 --oneline
git status
```

## File Organization

### Always Tracked (Our Development Work)

```
/docs/*                    # Our documentation and analysis
/*.md                      # Development pathway, analysis documents
/docker-compose.yml        # Development environment
/Dockerfile               # Build environment
/build-*.sh               # Build and setup scripts
/.gitignore               # Repository configuration
```

### Source Code (Tracked with Care)

```
/aleph/*                  # Source code from boq's repository
                         # Changes here can be contributed upstream
```

### Never Tracked

```
/toolchain/*              # Build artifacts and toolchains
/build/*                  # Temporary build files
```

## Contribution Strategy

### For Documentation and Environment

- Develop on `main` branch
- Push to your fork freely
- These are your development assets

### For Source Code Fixes

- Create feature branches from `source-sync`
- Make targeted fixes (memory management, performance, etc.)
- Create clean pull requests back to boq's repository
- Keep commits focused and well-documented

## Safety Measures

- Never push documentation directly to boq's repo
- Always create feature branches for source code changes
- Use descriptive commit messages for all changes
- Test all changes in development environment before committing
