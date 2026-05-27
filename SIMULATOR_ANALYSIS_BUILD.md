# Simulator Analysis & Build Guide

## Executive Summary

The Aleph project contains **two distinct simulator targets**, each serving a different layer of the hardware:

| Simulator | Layer | Language | Core Deps |
|-----------|-------|----------|-----------|
| `bfin_sim` | Blackfin DSP audio core | C | JACK, liblo (OSC) |
| `avr32_sim` (via `beekeep`) | AVR32 controller / BEES app | C | GTK3, Jansson |

Neither has a top-level `Makefile` of its own — they are included as source sets into other build targets. The `beekeep` utility at `utils/beekeep/` is the most complete desktop build of the AVR32 controller layer; it uses `avr32_sim` as a stub library.

This document covers everything needed to compile and run both simulators on modern macOS (Apple Silicon or x86_64).

---

## 1. Current Build System Analysis

### 1.1 Top-Level Makefile (`aleph-repo/Makefile`)

A thin Docker wrapper — it does **not** build native code. All targets shell out to Docker:

```makefile
make setup     # builds Docker image
make dev       # drops into interactive Docker container
make build-bees  # runs bees build inside Docker
```

The actual AVR32 build system lives in `libavr32/asf/avr32/utils/make/Makefile.avr32.in` (Atmel Software Framework). This is a cross-compile toolchain — not usable on the host for native builds.

### 1.2 `bfin_sim` — Blackfin DSP Simulator

**Location:** `utils/bfin_sim/`

**Build system:** No `Makefile` in the root; only `src/libfixmath/Makefile` (standalone static lib). The top-level sim has no standalone build rule.

**What it does:** Wraps a Blackfin audio DSP module as a JACK audio client. It:
1. Calls `module_init()` (from whatever `.ldr` module is linked)
2. Registers as a JACK client (`aleph_sim`)
3. On each JACK process callback: pumps `module_process_frame()` with audio buffers
4. Exposes an OSC server on port 7770 to set module parameters at runtime

**Architecture:** `bfin_sim/main.c` → JACK + liblo → calls `module.h` interface:
```c
void module_init(void);
void module_process_frame(void);
void module_set_param(u32 idx, ParamValue val);
```

Module code (e.g., `modules/lines/`, `modules/grains/`) must be compiled separately and linked in. When `ARCH_BFIN` is **not** set, `module.h` declares `in[4]`, `out[4]`, and `SDRAM_ADDRESS` as globals.

**Key source files:**
```
utils/bfin_sim/main.c          # JACK client + OSC server + main loop
utils/bfin_sim/fract_math.c    # Blackfin fract32 emulation on host
utils/bfin_sim/fract2float_conv.c  # fract32 <-> float conversion
utils/bfin_sim/src/fix.c       # fixed-point math
utils/bfin_sim/src/libfixmath/ # libfixmath (has its own Makefile)
```

### 1.3 `avr32_sim` — AVR32 Controller Simulator

**Location:** `utils/avr32_sim/`

**Build system:** `avr32_sim.mk` — a Makefile fragment (not standalone). It defines `src +=` lists and is `include`d by dependent projects.

**What it does:** Provides stub implementations of all AVR32 hardware peripherals so that the BEES application logic can compile and run on a desktop host. Hardware that is stubbed includes:
- ADC (no-op)
- OLED screen (functions exist, drawing is no-op)
- Encoders/switches (event queue, no GPIO)
- Filesystem (POSIX file I/O wrapping fat_io_lib)
- Flash (malloc-backed NVRAM)
- I2C/SPI/BFIN comms (stub or no-op)
- Timers (POSIX `gettimeofday` or no-op)
- USB (no-op stubs)

**How hardware is guarded:** AVR32 hardware code is wrapped in `#if 1 / #else` blocks where the `#if 1` path is the stub (compiled for host) and the `#else` path is the real hardware code. Example from `bfin.c`:
```c
void bfin_wait(void) {
#if 1
#else
  while (gpio_get_pin_value(BFIN_HWAIT_PIN) > 0) { ;; }
#endif
}
```

This means `avr32_sim` sources **already compile cleanly on host gcc** — the hardware paths are dead code.

**Architecture flags:**
- `BEEKEEP=1` — enables `#ifdef BEEKEEP` paths throughout (malloc, file I/O, etc.)
- `ARCH_AVR32=1` — still defined even in the sim; this selects the AVR32 type aliases in `compiler.h` (which is itself a stub in the sim)
- `DYNAMIC_NETWORK_ENABLED=1` — enables dynamic op networks in BEES

### 1.4 `beekeep` — The Most Complete Simulator Target

**Location:** `utils/beekeep/`

This is the real deliverable for simulating the controller layer. It compiles BEES operator logic + `avr32_sim` stubs + GTK3 UI into a desktop application.

**`utils/beekeep/Makefile` — what it actually does:**
```makefile
cflags += -I/opt/homebrew/Cellar/jansson/2.14.1/include
cflags += -std=gnu99 -D BEEKEEP=1 -D ARCH_AVR32=1 -D DYNAMIC_NETWORK_ENABLED=1
lflags += -L/opt/homebrew/Cellar/jansson/2.14.1/lib -Bstatic -ljansson
cflags += $(shell pkg-config --cflags gtk+-3.0)
lflags += -Bdynamic $(shell pkg-config --libs gtk+-3.0)
```

It uses **plain `gcc`** (not a cross-compiler). This is a native x86_64/arm64 build already. The Makefile **hardcodes Homebrew paths** for Jansson.

There is also a headless variant in `utils/beekeep/src/main_headless.c` that omits GTK entirely.

---

## 2. Dependencies Audit

### 2.1 `bfin_sim` Dependencies

| Library | Purpose | macOS Install | Notes |
|---------|---------|---------------|-------|
| **JACK** | Real-time audio I/O | `brew install jack` | JackOSX or JACK2 via Homebrew |
| **liblo** | OSC (Open Sound Control) | `brew install liblo` | For parameter control over OSC |
| **libm** | Math | Bundled with Xcode CLT | `-lm` |

**JACK on macOS caveat:** JACK on macOS requires a running JACK server (`jackd`). For CI/testing use the `-d dummy` driver:
```bash
jackd -d dummy &
```
Alternatively, consider a JACK-free variant using CoreAudio directly or PortAudio as a drop-in (see Section 5).

### 2.2 `avr32_sim` / `beekeep` Dependencies

| Library | Purpose | macOS Install | Already Present? |
|---------|---------|---------------|-----------------|
| **GTK3** | GUI toolkit | `brew install gtk+3` | ❌ Not installed |
| **Jansson** | JSON (scene files) | `brew install jansson` | ✅ (`jansson 2.14.1`) |
| **pkg-config** | Build helper | Bundled | ✅ |

**GTK3 on macOS:** GTK3 via Homebrew works but requires an X11/Quartz backend. Alternatively, the `main_headless.c` variant removes the GTK dependency entirely, making it much easier to build and test.

### 2.3 Check What's Already Installed

```bash
brew list | grep -E "jack|gtk|jansson|liblo|sdl2|pkg-config"
pkg-config --modversion gtk+-3.0 2>/dev/null || echo "gtk3 missing"
pkg-config --modversion liblo 2>/dev/null || echo "liblo missing"
jackd --version 2>/dev/null || echo "JACK missing"
```

Current state on this machine:
- ✅ `jansson` 2.14.1
- ✅ `sdl2` (installed but not needed by simulator directly)
- ✅ `pkg-config`
- ❌ `gtk+3` — needs install
- ❌ `jack` — needs install
- ❌ `liblo` — needs install

---

## 3. Desktop Compilation Strategy

### 3.1 Compiling BEES / `beekeep` for Host (arm64/x86_64)

The `beekeep` Makefile already targets the host compiler. The compilation model is:

```
gcc (native) + BEEKEEP=1 + ARCH_AVR32=1
    ├── apps/bees/src/*.c     (BEES operator logic — pure C, no hardware)
    ├── utils/avr32_sim/src/* (hardware stubs)
    └── utils/beekeep/src/*   (GTK UI + file I/O + JSON)
```

**Architecture-specific guidance:**

On Apple Silicon (arm64), `gcc` is actually Clang. This is fine — the code is standard C99. No AVR32 or Blackfin intrinsics appear in the simulator paths.

**Key compile flags to set:**
```makefile
CC = gcc  # or clang — both work
CFLAGS = -std=gnu99 -DBEEKEEP=1 -DARCH_AVR32=1 -DDYNAMIC_NETWORK_ENABLED=1
CFLAGS += -g -O0  # debug build
CFLAGS += -Wall -Wno-unused-variable  # sim stubs have many unused vars
```

**Universal binary (if needed):**
```bash
# arm64
CFLAGS += -arch arm64
# x86_64
CFLAGS += -arch x86_64
# Universal
CFLAGS += -arch arm64 -arch x86_64
```

### 3.2 Compiling `bfin_sim` for Host

The `bfin_sim` is a separate concern. It wraps a DSP module with JACK. The process:

1. Compile `libfixmath` for host
2. Compile the DSP module code (e.g., `modules/lines/`) with `ARCH_BFIN=0` and fract32 emulation
3. Compile `bfin_sim/main.c` against JACK + liblo

**Fract32 emulation:** `utils/bfin_sim/fract_math.c` provides host-side emulation of Blackfin fractional arithmetic. `fract2float_conv.c` bridges fract32 ↔ float for JACK I/O.

### 3.3 Headless Build (Easiest Path)

For CI and initial testing, the headless build is the best entry point — no GTK needed:

```bash
# Compile beekeep headless (no GTK)
gcc -std=gnu99 \
  -DBEEKEEP=1 -DARCH_AVR32=1 -DDYNAMIC_NETWORK_ENABLED=1 \
  -Iapps/bees -Iapps/bees/src -Icommon \
  -Iutils/avr32_sim -Iutils/avr32_sim/src \
  -Iutils/beekeep/src \
  $(find apps/bees/src -name "*.c") \
  $(find utils/avr32_sim/src -name "*.c" ! -path "*/libfixmath/*") \
  utils/beekeep/src/main_headless.c \
  utils/beekeep/src/app_beekeep.c \
  utils/beekeep/src/flash_beekeep.c \
  utils/beekeep/src/files.c \
  utils/beekeep/src/json_read_native.c \
  utils/beekeep/src/json_write_native.c \
  utils/beekeep/src/dot.c \
  -ljansson -o beekeep-headless
```

---

## 4. Mock Hardware Driver Stubs

### 4.1 What's Already Stubbed (in `avr32_sim/src/`)

The `avr32_sim` provides these stub implementations:

| Module | File | Stub Strategy |
|--------|------|---------------|
| **ADC** | `adc.c` | Returns zero / does nothing |
| **BFIN SPI** | `bfin.c` | All hardware transfers no-op'd with `#if 1/#else` |
| **Screen/OLED** | `screen.c` | Drawing functions are no-ops (no SDL/framebuffer) |
| **Encoders** | `encoders.c` | GPIO reads replaced with event injection |
| **Filesystem** | `filesystem.c` | POSIX file I/O via fat_io_lib |
| **Flash/NVRAM** | `flash.c` | `malloc`-backed under `#ifdef BEEKEEP` |
| **I2C** | `i2c.c` | No-op |
| **Timers** | `timers.c` | Software timer list (no hardware TC) |
| **USB** | `usb.c`, `usb/midi/`, `usb/ftdi/` | No-op stubs |
| **Serial** | `serial.c` | No-op or `printf` |
| **Print functions** | `print_funcs.c` | `printf`-backed |
| **Interrupts** | `interrupts.c` | No-op (no hardware IRQs) |
| **Memory** | `memory.c` | `malloc` under `BEEKEEP` |

### 4.2 Where `#ifdef BEEKEEP` Matters

The `BEEKEEP` flag enables POSIX/malloc paths in several files. Critical ones:

**`memory.c`:**
```c
#ifdef BEEKEEP
#include <stdlib.h>
#endif
// ...
// Under BEEKEEP: NVRAM is a malloc'd struct, not flash-mapped
static beesFlashData appFlashData;
```

**`flash.c`:**
```c
// Under BEEKEEP: reads/writes go to a host-side buffer, not AVR32 flashc
```

### 4.3 Screen Stub — Rendering Gap

The OLED screen has no visual output in the current simulator. The `screen_draw_region()` stub in `avr32_sim/src/screen.c` contains no SDL or framebuffer code — it just no-ops.

**To add visual output**, you'd replace `screen.c` with an SDL2 implementation:
```c
// screen_stub_sdl.c — example replacement
#include <SDL2/SDL.h>

static SDL_Window* window;
static SDL_Renderer* renderer;
static SDL_Texture* texture;
static uint8_t gram[4096];  // GRAM_BYTES

void screen_draw_region(u8 x, u8 y, u8 w, u8 h, u8* data) {
    // Decode 4-bit OLED gram → RGBA and blit to SDL texture
    // ...
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}
```

SDL2 is already installed on this machine (`brew list | grep sdl2` confirms it). This is the highest-value stub upgrade for visual development.

### 4.4 `#define` Guards Summary

```
ARCH_AVR32=1       — Enable AVR32 type aliases (compiler.h)
BEEKEEP=1          — Enable POSIX/malloc paths throughout
ARCH_BFIN=1        — Enable real Blackfin code (used in module builds, NOT in sim)
DYNAMIC_NETWORK_ENABLED=1 — Enable runtime op network growth in BEES
```

---

## 5. CI / Test Automation

### 5.1 What Can Be Tested Without Hardware

| Test Target | Feasibility | Notes |
|-------------|-------------|-------|
| libfixmath compilation | ✅ Easy | Standalone Makefile, pure C, no deps |
| `beekeep` headless build | ✅ Easy | Just needs Jansson |
| Scene file load/export | ✅ Moderate | Uses headless beekeep + `.scn` files |
| BEES operator unit tests | ✅ Moderate | Operators are pure functions |
| `bfin_sim` build | ⚠️ Needs JACK | JACK can run in dummy mode |
| DSP module correctness | ✅ Moderate | fract32 emu + known inputs/outputs |

### 5.2 Proposed GitHub Actions Workflow

```yaml
# .github/workflows/sim-build.yml
name: Simulator Build & Test

on: [push, pull_request]

jobs:
  build-beekeep-headless:
    runs-on: macos-14  # Apple Silicon runner

    steps:
      - uses: actions/checkout@v4

      - name: Install dependencies
        run: |
          brew install jansson pkg-config

      - name: Build libfixmath
        run: |
          cd utils/avr32_sim/src/libfixmath
          make
          echo "libfixmath: OK"

      - name: Build beekeep headless
        run: |
          cd utils/beekeep
          make HEADLESS=1 2>&1 | tee build.log
          test -f beekeep-headless

      - name: Smoke test — load scene file
        run: |
          ./utils/beekeep/beekeep-headless \
            utils/beekeep/test-scenes/empty.scn \
            --json-out /tmp/scene.json
          python3 -c "import json; d=json.load(open('/tmp/scene.json')); print('OK:', d)"

  build-bfin-sim:
    runs-on: ubuntu-22.04  # JACK easier on Linux

    steps:
      - uses: actions/checkout@v4

      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y libjack-jackd2-dev liblo-dev

      - name: Build libfixmath
        run: |
          cd utils/bfin_sim/src/libfixmath
          make

      - name: Build bfin_sim (stub module)
        run: |
          gcc -std=gnu99 \
            -Iutils/bfin_sim \
            -Iutils/bfin_sim/src \
            -Icommon \
            -Ibfin_lib/src \
            utils/bfin_sim/main.c \
            utils/bfin_sim/fract_math.c \
            utils/bfin_sim/fract2float_conv.c \
            utils/bfin_sim/src/fix.c \
            test/bfin_sim/stub_module.c \
            -ljack -llo -lm -o bfin_sim
```

### 5.3 Local Test Script

```bash
#!/bin/bash
# scripts/test-sim.sh — local smoke test for simulator builds

set -e

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$REPO_ROOT"

echo "=== Building libfixmath ==="
(cd utils/avr32_sim/src/libfixmath && make -s && echo "OK")

echo "=== Building beekeep headless ==="
# Requires: brew install jansson
(cd utils/beekeep && make HEADLESS=1 -s && echo "OK")

echo "=== Smoke test: app_init() runs without crash ==="
./utils/beekeep/beekeep-headless --self-test && echo "PASS"

echo "=== All simulator tests passed ==="
```

### 5.4 Pass/Fail Harness for DSP Correctness

For `bfin_sim`, a simple deterministic test harness:

```c
// test/bfin_sim/test_module.c
#include <assert.h>
#include "module.h"

int main(void) {
    SDRAM_ADDRESS = malloc(SDRAM_SIZE);
    module_init();

    // Set known inputs
    in[0] = float_to_fr32(0.5f);
    in[1] = float_to_fr32(-0.5f);
    in[2] = 0;
    in[3] = 0;

    // Run one frame
    module_process_frame();

    // Verify outputs are in valid range
    assert(fr32_to_float(out[0]) >= -1.0f);
    assert(fr32_to_float(out[0]) <=  1.0f);

    printf("PASS: module_process_frame() produces valid output\n");
    return 0;
}
```

---

## 6. Step-by-Step: Hello World Simulator Build

This section gets a working headless beekeep build running on macOS.

### Prerequisites

- macOS 12+ (Monterey or later)
- Xcode Command Line Tools: `xcode-select --install`
- Homebrew: `/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"`

### Step 1: Install Dependencies

```bash
brew install jansson pkg-config

# Verify
jansson_version=$(pkg-config --modversion jansson)
echo "Jansson: $jansson_version"
```

For the GTK3 (full GUI) build, also:
```bash
brew install gtk+3
# Note: First time this takes ~5-10 min, it's a large dependency tree
```

### Step 2: Build libfixmath

```bash
cd ~/aleph-repo/utils/avr32_sim/src/libfixmath
make
# Expect: libfixmath.a created
ls -la libfixmath.a
```

### Step 3: Build beekeep Headless

The `beekeep` Makefile needs a small patch to support headless mode. Add this to `utils/beekeep/Makefile`:

```makefile
# Add near top of Makefile, before target rules:
ifdef HEADLESS
  src := $(filter-out src/ui.c src/ui_files.c src/ui_handlers.c \
                      src/ui_lists.c src/ui_op_menu.c, $(src))
  src += src/main_headless.c
  cflags += -D HEADLESS=1
  # Remove GTK deps
  cflags := $(filter-out $(shell pkg-config --cflags gtk+-3.0), $(cflags))
  lflags := $(filter-out $(shell pkg-config --libs gtk+-3.0), $(lflags))
  lflags := $(filter-out -Bdynamic, $(lflags))
else
  src += src/main.c
endif
```

Then build:
```bash
cd ~/aleph-repo/utils/beekeep
make HEADLESS=1
# Binary: beekeep-<version>
```

If the above Makefile patch is too invasive, here's a direct compile command that doesn't require Makefile changes:

```bash
cd ~/aleph-repo

JANSSON_PREFIX=$(brew --prefix jansson)
SIM=utils/avr32_sim/src
BEES=apps/bees/src
BEEKEEP=utils/beekeep/src
COMMON=common

# Gather sources
SIM_SRCS=$(find $SIM -name "*.c" ! -path "*/libfixmath/*")
BEES_SRCS=$(find $BEES -name "*.c")
BEEKEEP_SRCS="$BEEKEEP/app_beekeep.c $BEEKEEP/flash_beekeep.c \
  $BEEKEEP/files.c $BEEKEEP/json_read_native.c $BEEKEEP/json_write_native.c \
  $BEEKEEP/dot.c $BEEKEEP/param_scaler.c \
  $BEEKEEP/embedded_descriptors.c $BEEKEEP/embedded_descriptors_data.c"
MAIN="$BEEKEEP/main_headless.c"

gcc -std=gnu99 \
  -DBEEKEEP=1 -DARCH_AVR32=1 -DDYNAMIC_NETWORK_ENABLED=1 \
  -I$COMMON \
  -Iutils/avr32_sim -I$SIM -I$SIM/usb -I$SIM/usb/midi -I$SIM/usb/ftdi \
  -I$SIM/usb/hid -I$SIM/usb/mouse -I$SIM/usb/hub \
  -Iapps/bees -I$BEES \
  -I$BEEKEEP \
  -I$JANSSON_PREFIX/include \
  -g -O0 -Wall -Wno-unused-variable -Wno-unused-function \
  $BEES_SRCS $SIM_SRCS $BEEKEEP_SRCS $MAIN \
  -L$JANSSON_PREFIX/lib -ljansson \
  -o beekeep-headless 2>&1 | head -50

echo "Exit: $?"
```

### Step 4: Run It

```bash
./beekeep-headless
# Expected: usage message (no scene file provided)

# Or with a scene file:
./beekeep-headless apps/bees/scenes/some.scn
```

### Step 5: Build `bfin_sim` (needs JACK + liblo)

```bash
brew install jack liblo

# Start JACK in dummy mode (no audio hardware needed)
jackd -d dummy &
sleep 1

# Build bfin_sim against a module (using a stub for quick test)
cat > /tmp/stub_module.c << 'EOF'
#include "module.h"
ModuleData gModuleData_data = { "stub", NULL, 0 };
ModuleData* gModuleData = &gModuleData_data;
u8 dbgFlag = 0;
void module_init(void) { SDRAM_ADDRESS = malloc(SDRAM_SIZE); }
void module_deinit(void) { free(SDRAM_ADDRESS); }
void module_process_frame(void) { out[0]=in[0]; out[1]=in[1]; out[2]=in[2]; out[3]=in[3]; }
void module_set_param(u32 idx, ParamValue val) {}
u32 module_get_num_params(void) { return 0; }
EOF

JACK_PREFIX=$(brew --prefix jack)
LO_PREFIX=$(brew --prefix liblo)

gcc -std=gnu99 \
  -Iutils/bfin_sim -Iutils/bfin_sim/src -Icommon -Ibfin_lib/src \
  -I$JACK_PREFIX/include -I$LO_PREFIX/include \
  utils/bfin_sim/main.c \
  utils/bfin_sim/fract_math.c \
  utils/bfin_sim/fract2float_conv.c \
  utils/bfin_sim/src/fix.c \
  /tmp/stub_module.c \
  -L$JACK_PREFIX/lib -ljack \
  -L$LO_PREFIX/lib -llo \
  -lm -o bfin_sim

# Run it (connects to running jackd)
./bfin_sim
```

Expected output:
```
bang osc port 7770 @ /param with two ints to test module
engine sample rate: 48000
```

Send OSC parameter commands with any OSC client:
```bash
# Using oscsend (from liblo tools)
oscsend localhost 7770 /param ii 0 8192
```

---

## 7. Known Issues & Gotchas

### 7.1 Hardcoded Jansson Path in `beekeep/Makefile`

```makefile
cflags += -I/opt/homebrew/Cellar/jansson/2.14.1/include
lflags += -L/opt/homebrew/Cellar/jansson/2.14.1/lib
```

This breaks if Jansson is upgraded. Replace with:
```makefile
cflags += $(shell pkg-config --cflags jansson)
lflags += $(shell pkg-config --libs jansson)
```

### 7.2 libfixmath `fix16_fract.h` Missing `fix.h`

Running `make` in `utils/bfin_sim/src/libfixmath/` fails:
```
fix16_fract.c:1: fatal error: 'fix.h' file not found
```

Fix: add `-I../../` to the libfixmath Makefile's `INC`:
```makefile
INC = -I../../
```
Or exclude `fix16_fract.c` from the libfixmath build (it's not used by the sim).

### 7.3 `sleep(-1)` in `bfin_sim/main.c`

The main loop calls `sleep(-1)` on non-Windows, which on macOS immediately returns (it's a `unsigned int` argument — wraps to a very large value on some platforms, or 0). The JACK process callback runs in a separate thread, so the main thread should be `pause()`'d instead:

```c
// Replace:
sleep(-1);
// With:
while(1) pause();
```

### 7.4 Duplicate Source Files with Spaces

The repo contains files named `foo 2.c` and `foo 2.h` (e.g., `avr32/src/debug 2.h`). These appear to be stale copies from macOS Finder duplicate-file behavior. They should not be included in build rules. All active build rules reference only the non-suffixed filenames.

### 7.5 No Standalone Makefile for `bfin_sim`

`utils/bfin_sim/` has no top-level Makefile. You must either write one or use the direct compile command above. This is the most significant gap in the build system.

---

## 8. Dependency Install Cheatsheet

```bash
# All deps for both simulators on macOS (Homebrew):
brew install jansson pkg-config gtk+3 jack liblo sdl2

# Verify all present:
for pkg in jansson gtk+-3.0 jack liblo sdl2; do
  ver=$(pkg-config --modversion $pkg 2>/dev/null) \
    && echo "✅ $pkg $ver" \
    || echo "❌ $pkg MISSING"
done
```

---

## 9. Recommended Next Steps

1. **Fix the libfixmath build** — add `-I../../` to `INC` in both `avr32_sim` and `bfin_sim` libfixmath Makefiles
2. **Add a `Makefile` to `bfin_sim`** — there's no standalone build rule; write one
3. **Upgrade `beekeep/Makefile`** to use `pkg-config` for Jansson instead of hardcoded paths
4. **Add headless build target** to `beekeep/Makefile` (`make headless`)
5. **Add SDL2 screen renderer** to `avr32_sim/src/screen.c` for visual output in the simulator
6. **Add GitHub Actions** — start with the headless beekeep build on `macos-14`; add `bfin_sim` on `ubuntu-22.04`
7. **Fix `sleep(-1)`** in `bfin_sim/main.c` → `while(1) pause()`
8. **Write a stub module** in `utils/bfin_sim/` for quick bfin_sim testing without a real DSP module

---

*Generated by analysis of `utils/avr32_sim/`, `utils/bfin_sim/`, `utils/beekeep/`, `apps/bees/`, `common/`, and all associated Makefiles. All observations are grounded in actual source code.*
