# Alternative Emulation Approaches for Aleph Testing

**Author**: Polly (direct research + synthesis)
**Date**: 2026-05-27
**Repos analyzed**: `btrailor/aleph` (fork, `develop` branch)

---

## Executive Summary

The Aleph hardware (AVR32 UC3A0512 + Blackfin BF533) is from a discontinued architecture family with very limited emulation support. After investigating QEMU, Renode, gem5, hardware dev boards, and transpilation approaches, **no single tool provides out-of-the-box Aleph emulation**. However, there are viable paths:

| Approach | Viability | Effort | Best For |
|----------|-----------|--------|----------|
| **Extend existing avr32_sim** | ✅ **YES** | Medium | Scene migration, network testing, UI logic |
| **QEMU AVR32 (flogosec)** | ⚠️ **MAYBE** | Large | Running actual AVR32 binaries, security research |
| **Renode** | ❌ **NO** | — | No AVR32 or Blackfin support |
| **gem5** | ❌ **NO** | — | No AVR32 support; targets different class of CPUs |
| **AVR32 dev board** | ✅ **YES** | Small | Quick smoke tests, CDC grid testing |
| **Blackfin emulator** | ⚠️ **MAYBE** | Medium | DSP module testing (separate from AVR32) |
| **AVR32→x86 transpilation** | ⚠️ **MAYBE** | Large | Fast desktop iteration, but needs stubbing |
| **Unicorn Engine** | ⚠️ **MAYBE** | Large | Low-level instruction emulation |

---

## 1. QEMU AVR32

### Status: EXISTS but immature

**Research findings**:
- **Florian Göhler's qemu-avr32** (github.com/flogosec/qemu-avr32) — first and only AVR32 implementation for QEMU
- Built as a Master's thesis project for satellite firmware security research (2023)
- Supports most AVR32 instruction set; missing Java bytecode instructions and some FPU operations
- Ships with an `avr32example-board` for testing; also has a `Nanomind A3200` board branch
- **Not upstreamed** to QEMU mainline (as of 2023)
- Includes AFL++ fuzzing integration and a testing framework
- Active blog series documenting QEMU architecture addition (Parts 1–8 as of 2023)

**Build process**:
```bash
../configure --target-list=avr32-softmmu
make -j 16
./build/avr32-softmmu/qemu-system-avr32 -M avr32example-board -bios [firmware.bin]
```

**Assessment for Aleph**:
- **Viability**: ⚠️ MAYBE — The UC3A0512 is not the same as the AP7000/A3200 that the existing board targets
- UC3A is a microcontroller; AP7 is an application processor. Peripherals (USBB, SPI, GPIO matrix, DMA) differ significantly
- Would need substantial board definition work to model UC3A0512 + the Aleph's specific peripheral mix
- The SPI Blackfin interface, USB host stack, and OLED screen would all need emulation
- **Effort**: Large (weeks to months) — need to define UC3A0512 SoC model, all peripherals, memory map
- **Relevance**: Good for running actual compiled `.hex` binaries; overkill for BEES scene/network testing

---

## 2. Renode Emulator

### Status: NO AVR32 OR BLACKFIN SUPPORT

**Research findings**:
- Renode (renode.io, github.com/renode/renode) — modern open-source embedded simulation framework from Antmicro
- Supports: x86 (Intel Quark), Cortex-A (NVIDIA Tegra), Cortex-M, SPARC (Leon), RISC-V, and many ARM-based SoCs
- **200+ supported boards** including many Atmel/Microchip SAM devices
- **No AVR32 support** in any supported boards list (checked official docs, Renodepedia, Zephyr dashboard)
- **No Blackfin support** — Blackfin is a VLIW DSP architecture, not in Renode's scope
- Platform definitions use `.repl` files describing hardware components and memory maps

**Assessment for Aleph**:
- **Viability**: ❌ NO — would require adding AVR32 architecture + UC3A0512 SoC from scratch
- Renode's architecture is modular, but AVR32 is entirely absent
- **Effort**: Very large (months) — need new CPU model, all peripherals, plus Blackfin
- **Relevance**: N/A

---

## 3. Other Embedded Simulators

### gem5
- Targets: x86, ARM, RISC-V, SPARC, MIPS, POWER, Alpha
- **No AVR32 support**
- Overkill for microcontroller simulation — designed for desktop/server architecture research
- **Viability**: ❌ NO

### Unicorn Engine
- Lightweight CPU emulator built on QEMU
- Supports all QEMU architectures including AVR (8-bit)
- **No AVR32 support** — only AVR 8-bit (ATmega/ATtiny)
- Could theoretically be extended, but same effort as QEMU AVR32
- **Viability**: ⚠️ MAYBE (if someone implements AVR32 core)

### SimAVR / simulavr
- AVR 8-bit simulators (ATmega, ATtiny)
- **No AVR32 support**
- **Viability**: ❌ NO

### OshonSoft AVR Simulator
- Windows-only IDE with BASIC compiler
- AVR 8-bit only
- **Viability**: ❌ NO

---

## 4. AVR32 → x86 Transpilation

### Status: THEORETICALLY POSSIBLE

**Concept**: Recompile BEES firmware for x86_64/arm64 host with hardware driver stubs replacing AVR32-specific code.

**What's already done**:
- The `avr32_sim` is essentially this approach — SDL GUI, mocked hardware drivers
- `avr32/src/` has many files with `#ifdef` patterns for platform-specific code
- `types.h` already has platform abstraction (`u8`, `u16`, `u32`, `s32`, `fract32`)

**What's missing**:
- AVR32-specific startup code (`sys.c`, interrupt vectors)
- Hardware register access (GPIO, SPI, USBB, TWI, USART)
- Memory layout (internal SRAM vs. external SDRAM)
- Real-time behavior (timers, event loop timing)

**Assessment**:
- **Viability**: ⚠️ MAYBE — The `avr32_sim` proves partial success
- **Effort**: Medium-to-Large — requires systematic stubbing of all hardware interfaces
- **Relevance**: Best path for BEES scene migration and network testing
- **Advantage**: Fast compile/run cycle, no binary translation overhead

---

## 5. Web/Container/Cloud Options

### Cloud CI for AVR32
- No known CI service supports AVR32 emulation
- Could potentially run `avr32_sim` in Docker on macOS/Linux
- `avr32_sim` depends on SDL and JACK — headless CI would need Xvfb or headless SDL
- **Viability**: ⚠️ MAYBE (for simulator CI, not for actual AVR32 binaries)

### Containerized Builds
- The Aleph cross-compilation toolchain (avr32-gcc, avr32binutils) could run in Docker
- But still requires physical device for testing
- **Viability**: ✅ YES for build automation, ❌ NO for testing

---

## 6. Hardware-in-the-Loop Alternatives

### AVR32 Dev Boards (Available!)

**EVK1105** — AT32UC3A0512 evaluation kit (same chip as Aleph AVR32!)
- Available on eBay (~$50-150 used)
- Also available from some electronics distributors (Mouser, Digi-Key, Newark)
- Reference hardware with QVGA LCD, audio codec, USB, SD card slot
- **Perfect for testing BEES logic** without risking the actual Aleph
- Could flash BEES (or a test variant) to this board

**STK1000** — AT32AP7000 development board (different chip family)
- Available from some surplus dealers (~$160)
- AP7000 is not UC3A — different architecture (AP7 vs. UC3)
- **Not suitable** for Aleph firmware testing

**Other UC3A boards**:
- EVK1100, EVK1101 — earlier UC3A variants, still useful
- Various third-party boards from ALVIDI and others

### JTAG Debuggers

**AVR32 JTAG**:
- Atmel AVR JTAGICE mkII or AVR Dragon (discontinued, available used)
- aJTAG or AVR32-specific debuggers on eBay
- **Critical**: Enables single-step debugging, breakpoints, memory inspection
- **Far superior** to printf debugging on physical hardware

**Blackfin JTAG**:
- ADI ICEbear (~€200-300 from section5.ch) — USB JTAG for Blackfin
- Emlink ICE for Blackfin (~$150-250) — works with VisualDSP++
- Lauterbach TRACE32 — professional, very expensive
- **Needed for**: DSP module debugging, audio processing verification

### Logic Analyzers
- Cheap 8-channel USB logic analyzers ($10-30) — Saleae clones
- Useful for SPI bus sniffing between AVR32 and Blackfin
- Can verify protocol behavior without firmware intervention

### Raspberry Pi as SPI Monitor
- Can bit-bang SPI slave to listen to AVR32↔Blackfin traffic
- Log all SPI transactions to SD card
- **Useful for**: Reverse-engineering the SPI protocol, verifying DSP load sequence

---

## 7. Recommendations by Testing Need

### Scene Migration Testing
| Approach | Fit |
|----------|-----|
| Extend avr32_sim | ⭐⭐⭐⭐⭐ Best — serialize/deserialize without hardware |
| QEMU AVR32 | ⭐⭐⭐ Overkill, but would work |
| EVK1105 board | ⭐⭐⭐ Good for integration testing |
| Transpilation | ⭐⭐⭐⭐ Same as extending sim |

### Network Integrity Testing
| Approach | Fit |
|----------|-----|
| Extend avr32_sim | ⭐⭐⭐⭐⭐ Best — create/connect/disconnect operators in headless mode |
| QEMU AVR32 | ⭐⭐⭐ Would work but slow |
| EVK1105 board | ⭐⭐⭐ Good for real-world validation |

### CDC Grid Behavior Testing
| Approach | Fit |
|----------|-----|
| Extend avr32_sim | ⭐⭐⭐ Needs USB host simulation layer |
| QEMU AVR32 | ⭐⭐⭐⭐ If UC3A USBB peripheral emulated |
| EVK1105 board | ⭐⭐⭐⭐⭐ Perfect — has USB, can connect real grid |
| JTAG debugger | ⭐⭐⭐ Can trace USB ISR execution |

### DSP Module Testing
| Approach | Fit |
|----------|-----|
| bfin_sim (extend) | ⭐⭐⭐⭐ Needs parameter interface + audio I/O |
| ICEbear JTAG | ⭐⭐⭐⭐⭐ Best for real Blackfin debugging |
| Emlink ICE | ⭐⭐⭐⭐ Good alternative |

---

## 8. Synthesis: Recommended Path

**Phase 1 (Immediate)**: Extend `avr32_sim` with headless test harness
- Add mock SPI Blackfin that responds to DSP load protocol
- Add SD card file system simulation (RAM-backed)
- Add event injection API (simulate encoder turns, key presses)
- **Goal**: Run scene migration tests, network integrity tests without hardware
- **Effort**: 2-3 weeks focused work

**Phase 2 (Short-term)**: Acquire EVK1105 board
- Flash BEES test build to EVK1105
- Test CDC grid compatibility with real monome hardware
- Validate scene migration on actual AVR32 silicon
- **Effort**: $100-200 + 2-3 days setup

**Phase 3 (Medium-term)**: Evaluate QEMU AVR32 extension
- If Phase 1 reveals simulator limitations, invest in QEMU board definition
- Port UC3A0512 peripheral models from existing work
- **Effort**: 1-2 months, requires deep QEMU knowledge

**Phase 4 (Long-term)**: Blackfin integration
- Extend `bfin_sim` or acquire ICEbear for DSP debugging
- Only needed if creating new audio modules
- **Effort**: 2-4 weeks

---

## References

1. **QEMU AVR32** — github.com/flogosec/qemu-avr32, fgoehler.com/projects/qemu-avr32/
2. **Renode** — renode.io, github.com/renode/renode, renode.readthedocs.io
3. **gem5** — gem5.org, github.com/gem5/gem5
4. **AVR32 EVK1105** — microchip.com/en-us/development-tool/ATEVK1105
5. **ICEbear JTAG** — section5.ch/dsp/icebear/
6. **Emlink ICE** — quickembed.com/en/ADIEm.htm
7. **Lauterbach Blackfin** — lauterbach.com/pdf/debugger_blackfin.pdf
8. **AVR32 on QEMU (blog series)** — fgoehler.com/blog/adding-a-new-architecture-to-qemu-01/
9. **QEMU AVR (8-bit)** — qemu-project.gitlab.io/qemu/system/target-avr.html
10. **FreeRTOS AVR32 demos** — docs.freertos.org/Documentation/02-Kernel/03-Supported-devices/04-Demos/Atmel-now-Microchip/portAVR32
