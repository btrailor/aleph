# Aleph Firmware Development

BEES firmware development for monome aleph.

## ⚠️ Current Status (2026-01-11)

**Build system blocked by Docker bug** - see [recovery doc](development/docs/STABILITY_RECOVERY_2026-01-11.md)

**Quick fix**: Update Docker Desktop to 24.x+ or use Linux machine for builds.

## Quick Links

- **[Development Checklist](development/docs/DEVELOPMENT_CHECKLIST.md)** - Daily workflow
- **[Building Guide](development/docs/BUILDING_BEES_GUIDE.md)** - How to build firmware
- **[Stability Strategy](development/docs/DEVELOPMENT_STABILITY_STRATEGY.md)** - Long-term plan
- **[Git Strategy](development/docs/GIT_STRATEGY.md)** - Branch workflow

## Getting Started

```bash
# 1. Clone with submodules
git clone --recurse-submodules https://github.com/[your-username]/aleph.git
cd aleph

# 2. Verify libavr32 submodule
cd libavr32
git checkout aleph-cdc-compat
cd ..

# 3. Build (when Docker works)
docker run --rm -v "$(pwd):/host" -w /tmp aleph-builder bash -c \
  "cp -r /host /tmp/aleph && cd /tmp/aleph/apps/bees && make && \
   cp aleph-bees.hex /host/apps/bees/"
```

## Project Structure

```
apps/
  bees/              # Main firmware application
    src/             # Application code
      scene.c        # Scene loading/management
      param.c        # Parameter system
      ops/           # Operator implementations

modules/             # DSP modules (blackfin)

libavr32/            # Hardware abstraction (submodule)
  asf/               # Atmel Software Framework
  src/               # Custom HAL code

development/
  docs/              # All documentation

utils/
  beekeep/           # Scene editor application
```

## Branch Strategy

```
main          - Stable releases (v0.8.3, v0.8.4)
  └── develop - Active development (recommended base)
      └── feature/* - Short-lived feature branches
```

See [GIT_STRATEGY.md](development/docs/GIT_STRATEGY.md) for details.

## Current Work

- Scene migration from v0.7.1 to v0.8.x format
- Parameter corruption bug diagnosis
- Build system stability improvements

## License

See [LICENSE.TXT](LICENSE.TXT)
