# Aleph Development Repository Structure

## Repository Purpose

This repository tracks our Phase 1 development work for BEES 1.0, including:

- Development environment setup and documentation
- Analysis and planning documents
- Build scripts and Docker configurations
- Source code changes and improvements (in aleph/ subdirectory)

## Git Branch Strategy

### Main Branches

- `main` - Our development branch with documentation and environment
- `source-sync` - Clean sync with boq's repository for source code
- `feature/*` - Individual feature development branches

### Remote Configuration

- `origin` - Your GitHub fork (brettgershon/aleph) - for your development work
- `upstream-boq` - boq's repository - for source code sync and potential contributions
- `upstream-monome` - Official monome repository - for official updates

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
