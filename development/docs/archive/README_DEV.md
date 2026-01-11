# Monome Aleph Development Environment

This directory contains the Monome Aleph firmware and development toolchain setup.

## What is Monome Aleph?

The Monome Aleph is an open-source audio computer with:
- **AVR32 controller** (UI, I/O, routing)
- **Blackfin DSP** (audio processing)

## Quick Start

**New to Aleph development?** Start here:

1. **Read [QUICKSTART.md](QUICKSTART.md)** - Get up and running in minutes
2. **Run the setup:**
   ```bash
   ./setup-dev.sh
   ```
3. **Start developing:**
   ```bash
   docker-compose run --rm aleph-dev
   ```

## Documentation

- **[QUICKSTART.md](QUICKSTART.md)** - Fast track to building firmware (start here!)
- **[DEVELOPMENT_SETUP.md](DEVELOPMENT_SETUP.md)** - Detailed setup instructions and troubleshooting
- **[aleph/README.md](aleph/README.md)** - Original Aleph firmware documentation

## Directory Structure

```
.
├── aleph/                  # Main Aleph firmware repository
│   ├── apps/              # Controller applications
│   │   └── bees/         # Main routing/management app
│   ├── modules/          # DSP modules
│   ├── dsp/              # DSP library
│   └── utils/            # Development utilities
│
├── toolchain/             # Toolchain sources
│   ├── avr32-toolchain/  # AVR32 GCC toolchain builder
│   └── avr32-patches.tar # Patches for AVR32 toolchain
│
├── Dockerfile             # Development environment
├── docker-compose.yml     # Docker configuration
├── setup-dev.sh          # Setup script
└── build-toolchains.sh   # Toolchain build script (run inside Docker)
```

## System Requirements

- **macOS** (tested on macOS 15.3, Apple Silicon)
- **Docker Desktop** ([download](https://www.docker.com/products/docker-desktop))
- **10GB free disk space** (for toolchains and builds)

## Why Docker?

The Aleph uses older toolchains (GCC 4.4.7 for AVR32, GCC 4.3 for Blackfin) that have compatibility issues with Apple Silicon and modern macOS. Docker provides a consistent Linux environment that works reliably.

## Development Workflow

```bash
# 1. Start Docker environment
docker-compose run --rm aleph-dev

# 2. Build toolchains (first time only, ~30-60 minutes)
/aleph/build-toolchains.sh

# 3. Set PATH
export PATH=/opt/avr32-tools/bin:/opt/uClinux/bfin-elf/bin:$PATH

# 4. Build firmware
cd /aleph/apps/bees && make          # Controller app
cd /aleph/modules/lines && make      # DSP module

# 5. Copy to SD card and flash to hardware
```

## What Can You Build?

### Controller Applications (AVR32)
- **BEES** - The main routing and management application
- **Custom apps** - Create your own controller firmware

### DSP Modules (Blackfin)
- **lines** - Delay-based module
- **mix** - Mixer module
- **waves** - Wavetable synth
- **Custom modules** - Create your own DSP processors

## Resources

- [Official Aleph Documentation](http://www.monome.org/docs/aleph)
- [Monome Forum](https://llllllll.co/)
- [Aleph GitHub](https://github.com/monome/aleph)

## Getting Help

1. Check [DEVELOPMENT_SETUP.md](DEVELOPMENT_SETUP.md) for troubleshooting
2. Read the original [aleph/README.md](aleph/README.md)
3. Ask on the [Monome forum](https://llllllll.co/)

## Contributing

The Aleph is an open-source project. Feel free to:
- Create custom applications and modules
- Improve documentation
- Fix bugs and submit patches
- Share your work with the community

## License

See [aleph/LICENSE.TXT](aleph/LICENSE.TXT) for the Aleph firmware license.

---

**Ready to start?** → Read [QUICKSTART.md](QUICKSTART.md)
