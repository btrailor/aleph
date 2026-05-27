# Aleph AVR32 Simulator Analysis
## Extending `utils/avr32_sim` into a Useful Firmware Testing Tool

**Date:** 2026-05-27  
**Scope:** `utils/avr32_sim/` — 97 files, ~24,645 total lines  
**Target firmware:** BEES (`apps/bees/`)  
**Related simulator:** `utils/beekeep/` — GTK-based scene editor (partial predecessor)

---

## 1. Current Simulator Capabilities

### What the simulator is

The `avr32_sim` directory is a **hardware-abstraction stub layer** — not a running simulator. It provides C source files that replace AVR32-specific ASF (Atmel Software Framework) peripherals with either no-ops or host (x86/ARM macOS) equivalents. The goal is that BEES source code can be compiled and linked against these stubs for non-hardware execution.

The stubs are used in two known contexts:
- **`utils/beekeep/`** — a GTK scene editor that compiles BEES ops and network logic against these stubs to load/save/inspect `.scn` files. It does run, builds with `gcc`, and links against GTK3 + jansson.
- **`utils/pd/`** — a PureData external wrapper (similar purpose).

### File inventory

| File | Lines | Purpose | Status |
|------|-------|---------|--------|
| `src/main.c` | 419 | Entry point; has two `main()` impls gated by `#if 1` | Only trivial path compiles: `app_init(); app_launch(0);` |
| `src/bfin.c` | 441 | SPI Blackfin communication stubs | All real logic `#if 0`'d out; `bfin_get_num_params()` returns 0, `bfin_get_param()` returns 0, all set/load/desc calls are no-ops |
| `src/events.c` | 134 | Circular event queue | All logic `#if 0`'d; `event_next()` always returns 0, `event_post()` always returns 0 |
| `src/filesystem.c` | 138 | SD card FAT I/O glue | `fat_init()` returns 0; `media_read/write` return 1 without doing anything |
| `src/screen.c` | 303 | OLED screen driver | All hardware SPI calls `#if 0`'d; no SDL or framebuffer output |
| `src/serial.c` | 261 | USART serial protocol (host↔device params) | All logic `#if 0`'d |
| `src/init.c` | 319 | Hardware peripheral initialization | All functions are no-ops (`#if 1 {} #else ... #endif`) |
| `src/interrupts.c` | 336 | ISR handlers + registration | All handlers no-ops; `register_interrupts()` no-op |
| `src/timers.c` | 230 | Software timer linked list | `init_timers()` and `timer_add/remove` are **fully implemented** — actual data structure in host RAM; `process_timers()` exists but is never called |
| `src/encoders.c` | 78 | Rotary encoder processing | `init_encoders()` no-op; `process_enc()` no-op |
| `src/switches.c` | 52 | Switch state + event posting | Partially implemented |
| `src/adc.c` | 142 | 4-channel ADC polling | `adc_convert()` `#if 0`'d; `adc_poll()` no-op |
| `src/memory.c` | 141 | Custom heap allocator | **Implemented**: falls through to `malloc()`/`free()`; `alloc_mem()` is live |
| `src/flash.c` | 176 | NVRAM/flash storage | `init_flash()` returns 1 (firstrun); `flash_nvram_data` is a static struct in BSS |
| `src/monome.c` | 957 | Monome grid/arc protocol | Protocol logic (frame dirty, LED buffers, TX encoding) is **substantially implemented**; reads/writes against `ftdi_*` which is stubbed |
| `src/usb/ftdi/ftdi.c` | 176 | FTDI USB class driver | Stub; `ftdi_write()` and `ftdi_rx_buf()` not wired to anything real |
| `src/usb/midi/midi.c` | 270 | MIDI USB class driver | Stub |
| `src/usb/hid/hid.c` | 109 | HID USB class driver | Stub |
| `src/print_funcs.c` | 39 | Debug printf | **Fully implemented**: wraps `printf()`/`fflush(stdout)` |
| `src/font.c` / `src/region.c` | 499 / 202 | Font rendering / dirty regions | Implemented for software screen buffer |
| `src/fix.c` + `libfixmath/` | 210 + ~1100 | Fixed-point math | **Fully implemented** |
| `src/delay.c` | 5 | Timing delays | No-op |
| `src/i2c.c` | 160 | TWI/I2C | No-op |
| `avr32_sim.mk` | 30 | Build fragment | Defines `src` list for inclusion in `beekeep` Makefile |

### What actually works today

1. **Memory allocation** (`alloc_mem`/`free_mem`) — falls through to `malloc`/`free`.
2. **Software timer data structure** — `timer_add`/`timer_remove`/`timer_already_linked` are live, but `process_timers()` is never called from any loop.
3. **Fixed-point math** — complete, tested via libfixmath.
4. **Debug output** — `print_dbg*` writes to stdout via `printf`.
5. **Monome protocol encoding** — LED buffer management, frame-dirty tracking, and wire-format packing (40h/series/mext) are implemented. The bytes just never go anywhere.
6. **Font/region system** — software screen buffer operations exist.
7. **The beekeep GTK tool builds and runs** — it executes `app_init()` + `app_launch(1)`, loads `.scn` files, and lets you inspect/edit BEES network topology. This is the one validated execution path.

### What does not work (stubs returning nothing)

- Event queue is dead: `event_post()` returns 0, `event_next()` returns 0. No events flow.
- Bfin is dead: `bfin_get_num_params()` returns 0, all param queries return 0, load is no-op.
- SD card is dead: `fat_init()` returns 0 (success) but no FAT library is attached; file I/O silently does nothing.
- Timer loop is dead: nothing calls `process_timers()`.
- Encoder and switch events never post.
- Screen never renders to any output.
- USB/FTDI/MIDI/HID are stubs with no wiring.

---

## 2. Missing Pieces — Analysis and Approach

### 2.1 The Foundational Problem: Dead Event Loop

Before any of the specific subsystem work matters, **the event queue must be brought to life**. Currently `event_post()` and `event_next()` are both `#if 0`'d skeletons. BEES is entirely event-driven — without a working queue, none of the app logic executes at all.

**Fix:** Uncomment the queue logic in `events.c`. The original code is all there, just gated. The only thing to change is removing the IRQ disable/enable calls (`cpu_irq_disable_level` / `cpu_irq_enable_level`) which don't exist on the host — replace with no-ops or a mutex if multithreaded. This is a 30-line change.

Similarly, `process_timers()` in `timers.c` must be called from the main loop, and the loop in `main.c` must actually run: change the trivial `main()` (currently just `app_init(); app_launch(0); return 0;`) to match the fuller version (currently `#if 0`'d) which calls `check_events()` in a `while(1)` loop.

**Estimated effort: Small** (~2 hours, no new files)

Files to modify:
- `src/events.c` — uncomment queue logic, remove/stub IRQ calls
- `src/main.c` — activate the real `main()` block
- `src/timers.c` — ensure `process_timers()` is called from event loop or a `SIGALRM` handler

---

### 2.2 Mock SPI Blackfin

**What it needs to do:**  
When BEES calls `bfin_load_buf()`, it's loading a DSP `.ldr` file. After that, it calls `bfin_get_num_params()`, `bfin_get_param_desc()`, `bfin_get_module_name()`, and eventually `bfin_set_param()` per parameter change. For testing purposes, you don't need a real Blackfin — you need a mock that answers these queries with plausible data.

The SPI protocol is defined in `common/protocol.h`. The command bytes are: `MSG_GET_NUM_PARAMS_COM (2)`, `MSG_GET_PARAM_DESC_COM (3)`, `MSG_GET_MODULE_NAME_COM (4)`, `MSG_SET_PARAM_COM (0)`, `MSG_GET_PARAM_COM (1)`.

**Approach:**  
Instead of simulating the byte-level SPI transaction, implement the `bfin_*` API functions directly with a mock module descriptor. The `.dsc` file format (module descriptor) is the natural source of truth — parse it at startup to populate a mock state table.

The mock needs:
- `static u32 mock_num_params`
- `static ParamDesc mock_params[MAX_PARAMS]`
- `static fix16_t mock_param_values[MAX_PARAMS]`
- `bfin_load_buf()` — parse the paired `.dsc` file (named from `bfinLdrString`) to populate descriptors
- `bfin_get_num_params()` — return `mock_num_params`
- `bfin_get_param_desc()` — copy from `mock_params[idx]`
- `bfin_get_module_name()` — copy mock module name
- `bfin_set_param()` — store to `mock_param_values[idx]`; optionally log
- `bfin_get_param()` — return `mock_param_values[idx]`

The `.dsc` parser already exists in the codebase (`bfin_lib/` or modules). You'll need to find or re-implement a minimal reader for the binary descriptor format.

**Estimated effort: Medium** (~1–2 days)

Files to create:
- `src/bfin_mock.c` — replaces `bfin.c` or conditionally compiled via `MOCK_BFIN=1`
- `src/bfin_mock.h` — mock state accessors for test inspection

Files to modify:
- `src/bfin.c` — add `#ifdef MOCK_BFIN` branching, or replace with `bfin_mock.c` in build
- `avr32_sim.mk` — swap source file under `MOCK_BFIN`

---

### 2.3 SD Card / Filesystem Simulation

**What it needs to do:**  
BEES uses the SD card for scene files (`.scn`), module loaders (`.ldr`), and module descriptors (`.dsc`). The `files.c` in BEES calls into the FAT layer via `fl_*` functions (fat_io_lib). Currently `fat_init()` returns success but never attaches `media_read`/`media_write` to any real backing store.

**Approach:**  
The cleanest path: implement `media_read` and `media_write` in `filesystem.c` to read from and write to a flat file image (`sdcard.img`) on the host filesystem, using the sector-based interface that fat_io_lib expects. The `fl_attach_media()` call in `fat_init()` already expects `fn_diskio_read` and `fn_diskio_write` function pointers — just provide real implementations backed by `fseek`/`fread`/`fwrite` on an image file.

Simpler alternative for scene I/O only: bypass fat_io_lib entirely. The beekeep tool already does this — `utils/beekeep/src/files.c` reimplements `files_load_scene_name()` and `files_save_scene_name()` to open host files directly with `fopen`. The same pattern can be applied here: compile in beekeep's `files.c` instead of BEES's `files.c` under `BEEKEEP=1` (this is already the pattern in `beekeep/Makefile`).

For the test harness use case, the direct `fopen` path is adequate and much simpler. The full sector-image path is only needed if you're testing FAT-layer behavior specifically.

**Estimated effort: Small** (direct fopen approach, ~4 hours) or **Medium** (sector image, ~1 day)

Files to create:
- `test/sdcard/` — directory of test scene/module files
- `src/filesystem_host.c` — host-backed media_read/write (if sector image path)

Files to modify:
- `src/filesystem.c` — activate `fat_init()` with real `fl_attach_media()` call
- Build system — point to `utils/beekeep/src/files.c` or the new host filesystem

---

### 2.4 USB Host Simulation (Monome Grid + MIDI)

**What it needs to do:**  
Simulate device connect/disconnect events and input messages (grid key presses, MIDI note/CC packets) that drive BEES operators. The USB class drivers (`uhi_ftdi.c`, `uhi_midi.c`, `uhi_hid.c`) are all stubs.

**Approach:**  
Don't simulate USB at all. The correct insertion point is one level up: post events directly into the event queue. The events already exist:

- `kEventFtdiConnect` / `kEventMonomeConnect` — triggers device setup
- `kEventMonomeGridKey` — grid key with `(x | (y << 8) | (z << 16))` packed in `data`
- `kEventMidiConnect` / `kEventMidiPacket` — MIDI messages

For a test harness, implement an **event injection API** (see §2.5 below) and use it to synthesize these events. For interactive testing, read from a Unix socket or named pipe: a script sends `{ "type": "grid_key", "x": 3, "y": 2, "z": 1 }` and the simulator posts the corresponding event.

For the monome specifically, the real output (LED state) is already tracked in `monomeLedBuffer` in `monome.c`. After refresh, that buffer holds the current LED state. To observe it from tests, expose a read accessor.

MIDI output (sent from BEES to a connected device) goes through `midi.c` → `uhi_midi.c`. Stub `uhi_midi_write()` to log to a buffer that tests can inspect.

**Estimated effort: Small** for event injection; **Medium** for full bidirectional socket interface

Files to create:
- `src/usb_sim.c` — inject connect events, expose LED buffer reader, log outbound MIDI
- `src/usb_sim.h` — API for test harness

Files to modify:
- `src/usb/ftdi/ftdi.c` — `ftdi_write()` logs outbound data instead of discarding
- `src/usb/midi/midi.c` — outbound MIDI buffering for test inspection

---

### 2.5 Event Injection

**What it needs to do:**  
Allow test scripts to simulate: encoder turns (with direction and magnitude), button presses (8 switches + mode + power + footswitch), ADC value changes, grid key events, MIDI packets.

**Approach:**  
The event queue already has the right structure (`event_t { etype type; s32 data }`). Event injection is simply a wrapper that calls `event_post()` with the right type and data encoding.

The encoders on hardware post `kEventEncoder0..3` with a signed delta in `data`. Switches post `kEventSwitch0..7` with 0 (up) or 1 (down). The data format for each is established in `interrupts.c` (the `#if 0` sections show the hardware path).

Implement a simple injection API:

```c
// src/event_inject.h
void inject_encoder(u8 idx, s32 delta);
void inject_switch(u8 idx, u8 state);
void inject_adc(u8 ch, u16 value);
void inject_grid_key(u8 x, u8 y, u8 z);
void inject_midi_packet(u8 b0, u8 b1, u8 b2, u8 b3);
void inject_monome_connect(void);
void inject_midi_connect(void);
```

Each function packs `data` per the hardware convention and calls `event_post()`.

For batch/scripted testing, add a simple text-protocol reader: read lines from stdin or a file like `"enc 0 +1\nswitch 2 press\ngrid 3 4 down\n"` and convert to event posts.

**Estimated effort: Small** (~3–4 hours)

Files to create:
- `src/event_inject.c`
- `src/event_inject.h`
- `test/inject_script.c` — optional text protocol reader

---

### 2.6 Serial Debug Output Capture

**What it needs to do:**  
`print_dbg*` is already wired to `printf`. That's the serial debug path. No additional work needed for basic capture — just redirect stdout.

The host-to-device serial protocol (for param get/set over FTDI USART) is in `serial.c`. All of `serial_process()`, `serial_send_*()`, and the command handlers are `#if 0`'d. For testing the serial param interface, uncomment these and pipe them to a host buffer or socket rather than a USART register.

The structured serial protocol uses:
- ESC (27) as escape character
- US (31) as separator
- NULL (0) as end-of-packet
- Command index as first byte

For test purposes, replace `usart_putchar(DBG_USART, ...)` with `fwrite()` to a buffer, and replace `usart_read_char(FTDI_USART, ...)` with reads from a test input buffer. The rest of the framing logic can run as-is.

**Estimated effort: Small** (~2–3 hours to activate and redirect)

Files to modify:
- `src/serial.c` — replace `usart_*` calls with buffer I/O under `#ifdef SIMULATION`
- `src/serial_host.c` (new) — stdin/stdout wiring for host serial protocol

---

## 3. Headless Test Harness Design

### Architecture

The goal is to run BEES logic without SDL, GTK, or any display, in a way that can be driven by a script and produce verifiable output.

```
test_runner.c
    |
    +-- event_inject.c      (stimulus: encode turns, presses, grid keys, MIDI)
    |
    +-- [bees app logic]    (net, ops, scene, preset, handler)
    |
    +-- bfin_mock.c         (param state; logs set_param calls)
    +-- usb_sim.c           (LED buffer reader; MIDI output log)
    +-- filesystem_host.c   (fopen-backed scene I/O)
    +-- events.c            (real queue, no IRQ gating)
    +-- timers.c            (tick driven by test runner, not real time)
```

### Main loop for headless tests

```c
// test_runner.c sketch
void sim_tick(u32 n_ticks) {
    for (u32 i = 0; i < n_ticks; i++) {
        tcTicks++;
        process_timers();      // drive software timers
        check_events();        // drain event queue
    }
}

int main(void) {
    app_init();
    app_launch(1);             // firstrun=1 to skip flash checks

    // inject device connections
    inject_monome_connect();
    sim_tick(10);

    // load a scene
    files_load_scene_name("test/scenes/basic.scn");
    sim_tick(5);

    // simulate an encoder turn
    inject_encoder(0, +1);
    sim_tick(5);

    // assert: check bfin mock received expected param set
    assert(bfin_mock_get_last_set_param_idx() == EXPECTED_IDX);
    assert(bfin_mock_get_param_value(EXPECTED_IDX) == EXPECTED_VAL);

    // assert: check LED buffer state
    assert(usb_sim_get_led(3, 2) == EXPECTED_BRIGHTNESS);

    return 0;
}
```

### Key design decisions

**Time is manual.** Don't use `SIGALRM` or threads for timer ticks. Drive `tcTicks` and `process_timers()` explicitly from the test loop. This makes tests deterministic and fast — no real-time waiting.

**No SDL required.** The screen buffer lives in RAM. For test assertions, you only care about what BEES's render layer writes to the region buffers, not pixels on a display. The region system (`region.c`) and font system are already host-compatible.

**Scene files as test fixtures.** Put `.scn` files in `test/scenes/`. Each test loads a scene, injects stimulus, and checks output state. This exactly mirrors real usage.

**Log-based assertions for bfin.** The mock `bfin_set_param()` should append to a `set_param_log[]` array. Tests walk the log. Same pattern for MIDI output.

### Build target

Add a `sim` target to the beekeep Makefile (or a new `test/Makefile`):

```makefile
cflags += -D SIMULATION=1 -D BEEKEEP=1 -D MOCK_BFIN=1
src += test/test_runner.c src/event_inject.c src/bfin_mock.c src/usb_sim.c
```

No GTK dependency. No SDL. Compile with `gcc`, run with `./sim_test`.

---

## 4. Code Architecture Recommendations

### What's already right

The `#if 1 ... #else (hardware) ... #endif` pattern is unambiguous. You know exactly what's stubbed. The factoring into per-peripheral files (bfin, events, timers, monome, filesystem) is correct. The event queue design (type + s32 data, circular buffer) is clean.

### What needs fixing before you build on it

**1. The event queue must be the first thing you fix.**  
Every other subsystem depends on it. Right now you cannot test anything end-to-end because events never flow. This is the critical path blocker.

**2. Don't mix hardware-path restoration with simulation extension.**  
When you activate the event queue, use `#ifdef SIMULATION` (or just remove the `#if 0` gates entirely and replace `cpu_irq_disable_level` calls with no-ops). Don't try to make the same code compile for both AVR32 and host by adding more nested `#if` guards — that path is already broken and unmaintainable. The simulation is a separate build target; own that separation cleanly.

**3. Keep the mock Blackfin behind a thin API boundary.**  
`bfin.h` defines 8 functions. That's the interface. Do not let test code reach into `bfin_mock_state` directly — expose typed accessors. This way you can swap in a more sophisticated mock (e.g., one that simulates parameter range clamping) without touching test code.

**4. The timer tick driver belongs in the test harness, not in a timer ISR.**  
The existing `irq_tc()` ISR (all `#if 0`'d) increments `tcTicks` and calls `process_timers()`. In simulation, your test loop does this. Never use `SIGALRM` for this — it introduces nondeterminism that will cause you pain in CI.

**5. Filesystem: use beekeep's files.c as the model.**  
`utils/beekeep/src/files.c` already does what you need — it reimplements BEES's scene file I/O using `fopen` on host paths. The pattern is right. The `BEEKEEP=1` define already gates this. Lean on it instead of trying to simulate FAT sectors.

**6. Don't simulate USB below the event layer.**  
Simulating FTDI byte framing, HID reports, and MIDI packet delivery is unnecessary complexity. Post events at the semantic layer: `kEventMonomeGridKey` with packed x/y/z data. The wire format encoders in `monome.c` are real code that belongs in the sim because they produce the LED TX data that tests need to verify. The RX side (parsing incoming grid key bytes) can be driven directly by `inject_grid_key()`.

### Recommended implementation order

1. **Fix event queue** (`events.c`) — ~30 min
2. **Fix main loop** (`main.c`) — ~30 min  
3. **Fix timer tick** (call `process_timers()` from main loop) — ~15 min
4. **Implement event injection** (`event_inject.c`) — ~3 hours
5. **Implement mock bfin** (`bfin_mock.c`) — ~1 day
6. **Wire filesystem** (activate beekeep's `files.c` path) — ~2 hours
7. **Write first end-to-end test** (load scene → encoder → assert param set) — ~2 hours
8. **Add LED buffer inspection** to monome/usb_sim — ~3 hours
9. **Add MIDI output capture** — ~2 hours

Total to minimum useful test harness: **~3–4 days of focused work.**

---

## 5. Summary Table

| Feature | Effort | Approach | Key Files |
|---------|--------|----------|-----------|
| Event queue activation | Small | Uncomment `events.c`; remove IRQ gating | `events.c`, `main.c`, `timers.c` |
| Event injection API | Small | Wrap `event_post()` with typed helpers | `event_inject.c` (new), `event_inject.h` (new) |
| Mock SPI Blackfin | Medium | Implement `bfin_*` API with module descriptor state; parse `.dsc` at load | `bfin_mock.c` (new), `bfin.c` (modify) |
| SD card / filesystem | Small | Use beekeep's `files.c` with `fopen`; activate `BEEKEEP=1` path | `filesystem.c`, `beekeep/src/files.c` |
| USB host / monome sim | Small–Medium | Post events at semantic layer; expose LED buffer; log MIDI TX | `usb_sim.c` (new), `ftdi.c`, `midi.c` |
| Serial debug capture | Small | `print_dbg` already works; activate serial protocol under `SIMULATION` | `serial.c`, `serial_host.c` (new) |
| Headless test harness | Medium | Manual tick driver; assertion helpers; scene fixtures | `test/test_runner.c` (new), `test/Makefile` (new) |

The beekeep tool proves the compilation path is viable. The gap between beekeep and a test harness is primarily: a working event queue, a mock bfin with observable state, and a main loop that drives time manually. None of this requires new architectural insight — it requires uncommenting and wiring what's already there.
