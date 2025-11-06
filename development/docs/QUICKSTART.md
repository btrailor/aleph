# Quick Start Guide - Monome Aleph Development

## Getting Started (5 minutes)

### 1. Set up the development environment

```bash
./setup-dev.sh
```

This will build the Docker image with all necessary dependencies.

### 2. Start the development container

```bash
docker-compose run --rm aleph-dev
```

You're now inside a Linux container with all build tools ready.

### 3. Build the toolchains (inside container, 30-60 minutes)

```bash
/aleph/build-toolchains.sh
```

Or manually:

```bash
# AVR32 toolchain
cd /toolchain/avr32-toolchain
make install-cross PREFIX=/opt/avr32-tools

# Blackfin toolchain
cd /tmp
wget http://sourceforge.net/projects/adi-toolchain/files/2012R2/2012R2-RC2/i386/blackfin-toolchain-elf-gcc-4.3-2012R2-RC2.i386.tar.bz2
tar -xjf blackfin-toolchain-elf-gcc-4.3-2012R2-RC2.i386.tar.bz2 -C /
```

### 4. Set up PATH

```bash
export PATH=/opt/avr32-tools/bin:/opt/uClinux/bfin-elf/bin:$PATH
```

Or add to `.bashrc`:
```bash
echo 'export PATH=/opt/avr32-tools/bin:/opt/uClinux/bfin-elf/bin:$PATH' >> ~/.bashrc
source ~/.bashrc
```

### 5. Build your first firmware

```bash
# Build BEES controller app
cd /aleph/apps/bees
make

# Build a DSP module (e.g., lines)
cd /aleph/modules/lines
make
```

## Common Commands

### Start development session
```bash
docker-compose run --rm aleph-dev
```

### Build controller firmware (AVR32)
```bash
cd /aleph/apps/bees && make
```

### Build DSP module (Blackfin)
```bash
cd /aleph/modules/lines && make
```

### Clean build artifacts
```bash
make clean
```

### Check toolchain versions
```bash
avr32-gcc --version
bfin-elf-gcc --version
```

## File Locations

After building:
- Controller firmware (`.hex`): `aleph/apps/bees/aleph-bees.hex`
- DSP modules (`.ldr`): `aleph/modules/MODULE_NAME/MODULE_NAME.ldr`

## Loading onto Hardware

1. **Prepare SD card:**
   - Create `/hex` folder for controller firmware
   - Create `/data` folder for DSP modules

2. **Enter bootloader:**
   - Hold MODE switch (square button, upper right) while powering on

3. **Flash firmware:**
   - Select the `.hex` file from the bootloader menu
   - Device will reset and launch the new firmware

## Troubleshooting

### "Command not found: avr32-gcc"
Make sure PATH is set: `export PATH=/opt/avr32-tools/bin:$PATH`

### "Command not found: bfin-elf-gcc"
Make sure PATH is set: `export PATH=/opt/uClinux/bfin-elf/bin:$PATH`

### Build fails with missing headers
Make sure you're inside the Docker container, not on your Mac directly.

### Docker container exits immediately
Use `docker-compose run --rm aleph-dev` not `docker-compose up`

## Next Steps

- Read [DEVELOPMENT_SETUP.md](DEVELOPMENT_SETUP.md) for detailed information
- Explore the codebase in `aleph/`
- Check out existing apps in `aleph/apps/`
- Look at DSP modules in `aleph/modules/`
- Read the [official Aleph documentation](http://www.monome.org/docs/aleph)

## Project Structure

```
aleph/
├── apps/bees/          # Main controller app
├── modules/            # DSP modules
│   ├── lines/         # Delay-based module
│   ├── mix/           # Mixer module
│   └── ...
├── dsp/               # DSP library functions
├── common/            # Shared code
└── utils/             # Utilities and tools
```

## Useful Make Targets

```bash
make         # Build the project
make clean   # Remove build artifacts
make program # (if supported) Flash to hardware
```
