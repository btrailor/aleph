# Aleph Firmware — Codebase Comprehension Document

**Author**: Polly (automated analysis)  
**Date**: 2026-05-27  
**Repos analyzed**:
- `monome/aleph` (upstream baseline, bees 0.7.1 → 0.8.x)
- `btrailor/aleph` (fork, `develop` branch with CDC + scene migration)

---

## 1. Hardware Architecture

```
┌─────────────────────────────────────────┐
│              ALEPH DEVICE               │
├─────────────────┬───────────────────────┤
│  AVR32 UC3A0512 │   Blackfin BF533       │
│  (Controller)   │   (DSP / Audio)        │
├─────────────────┼───────────────────────┤
│ • Screen (OLED) │ • Audio I/O            │
│ • Encoders (4)  │ • DSP Processing       │
│ • Switches (4)  │ • CV Output            │
│ • USB Host      │                        │
│ • SD Card       │                        │
│ • I2C           │                        │
│ • CV Input      │                        │
└─────────────────┴───────────────────────┘
              │
              ▼ SPI Bus (3-wire serial)
```

**Communication**: AVR32 boots the Blackfin by sending `.ldr` binaries over SPI. Runtime parameter changes also flow over SPI.

---

## 2. Software Architecture

### Directory Structure

```
aleph/
├── apps/
│   └── bees/              ← Main control application (BEES)
│       ├── src/
│       │   ├── app_bees.c       ← App lifecycle (init, launch)
│       │   ├── net.c/h          ← Operator network (routing graph)
│       │   ├── scene.c/h        ← Scene save/load (serialization)
│       │   ├── op.c/h           ← Operator base class + registry
│       │   ├── op_pool.c/h      ← Memory management for ops
│       │   ├── render.c         ← Screen rendering
│       │   ├── pages.c          ← UI page system
│       │   ├── play.c           ← Play mode handling
│       │   ├── files.c          ← SD card file I/O
│       │   ├── scalers/         ← Parameter value conversion
│       │   └── ops/             ← 60+ operator implementations
│       ├── version.mk           ← BEES version (0.8.3 in develop)
│       └── Makefile
├── avr32/
│   └── src/               ← Low-level AVR32 drivers
│       ├── main.c               ← Event loop, USB handlers
│       ├── app.c                ← App framework
│       ├── bfin.c               ← Blackfin SPI communication
│       ├── screen.c             ← OLED display driver
│       ├── encoders.c           ← Rotary encoder input
│       ├── switches.c           ← Panel switch input
│       ├── usb/                 ← USB stack (FTDI, HID, MIDI)
│       └── ...
├── bfin_lib/
│   └── src/               ← Blackfin low-level audio
├── bfin_lib_block/
│   └── src/               ← Block-processing variant
├── common/
│   ├── types.h                  ← Shared type definitions
│   └── param_common.h           ← Parameter protocol
├── dsp/
│   └── ...                ← Audio processing library
├── modules/
│   ├── lines/             ← Delay/loop module
│   ├── mix/               ← Mixer
│   ├── waves/             ← Wavetable synth
│   ├── fmsynth/           ← FM synthesizer
│   └── pitch_shift/       ← Pitch shifter
├── libavr32/              ← Git submodule (monome's AVR32 library)
│   ├── src/
│   │   ├── usb/cdc/             ← CDC USB transport (NEW)
│   │   ├── monome.c             ← Monome grid detection
│   │   └── ...
├── utils/
│   ├── avr32_sim/         ← AVR32 simulator (SDL + JACK)
│   ├── bfin_sim/          ← Blackfin simulator (JACK audio)
│   ├── beekeep/           ← Scene editor (Python/PD)
│   └── avr32_boot/        ← Bootloader
└── development/docs/      ← Your comprehensive documentation
```

---

## 3. Core Systems

### 3.1 BEES Application Lifecycle (`app_bees.c`)

```c
app_init()      → Initialize memory pools, presets, network, scene, files, render, flash, serial
app_launch()    → Boot sequence:
  1. Render "BEES" + version on screen
  2. Wait for SD card
  3. If first-run: write scaler data, load default DSP module
  4. If normal: load scene (clean or default), wait for DSP, enable audio
  5. Initialize pages, play mode, timers, event handlers
```

**Launch States**:
- `eLaunchStateFirstRun` — Initial setup, writes flash data
- `eLaunchStateNormal` — Load default scene
- `eLaunchStateClean` — Load "clean.scn" (panic button)

### 3.2 Operator Network (`net.c/h`)

The heart of BEES: a dynamic routing graph where operators process control-rate signals.

**Data Structures**:
```c
typedef struct {
  u16 numOps;           // operator count
  u16 numIns;           // input node count
  u16 numOuts;          // output node count
  u16 numParams;        // parameter count
  op_t* ops[NET_OPS_MAX];
  inode_t* inodes;
  onode_t* onodes;
  pnode_t* pnodes;
  // ...
} ctlnet_t;
```

**Key Functions**:
- `net_init()` — Initialize network (fixed-size arrays)
- `net_add_op(opId)` — Create operator, auto-connect I/O
- `net_connect(outIdx, inIdx)` — Connect output → input
- `net_activate(op, outIdx, val)` — Propagate value through network
- `net_pickle()/net_unpickle()` — Serialize/deserialize scene

### 3.3 Scene System (`scene.c/h`)

Scenes are the "patches" — complete network configurations saved to SD card.

**Format**:
```c
typedef struct {
  sceneDesc_t desc;           // Name, module name, versions
  u8 pickle[SCENE_PICKLE_SIZE];  // Serialized network data (256KB)
} sceneData_t;
```

**Operations**:
- `scene_write_buf()` — Serialize current network to RAM buffer
- `scene_read_buf()` — Deserialize from RAM buffer to network
- `scene_write_default()` — Save as default scene
- `scene_read_default()` — Load default scene
- `scene_read_clean()` — Load clean-boot scene

**Version Compatibility Issue** (CRITICAL):
- 0.7.1 scenes store operator IDs and output indices directly
- 0.8.x added dummy outputs to some operators (BARS, BIGNUM, SCREEN, etc.)
- This shifts all output indices after those operators → connections break
- **Solution**: `scene_convert.c` (stub) + `OPERATOR_ID_MAPPING.h` + `OPERATOR_OUTPUT_CHANGES.h`

### 3.4 Operator System (`op.c/h`)

**Base Class** (`op_t`):
```c
typedef struct op_struct {
  u8 numInputs, numOutputs;
  op_in_fn* in_fn;          // Input processing functions
  op_out_t* out;            // Output target indices
  const char* opString;     // Display name
  u32 type;                 // Operator class ID
  u32 flags;                // Behavior flags
  void* child;              // Derived class data
} op_t;
```

**Registry** (`op_registry[numOpClasses]`):
- 67 operator classes (0.8.x)
- 60+ user-creatable operators
- System ops (ADC, ENC, SW, etc.) added automatically

**Operator Categories**:
- **Input**: ENC, ADC, SW, FTDI/CDC grid, MIDI, HID
- **Math**: ADD, MUL, DIV, SUB, MOD, BITS, SHL, SHR
- **Logic**: GATE, THRESH, IS, LOGIC, CHANGE
- **Timing**: METRO, TIMER, DELAY, CKDIV
- **Data**: LIST2/4/8/16, MEM0D/1D/2D, ACCUM, HISTORY
- **Display**: SCREEN, BARS8, BIGNUM
- **Monome**: GRID_CLASSIC, GRID_RAW, WW, KRIA, CASCADES
- **MIDI**: MIDI_NOTE, MIDI_CC, MIDI_CLOCK, MIDI_PROG
- **DSP**: PARAM (bridges to Blackfin parameters)

### 3.5 Rendering (`render.c`, `pages.c`)

**Screen**: 128×64 OLED, 1-bit (monochrome)

**Page System**:
- **PLAY** — Live performance view (selected inputs/outputs)
- **INS** — Input value editor
- **OPS** — Operator list and creation
- **PRESETS** — Preset management
- **SCENES** — Scene load/save
- **DSP** — Module selection and parameters

**Rendering Pipeline**:
```
Event → page handler → fill region buffers → screen_refresh() → SPI to OLED
```

---

## 4. Your Fork Deltas (`btrailor/aleph` vs `monome/aleph`)

### 4.1 CDC USB Grid Support (Major Feature)

**Problem**: Modern monome grids (2021+) use USB CDC (VID 0x0483) instead of FTDI (VID 0x16c0).

**Solution** (in `libavr32` + `avr32/src/main.c`):
- `usb/cdc/cdc.c` — CDC driver implementation
- `monome_transport.c/h` — Transport abstraction (FTDI ↔ CDC)
- `monome.c` — Grid detection: string encoding fix, serial pattern matching
- `avr32/src/main.c` — `handler_CdcConnect/CdcDisconnect` events

**Files Added/Modified**:
```
libavr32/src/usb/cdc/         ← CDC driver (new)
libavr32/src/monome.c         ← Transport detection, grid size
avr32/src/monome_transport.c  ← Transport abstraction (new)
avr32/src/monome_transport.h  ← Transport API (new)
avr32/src/main.c              ← CDC event handlers
apps/bees/src/ops/op_monome_grid_*.c  ← Grid focus fixes
```

### 4.2 Scene Migration Framework (WIP)

**Files Added**:
```
apps/bees/src/scene_convert.c       ← Conversion logic (stub)
apps/bees/src/scene_convert.h       ← Conversion API
apps/bees/src/OPERATOR_ID_MAPPING.h   ← 67-operator ID map
apps/bees/src/OPERATOR_OUTPUT_CHANGES.h  ← Output shift analysis
```

**Status**: Analysis complete, stub implemented, actual conversion logic pending.

### 4.3 Dynamic Network (Experimental)

**Files Added**:
```
apps/bees/src/dynamic_network.c  ← Expandable arrays for ops/ins/outs/params
apps/bees/src/dynamic_network.h  ← Dynamic allocation API
apps/bees/src/dynamic_flash_buffer.c  ← Flash-backed dynamic buffer
```

**Concept**: Replace fixed `NET_OPS_MAX=128`, `NET_INS_MAX=256` with expandable arrays that grow as needed. Start small (16 ops, 64 ins), double when full.

**Status**: Header complete, implementation in `net.c` with `#ifdef DYNAMIC_NETWORK_ENABLED`.

### 4.4 Build System & Tooling

**Your Additions**:
```
development/                    ← Comprehensive docs
├── docs/                       ← Architecture, planning, testing, workflow
├── docker/Dockerfile.avr32     ← Build environment
├── builds/                     ← Debug tools
BUILD.md                        ← This guide
```

---

## 5. Emulation & Testing Options

### 5.1 Existing Simulators (from monome repo)

**AVR32 Simulator** (`utils/avr32_sim/`):
- Platform: Desktop SDL (graphics) + optional JACK (audio)
- Implements: screen, encoders, switches, timers, events, filesystem, monome
- **Status**: Skeleton exists, `main.c` calls `app_init(); app_launch(0); return 0;`
- Missing: USB stack simulation, SD card simulation, SPI to Blackfin

**Blackfin Simulator** (`utils/bfin_sim/`):
- Platform: JACK audio server
- Wraps DSP modules as JACK clients
- **Status**: Functional for testing audio algorithms
- Limitation: No SPI parameter communication with AVR32

### 5.2 What's Needed for Firmware Testing Without Hardware

The simulators are incomplete. For meaningful firmware testing without flashing:

**Option A: Extend AVR32 Simulator**
- Add USB host simulation (for grid/midi connections)
- Add SD card file simulation (for scene I/O)
- Add mock SPI Blackfin (responds to DSP load/parameter commands)
- Add serial debug output capture (already partially there via `print_funcs`)
- **Effort**: Moderate — mostly plumbing, not algorithmic

**Option B: Headless Test Harness**
- Compile BEES app for desktop (not AVR32 target)
- Replace hardware drivers with mock implementations
- Run scene load/unpickle cycles as unit tests
- Validate network connectivity after unpickling
- **Effort**: Low-Moderate — your `avr32_sim` is close to this

**Option C: QEMU AVR32 Emulation**
- QEMU has some AVR32 support (experimental)
- Would run actual `.elf` firmware binary
- **Effort**: High — AVR32 QEMU support is minimal/unmaintained

**Option D: Beekeep Scene Validator**
- The desktop scene editor (`utils/beekeep/`) can load scenes
- Can validate scene structure, operator counts, connections
- **Limitation**: Can't test runtime behavior (network evaluation)

### 5.3 Recommended Approach

For your specific needs (testing scene migration, network integrity):

1. **Immediate**: Extend `avr32_sim` to be a **headless test harness**
   - Add `test_scene_load()` function that exercises `scene_read_buf()` → `net_unpickle()`
   - Add `test_network_integrity()` that validates connections after unpickling
   - Add `test_operator_creation()` that creates all 67 operators
   - Print pass/fail results to stdout

2. **Short-term**: Add mock SPI Blackfin to simulator
   - Respond to DSP load commands with "ready" signal
   - Echo parameter changes back
   - This enables testing the full `app_launch()` sequence

3. **Medium-term**: Full hardware-in-the-loop test
   - Only for final validation before release
   - Keep as occasional check, not daily workflow

---

## 6. Key Files for Deep Understanding

### Must-Read (in order)

1. `apps/bees/src/app_bees.c` — App lifecycle, boot sequence
2. `apps/bees/src/net.c` — Network initialization, add_op, connect, activate
3. `apps/bees/src/scene.c` — Scene serialization/deserialization
4. `apps/bees/src/op.c` — Operator base class, registry
5. `apps/bees/src/op_pool.c` — Memory allocation for operators
6. `apps/bees/src/render.c` — Screen rendering pipeline
7. `avr32/src/main.c` — Event loop, USB/monome handlers
8. `avr32/src/bfin.c` — SPI communication with Blackfin
9. `libavr32/src/monome.c` — Monome grid detection/communication

### Your Custom Files (understand deeply)

1. `apps/bees/src/dynamic_network.c/h` — Your expandable network system
2. `apps/bees/src/scene_convert.c/h` — Your migration framework
3. `avr32/src/monome_transport.c/h` — Your transport abstraction
4. `libavr32/src/usb/cdc/` — Your CDC driver
5. `apps/bees/src/OPERATOR_ID_MAPPING.h` — Your operator analysis
6. `apps/bees/src/OPERATOR_OUTPUT_CHANGES.h` — Your output shift analysis

---

## 7. Open Questions & Next Steps

### From Your TODO List

- [ ] **Scene Conversion Implementation** — Stub exists, needs actual pickle parsing
- [ ] **Dynamic Network Integration** — `#ifdef` exists, needs testing on hardware
- [ ] **CDC Grid Testing** — Can't test without modern grid hardware
- [ ] **Build System** — Docker issue resolved, needs testing

### Suggested Priority

1. **Implement `scene_convert_v07_to_v08()`** — Your analysis files are complete, the logic needs writing
2. **Create headless test harness** — Extend `avr32_sim` for rapid iteration
3. **Test scene migration with real scenes** — Use `beekeep` or test harness
4. **Build and flash to verify CDC fix** — One-time hardware validation

---

## 8. Quick Reference: File Counts

| Category | monome/aleph | btrailor/aleph (develop) |
|----------|-------------|--------------------------|
| bees src files | 53 C/H | 53 + 9 new (dynamic, convert) |
| operators | 60+ | 60+ (same set) |
| DSP modules | 6 | 6 + pitch_shift |
| docs | README, CHANGELOG | 30+ markdown files |
| utils | 10 | 10 + development/ |
| Total lines changed vs upstream | — | ~5,000+ (major fork) |
