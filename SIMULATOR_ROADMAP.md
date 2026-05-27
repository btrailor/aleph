# Aleph Simulator Development Roadmap

**Synthesized from three parallel analyses**  
**Date**: 2026-05-27  
**Sources**:
- `SIMULATOR_ANALYSIS_CODE.md` — code-architect (21,839 bytes)
- `SIMULATOR_ANALYSIS_BUILD.md` — devops-engineer (~24,000 bytes)
- `SIMULATOR_ANALYSIS_ALTERNATIVES.md` — Polly direct research (10,819 bytes)

---

## The Situation

The Aleph has two incomplete simulators that could eliminate the slow physical flash/test cycle. Right now:
- **avr32_sim** compiles but does nothing — event queue is dead, all peripherals are `#if 0`'d
- **bfin_sim** has no Makefile and an infinite-loop bug (`sleep(-1)`)
- **beekeep** (GTK scene editor) is the *one* thing that actually runs — it proves the compilation path works

The gap from "compiles" to "useful for testing" is **surprisingly small**.

---

## What the Simulators Are

| Simulator | Purpose | Lines | Status |
|-----------|---------|-------|--------|
| **avr32_sim** | AVR32 controller stubs for BEES | 24,645 | Compiles, dead event loop |
| **bfin_sim** | Blackfin DSP audio wrapper | 2,950 | No Makefile, JACK-dependent |
| **beekeep** | GTK scene editor (uses avr32_sim) | ~3,500 | **Actually runs** |

---

## Three Parallel Findings

### 1. Code Analysis (code-architect)

**The critical blocker**: `event_post()` and `event_next()` are `#if 0`'d in `events.c`. BEES is entirely event-driven. Until ~30 lines are uncommented (with IRQ calls replaced by no-ops), **nothing end-to-end can execute**.

**What's already working**:
- Memory allocation (`malloc`/`free` fallback)
- Software timer data structures (but `process_timers()` never called)
- Fixed-point math (complete via libfixmath)
- Debug output (`printf` to stdout)
- Monome protocol encoding (LED buffers, frame-dirty tracking — bytes just go nowhere)
- Font/region system (software screen buffer)
- beekeep GTK tool builds and runs

**Effort to minimum useful testing: 3–4 focused days**.

| Missing Piece | Effort | Key Work |
|---------------|--------|----------|
| Event queue + main loop + timer tick | **Small** (~2 hrs) | Uncomment `events.c`, activate real `main()` |
| Event injection API | **Small** (~3–4 hrs) | `inject_encoder()`, `inject_switch()`, `inject_grid_key()` etc. |
| Mock Blackfin with `.dsc` parsing | **Medium** (~1–2 days) | Replace `bfin.c` with mock that answers param queries |
| Filesystem (beekeep's `fopen` path) | **Small** (~2–4 hrs) | Use beekeep's `files.c` instead of FAT layer |
| USB sim / LED buffer inspection / MIDI log | **Small–Medium** (~4–8 hrs) | Event injection for connect/disconnect, expose `monomeLedBuffer` |
| Headless test harness with scene fixtures | **Medium** (~1 day) | Load `.scn`, run ops, assert state |

### 2. Build Infrastructure (devops-engineer)

**Key findings**:
- `beekeep` already compiles for host (x86_64/arm64) with plain `gcc` — it's **not a cross-compile**
- `bfin_sim` has **no Makefile at all** — must write one or use direct compile command
- `libfixmath` build fails on both simulators (`fix16_fract.c` can't find `fix.h`)
- `beekeep/Makefile` hardcodes Jansson path (`/opt/homebrew/Cellar/jansson/2.14.1/`) — breaks on upgrade
- `bfin_sim/main.c` has `sleep(-1)` which immediately returns on macOS — should be `while(1) pause()`
- SDL2 is already installed — ready for screen rendering if we add it
- **Headless build is the easiest path** — no GTK needed

**One-command headless build** (from devops-engineer's analysis):
```bash
gcc -std=gnu99 \
  -DBEEKEEP=1 -DARCH_AVR32=1 -DDYNAMIC_NETWORK_ENABLED=1 \
  -Icommon -Iutils/avr32_sim -Iapps/bees -Iutils/beekeep/src \
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

### 3. Alternative Approaches (Polly research)

| Approach | Viability | Effort | Verdict |
|----------|-----------|--------|---------|
| **Extend existing avr32_sim** | ✅ YES | Medium | **Best path** |
| **QEMU AVR32** (flogosec) | ⚠️ MAYBE | Large | Exists but immature; not upstreamed; UC3A0512 not modeled |
| **Renode** | ❌ NO | — | No AVR32 or Blackfin support |
| **gem5** | ❌ NO | — | No AVR32 support |
| **AVR32 dev board (EVK1105)** | ✅ YES | Small | **Same chip as Aleph** — ~$50-150 on eBay |
| **Blackfin JTAG (ICEbear)** | ⚠️ MAYBE | Medium | ~€200-300; enables DSP debugging |
| **AVR32→x86 transpilation** | ⚠️ MAYBE | Large | Same as extending sim, but more systematic |

**Notable discovery**: The **EVK1105** evaluation kit uses the exact same chip as the Aleph (AT32UC3A0512). Available on eBay. Perfect for testing BEES without risking the actual module.

---

## Recommended Phased Approach

### Phase 0: Quick Wins (This Week)

These require no code changes to BEES — just fix the build system:

1. **Fix libfixmath build** — add `-I../../` to both simulators' libfixmath Makefiles
2. **Write a Makefile for bfin_sim** — there's literally none
3. **Patch beekeep Makefile** — use `pkg-config` for Jansson instead of hardcoded path
4. **Add `HEADLESS=1` target** to beekeep Makefile
5. **Fix `sleep(-1)`** in `bfin_sim/main.c` → `while(1) pause()`
6. **Get a headless beekeep build running** — verify `app_init()` + `app_launch()` executes

**Effort**: 1 day
**Deliverable**: `make headless` works, `beekeep-headless` binary exists

---

### Phase 1: Minimum Viable Simulator (2–3 Weeks)

Bring the event loop to life and add the pieces needed for BEES scene/network testing:

1. **Resurrect event queue** (`events.c`) — uncomment, stub IRQ calls
2. **Activate main loop** (`main.c`) — run `check_events()` in `while(1)`
3. **Wire timer processing** — call `process_timers()` from event loop or `SIGALRM`
4. **Add event injection API** — `inject_encoder()`, `inject_switch()`, `inject_grid_key()`, `inject_midi_packet()`
5. **Mock Blackfin** — parse `.dsc` file at startup, answer param queries with static data
6. **Filesystem via beekeep path** — use direct `fopen` instead of FAT layer for scene I/O
7. **Headless test harness** — load `.scn`, run fixtures, assert operator state

**Effort**: 3–4 focused days of work  
**Deliverable**: Can load a BEES scene in simulator, verify network integrity, save/load round-trip

---

### Phase 2: Useful Testing (1 Month)

Add the capabilities needed for real development confidence:

1. **USB simulation layer** — inject connect/disconnect events, inspect LED buffer state
2. **Scene migration test suite** — load 0.7.1 scenes, verify remapped operators, save as 0.8.x
3. **Serial debug capture** — route `print_dbg*` to log file for post-test analysis
4. **SDL2 screen renderer** — visual output for interactive development
5. **Automated test runner** — script that loads fixtures, runs scenarios, reports pass/fail
6. **GitHub Actions CI** — headless build on every push

**Effort**: 2–3 weeks  
**Deliverable**: CI runs scene migration tests automatically; SDL window shows BEES UI

---

### Phase 3: Hardware Confidence (Parallel Track)

Physical validation layer — doesn't block simulator work:

1. **Acquire EVK1105 board** (~$50-150 on eBay) — same AT32UC3A0512 chip
2. **Flash BEES to EVK1105** — validate on real silicon
3. **Test CDC grid with actual monome hardware** — verify the community fork's CDC changes
4. **Acquire Blackfin JTAG debugger** (ICEbear or Emlink) — if DSP module work is needed

**Effort**: $100-300 + setup time  
**Deliverable**: Physical smoke-test platform that catches simulator false positives

---

### Phase 4: Long-term (If Needed)

1. **Evaluate QEMU AVR32 extension** — only if simulator reveals fundamental gaps
2. **Extend bfin_sim for audio testing** — JACK integration for DSP module validation
3. **Containerized builds** — Docker for reproducible cross-compilation

---

## File Map (New Files to Create)

```
avr32_sim/src/
  event_inject.h          # Event injection API
  event_inject.c          # Implementation
  bfin_mock.h             # Mock Blackfin state
  bfin_mock.c             # Mock Blackfin implementation
  usb_sim.h               # USB simulation layer
  usb_sim.c               # Event injection for USB, LED buffer reader
  screen_sdl.c            # SDL2 screen renderer (optional Phase 2)

test/
  fixtures/
    empty.scn             # Minimal test scene
    basic_network.scn     # Two-op network with connection
    0.7.1_scene.scn       # Legacy format for migration test
  harness.c               # Test runner main()
  test_scene_migration.c  # Migration test cases
  test_network_integrity.c # Network creation/destruction tests

scripts/
  test-sim.sh             # Local smoke test script

.github/workflows/
  sim-build.yml           # CI: headless build + test
```

---

## Risk Assessment

| Risk | Likelihood | Mitigation |
|------|------------|------------|
| Event loop resurrection harder than expected | Medium | beekeep proves the compilation path; it's ~30 lines |
| Mock Blackfin `.dsc` parsing complex | Low | `.dsc` format is simple binary struct; existing parsers in bfin_lib/ |
| macOS JACK issues for bfin_sim | Medium | Use dummy JACK driver; or skip bfin_sim initially |
| EVK1105 unavailable / expensive | Low | Multiple sellers on eBay; STK1000 also works (different chip family though) |
| QEMU AVR32 path is dead end | High | Not recommended as primary path; keep as Phase 4 option only |

---

## Bottom Line

The simulator is **not a from-scratch build**. It's a **cleanup and wiring job**. The code-architect and devops-engineer both independently arrived at the same conclusion: **3–4 focused days** gets you from "compiles and exits" to "loads scenes and verifies network state".

The biggest risk is **not technical complexity** — it's **scope creep**. The temptation to model every AVR32 peripheral perfectly, or to chase QEMU/Renode integration, would turn a 3-day job into a 3-month job. The recommended path stays disciplined: uncomment the event loop, add mocks for the specific subsystems BEES needs, validate against physical hardware periodically.

**Next action**: Phase 0 quick wins — fix the build system, get `beekeep-headless` compiling and running.
