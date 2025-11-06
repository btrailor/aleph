# Aleph Developer Usage Guide

**Complete guide for developing on the Aleph platform with real hardware compilation**

## Quick Start

### Prerequisites Checklist

- [ ] Docker Desktop installed with multi-platform support
- [ ] At least 8GB RAM and 20GB free disk space
- [ ] Aleph source code (working copy from `/Users/brettgershon/aleph-development/aleph-builder-main/aleph/`)
- [ ] Both containers set up (see ALEPH_DEVELOPMENT_ENVIRONMENT.md)

### Verify Your Setup

```bash
# Check containers are running
docker ps | grep aleph

# Expected output:
# aleph-avr32     (ARM64 container)
# aleph-blackfin  (x86_64 container)
```

## Development Workflows

### 🔧 Firmware Development (AVR32)

**Use Case**: Modifying BEES firmware, adding new hardware features, system-level changes

#### 1. Enter Development Environment

```bash
docker exec -it aleph-avr32 bash
export PATH=/root/avr32-tools-6711e09/bin:$PATH
cd /workspace/aleph/apps/bees
```

#### 2. Build Firmware

```bash
# Clean previous build
make clean

# Full build
make

# Quick verification
ls -la aleph-bees.*
# Expected: aleph-bees.elf (984KB), aleph-bees.hex (25KB), aleph-bees.bin (9KB)
```

#### 3. Development Cycle

```bash
# Edit source files (use host editor or container)
vim src/main.c

# Rebuild
make

# Check for errors
echo $?  # Should be 0 for success
```

#### 4. Deploy to Hardware

```bash
# Use generated files:
# - aleph-bees.elf: For debugging with GDB
# - aleph-bees.hex: For flash programming
# - aleph-bees.bin: For direct memory loading
```

### 🎵 DSP Module Development (Blackfin)

**Use Case**: Creating audio processing algorithms, effects, synthesizers

#### 1. Enter Development Environment

```bash
docker exec -it aleph-blackfin bash
export PATH=/opt/uClinux/bfin-elf/bin:$PATH
cd /workspace/aleph/modules
```

#### 2. Choose/Create Module

```bash
# Work with existing module
cd mix

# Or create new module (copy template)
cp -r mix mynewmodule
cd mynewmodule
```

#### 3. Build DSP Module

```bash
# Ensure complete linker script
cp ../lines/lines.lds mynewmodule.lds

# Clean and build
make clean
make

# Verify output
ls -la *.ldr *.elf
# Expected: mynewmodule.ldr (hardware loader), mynewmodule (ELF binary)
```

#### 4. Development Cycle

```bash
# Edit DSP algorithm
vim mynewmodule.c

# Rebuild and test
make

# Check compilation
bfin-elf-objdump -f mynewmodule
# Should show: file format elf32-bfin, architecture: bfin
```

## Code Examples

### AVR32 Firmware Example

```c
// Example: Adding a new hardware feature to BEES
// File: /workspace/aleph/apps/bees/src/custom_feature.c

#include "print_funcs.h"
#include "events.h"

void custom_feature_init(void) {
    print_dbg("\r\n custom feature initialized");
    // Initialize your hardware feature
}

void custom_feature_process(void) {
    // Process your feature in main loop
}
```

### Blackfin DSP Module Example

```c
// Example: Simple audio effect module
// File: /workspace/aleph/modules/mynewmodule/mynewmodule.c

#include "module.h"
#include "params.h"

// DSP processing function
void module_process_frame(void) {
    fract32 input, output;

    // Get audio input
    input = in_0[processIndex];

    // Apply your DSP algorithm
    output = mult_fr1x32x32(input, param_smooth[eParam_Gain]);

    // Send audio output
    out_0[processIndex] = output;
}

// Parameter handling
void module_set_param(u32 idx, ParamValue v) {
    switch(idx) {
        case eParam_Gain:
            param_smooth[eParam_Gain] = v.fix;
            break;
    }
}
```

## Build System Details

### AVR32 Build Process

1. **Preprocessing**: Handle includes and macros
2. **Compilation**: Generate AVR32 assembly
3. **Assembly**: Create object files
4. **Linking**: Combine with ASF framework
5. **Post-processing**: Generate .hex and .bin files

### Blackfin Build Process

1. **Compilation**: Generate BF533-optimized code
2. **DSP Library Linking**: Link with `-lbfdsp -lbffastfp`
3. **Memory Layout**: Apply linker script for L1/L2 regions
4. **LDR Generation**: Create hardware boot loader

## Debugging and Testing

### AVR32 Debugging

```bash
# Check build errors
make 2>&1 | grep -i error

# Verify toolchain
avr32-gcc --version
avr32-objdump -f aleph-bees.elf

# Memory usage analysis
avr32-size aleph-bees.elf
```

### Blackfin Debugging

```bash
# Check compilation
bfin-elf-gcc --version

# Analyze generated code
bfin-elf-objdump -d mynewmodule.o | head -20

# Memory regions check
bfin-elf-objdump -h mynewmodule

# Verify DSP optimizations
bfin-elf-objdump -S mynewmodule.o | grep -A5 -B5 "multiply"
```

## Advanced Usage

### Custom Build Configurations

#### AVR32 Makefile Customization

```makefile
# Add custom compiler flags
CFLAGS += -DCUSTOM_FEATURE_ENABLED
CFLAGS += -O3  # Maximum optimization

# Add custom source files
SRC += custom_feature.c
```

#### Blackfin Makefile Customization

```makefile
# Add DSP-specific optimizations
CFLAGS += -mcpu=bf533 -mfast-fp
CFLAGS += -ffast-math -funroll-loops

# Custom DSP libraries
LDFLAGS += -lmydsplibrary
```

### Performance Optimization

#### AVR32 Optimization Tips

- Use `-O2` for balanced performance/size
- Enable specific CPU features with `-mcpu=uc3a0512`
- Use ASF optimized functions for peripherals
- Profile with `avr32-gprof` for hotspots

#### Blackfin DSP Optimization Tips

- Always use `-mcpu=bf533` for hardware optimization
- Use fixed-point arithmetic (`fract32`) instead of float
- Leverage L1 memory for critical code paths
- Use built-in DSP instructions via intrinsics

### Testing Modules

#### Unit Testing Framework

```bash
# Create test harness
mkdir test
cd test

# Test individual functions
cat > test_mynewmodule.c << 'EOF'
#include "../mynewmodule.c"

int main() {
    // Test your DSP function
    fract32 test_input = 0x40000000;  // 0.5 in fract32
    fract32 result = my_dsp_function(test_input);

    printf("Input: %d, Output: %d\n", test_input, result);
    return 0;
}
EOF

# Compile and run test
bfin-elf-gcc -o test_mynewmodule test_mynewmodule.c
# Note: This creates a test binary, not for hardware
```

## Troubleshooting Guide

### Common Build Errors

#### AVR32 Issues

| Error                              | Cause        | Solution                                          |
| ---------------------------------- | ------------ | ------------------------------------------------- |
| `avr32-gcc: command not found`     | PATH not set | `export PATH=/root/avr32-tools-6711e09/bin:$PATH` |
| `fatal error: asf.h: No such file` | Missing ASF  | Install ASF framework                             |
| `undefined reference to 'syscall'` | Missing libc | Use custom minimal headers                        |

#### Blackfin Issues

| Error                               | Cause             | Solution                  |
| ----------------------------------- | ----------------- | ------------------------- |
| `bfin-elf-gcc: command not found`   | Wrong container   | Use x86_64 container      |
| `no memory region specified`        | Bad linker script | Copy from lines/lines.lds |
| `undefined reference to '__mulsi3'` | Missing math lib  | Add `-lm` to LDFLAGS      |

### Performance Issues

- **Slow builds**: Use `make -j4` for parallel compilation
- **Large binaries**: Check optimization flags and remove debug symbols
- **Runtime issues**: Profile with appropriate tools for each architecture

### Container Issues

```bash
# Restart containers
docker restart aleph-avr32 aleph-blackfin

# Check resource usage
docker stats aleph-avr32 aleph-blackfin

# Access container logs
docker logs aleph-avr32
```

## Best Practices

### Code Organization

- Keep AVR32 firmware code in `/workspace/aleph/apps/`
- Keep Blackfin DSP modules in `/workspace/aleph/modules/`
- Use common headers from `/workspace/aleph/common/`
- Share DSP functions via `/workspace/aleph/dsp/`

### Version Control

- Commit working builds before major changes
- Tag successful firmware releases
- Keep separate branches for experimental features
- Document build configurations in commit messages

### Development Safety

- Always `make clean` before important builds
- Test modules individually before integration
- Keep backup copies of working configurations
- Use container snapshots for stable states

## Resources

### Documentation

- [Aleph Hardware Manual](https://monome.org/docs/aleph/)
- [Blackfin DSP Guide](https://monome.org/docs/aleph/dev/dsp/)
- [AVR32 Documentation](https://www.microchip.com/en-us/products/microcontrollers-and-microprocessors/32-bit-mcus/avr32-mcus)
- [ADI Blackfin Resources](https://www.analog.com/en/products/processors-dsp/blackfin.html)

### Community

- [Monome Community](https://llllllll.co/)
- [Aleph Development Forums](https://llllllll.co/c/aleph)
- [GitHub Issues](https://github.com/monome/aleph)

### Tools

- **AVR32 Tools**: GCC 4.4.7, Binutils 2.23.1, GDB
- **Blackfin Tools**: ADI 2014R1, LDR utilities, VDSP++
- **Debugging**: GDB, objdump, size, nm
- **Analysis**: Profiling tools, memory analyzers

---

**Happy Aleph Development!** 🎵  
_This environment provides professional-grade embedded development with real hardware compilation._
