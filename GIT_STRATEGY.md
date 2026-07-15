# Aleph Git Strategy

**Updated**: 2026-07-15

## Repository Purpose

Firmware, tooling, and documentation for the monome Aleph. The `libavr32`
hardware abstraction layer is included as a Git submodule.

## Branch Model

Two long-lived branches only:

```
main            - Stable, tested releases (v0.8.3, v0.8.4, ...)
  └── develop   - Active development / next integration branch
      └── feature/*  - Short-lived feature branches
```

### `main`

- Only receives complete, tested changes.
- Always expected to build.
- Tagged for releases (`v0.8.3`, `v0.8.4`, ...).
- `libavr32` tracks upstream `monome/libavr32` `main`.

### `develop`

- Default branch for active work.
- Always expected to be buildable.
- Feature branches are created from here and merge back here first.
- `libavr32` also tracks upstream `monome/libavr32` `main`; aleph-specific
deltas are kept minimal and upstreamed when possible.

### Feature branches

- Name: `feature/<short-description>`.
- Created from `develop`.
- Deleted after merge.

## Workflow

### Start a feature

```bash
git checkout develop
git pull origin develop
git checkout -b feature/my-feature
# work, commit, push
git push origin feature/my-feature
```

### Finish a feature

```bash
git checkout develop
git pull origin develop
git merge --no-ff feature/my-feature
git push origin develop
git branch -d feature/my-feature
git push origin --delete feature/my-feature
```

### Release

```bash
git checkout main
git pull origin main
git merge --no-ff develop
git tag -a v0.8.x -m "Release v0.8.x"
git push origin main --tags
```

## libavr32 Submodule

```ini
[submodule "libavr32"]
	path = libavr32
	url = https://github.com/monome/libavr32.git
	branch = main
```

Rules:

1. `libavr32` tracks upstream `monome/libavr32` `main`.
2. If an aleph-specific change is needed, apply it in the aleph tree first
   and work to upstream the generalizable parts.
3. Never force-push or rewrite submodule history that has been released.
4. After changing `.gitmodules`, run `git submodule sync`.

## Remotes

- `origin` — your fork (`btrailor/aleph` or your own).
- `upstream` — `https://github.com/monome/aleph.git` (read-only for most).

For `libavr32`:

- `origin` (inside the submodule) — `https://github.com/monome/libavr32.git`.

## Legacy Branches

The following one-off / exploratory branches have been archived as tags and
deleted from `origin`:

- `archive/cdc-support-clean` — working CDC lineage on bees 0.7.2.
- `archive/cdc-transport-poc` — CDC transport proof-of-concept (`cdc-dev`).
- `archive/beekeep-m1` — M1 Mac support and scene-loading fixes.
- `archive/bees-1.0-dev-v0.8` — clean 0.8 baseline.
- `archive/dev` — legacy dev mirror.
- `archive/develop-local` — local CDC integration work.
- `archive/cdc-grid-on-0.8.1` / `archive/cdc-grid-on-stable` — earlier 0.8.1 CDC ports.

Use `git show archive/<name>` to inspect historical state.
