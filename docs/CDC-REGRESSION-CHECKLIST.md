# CDC Grid Regression Checklist

**Purpose:** a hardware checklist to run against a real modern (2021+, USB-C)
monome grid before and after any change to the CDC path, so that months of
hard-won debugging are not silently undone.

**Rules:**
- Run this **before** a change (establish the baseline behavior) and **after**.
- A change is not complete until every item below matches the pre-change result.
- If an item was already failing before the change, note it — do not treat it as
  a regression, but do not let it slide either.

**Binary under test:** `apps/bees/aleph-bees.hex`
**Known-good reference binary:** `artifacts/baseline-2026-08-03/aleph-bees.hex`
(sha256 `8305bdc882cb4d6e53da155bf4ba8197a6813411c11ca5afac7f9f3e5e3a4542`)

---

## A. Enumeration — does the grid get found at all?

These are the behaviors our fork implements that **upstream monome/libavr32 does
not have**. If any of them silently regress, CDC support is gone even though
the code still compiles.

| # | Behavior | Where | Expected | How to observe |
|---|---|---|---|---|
| A1 | CDC device enumerates as a USB device | `libavr32/src/usb/cdc/uhi_cdc.c` | Grid appears as attached; no "unsupported VID, rejecting" | Debug serial output: `CDC: VID accepted (0x483)` |
| A2 | VID check accepts **`0x0483`** (2021+ STM32 grids) | `uhi_cdc.c` | Accepts `0x0483` **and** `0x16c0` (original monome) | Debug: `CDC: VID accepted (...)` |
| A3 | USB driver order is **FTDI, CDC, HID** | `conf/aleph/conf_usb_host.h` | CDC claims the grid **before** HID tries | `USB_HOST_UHI UHI_FTDI, UHI_CDC, UHI_HID, UHI_MIDI` |
| A4 | HID driver does **not** reject monome VIDs | `src/usb/hid/uhi_hid.c` | Grid is not grabbed by the HID driver | Grid enumerates as CDC, not HID |
| A5 | CDC **line coding** set: 115200 8N1 | `src/usb/cdc/cdc.c` (`uhi_cdc_open`) | Port opens; grid accepts the configuration | Grid enumerates and stays connected |
| A6 | Serial string starts with **`"cdc"`** → modern grid | `src/monome.c` `check_monome_device_desc()` | Takes the modern-grid path, returns 1 | No mext query round-trip at connect |
| A7 | CDC descriptor strings read as **ASCII** (not UTF-16) | `src/monome.c` | Manufacturer string matches `"monome"` | Grid is identified; A6 path taken |
| A8 | `cdc_setup()` → `monome_setup_mext()` → `kEventMonomeConnect` | `cdc.c` → `monome.c` → `events.h` | Connect event fires exactly once | `monomeConnect` set; timers start |
| A9 | Duplicate connect-event guard | `cdc.c` (`lastPlug`) | Interrupts do not spam connect events | One connect event, not many |
| A10 | `use_cdc = 1` set inside `monome_setup_mext()` | `src/monome.c` (~line 416) | Transport routes to `cdc_*`, not `ftdi_*` | LEDs/keys work over CDC |
| A11 | Shared `setup_mext()` path does **NOT** set `use_cdc` | `src/monome.c` (~line 676) | Comment: `// use_cdc = 1; // REMOVED: setup_mext is used for both FTDI and CDC grids`. This is deliberate — an FTDI grid reaching the shared mext path must stay on the FTDI transport. **Do not "fix" this by re-adding the line.** | FTDI grid still routes to FTDI |

## B. Transport — is data actually flowing?

| # | Behavior | Expected | How to observe |
|---|---|---|---|
| B1 | `monome_transport_read()` polls the CDC path | Grid data received | Keys register when pressed |
| B2 | `monome_transport_write()` sends LEDs over CDC | Grid LEDs light | LEDs illuminate |
| B3 | FTDI path still works (old grid) | FTDI grid unaffected | **NO FTDI HARDWARE AVAILABLE** - verify by code review against the FTDI items in section G below |
| B4 | FTDI RX callback does **not** fire for CDC devices | No cross-talk | `ftdi.c` guards on `ftdiConnect` |

### FTDI preservation (no hardware - code review only)

**Constraint (Brett, 2026-10-30):** no classic FTDI grid is available for
testing, but FTDI support **must not regress** - other Aleph users still use
those grids. The FTDI path must therefore be preserved **by construction**:
port our implementation rather than replacing it with upstream's CDC-only
path.

| # | Behavior | Where | Must remain | Verified by |
|---|---|---|---|---|
| F1 | FTDI transport reachable via `use_cdc == 0` dispatch | `src/monome.c:14-29` | `monome_transport_write` -> `ftdi_write`, `monome_transport_read` -> `ftdi_read`, `monome_transport_tx_busy` -> `ftdi_tx_busy`, `monome_transport_rx_buf` -> `ftdi_rx_buf` | code review |
| F1b | Protocol-level read/write still dispatched by protocol | `src/monome.c:167-171, 556` | `readSerialFuncs[]` = 40h / series / mext, assigned in `set_funcs()` | code review |
| F2 | `ftdi_setup()` still reachable from `handler_FtdiConnect()` | `aleph avr32/src/main.c` | FTDI connect path intact | code review |
| F3 | FTDI driver still in `USB_HOST_UHI` **and ordered before CDC** | `conf/aleph/conf_usb_host.h` | `UHI_FTDI, UHI_CDC, UHI_HID, UHI_MIDI` | code review |
| F4 | FTDI USB **string descriptors parsed as UTF-16** | `src/monome.c` (`isUnicode`) | `mstr[i*2]` for FTDI, `mstr[i]` for CDC | code review |
| F5 | FTDI RX callback guard intact | `src/usb/ftdi/ftdi.c:54` | guarded on `ftdiConnect` | code review |
| F6 | FTDI-only device types still detected: `a40h`, `m64-`, `m128-`, `m256-` | `src/monome.c` `check_monome_device_desc()` | `setup_40h` / `setup_series` paths | code review |
| F7 | `use_cdc` NOT set on the shared `setup_mext()` path | `src/monome.c` (~line 676) | Comment: `// use_cdc = 1; // REMOVED: setup_mext is used for both FTDI and CDC grids`. An FTDI grid reaching the shared mext path must stay on FTDI. **Do not "fix" this.** | code review |
| F8 | `monome_transport_*` helpers dispatch to `ftdi_*` when `use_cdc == 0` | `src/monome.c:13-29` | dual transport preserved | code review |

**Note on our fork's mechanism:** our fork does **not** use upstream's
`serial_read = &ftdi_read` function-pointer style. It uses a `use_cdc` flag
with inline dispatch in `monome_transport_*` helpers (`src/monome.c:13-29`),
plus protocol-level dispatch via `readSerialFuncs[]` (`src/monome.c:167-171`).
**This is a deliberate difference from upstream and must be preserved.**

**F7 is the highest-risk item.** Upstream's design points `setup_mext` at CDC
function pointers unconditionally, which would silently break any FTDI grid
that reaches the shared mext path. Porting upstream's `monome.c` wholesale
would reintroduce this. The transport abstraction must be carried over intact.

## C. Grid operator behavior

| # | Behavior | Expected | How to observe |
|---|---|---|---|
| C1 | Grid op exists in the network | `MONOME_GRID_CLASSIC` present | OPS page shows the grid op |
| C2 | Grid op has **focus** | `monomeOpFocus != NULL` | Key events reach the op, not `dummyHandler` |
| C3 | Key events route to the op | Pressing a grid key changes a value | Visible in OUTS/INS page |
| C4 | LEDs light on the physical grid | Grid illuminates | Visual |
| C5 | Grid op gains **focus** when loaded at runtime (not during scene recall) | `apps/bees/src/ops/op_step.c:105`, `op_life_classic.c:106`, `op_monome_grid_raw.c:89` | `if(!recallingScene) { net_monome_set_focus(&(op->monome), 1); }` | Grid op has focus after load |
| C5a | **Focus is NOT grabbed during scene recall** (the `recallingScene` guard) | same sites | The `if(!recallingScene)` guard is present at all three sites | Correct focus ownership after load |
| C5b | Focus is released on op deinit | `net_monome_set_focus(..., 0)` at each deinit | Old op releases focus | No stale focus |
| C6 | Grid op outputs unconnected is *correct* 0.7.1 behavior | Not a regression | See `grid-cdc-investigation.md` §2.4 |

## D. Scene interaction

| # | Behavior | Expected | How to observe |
|---|---|---|---|
| D1 | `stepwaves.scn` loads; grid step sequencer runs | ✅ known-good | Audio + grid |
| D2 | `life.scn` loads and functions | ✅ known-good | Visual |
| D3 | Second-scene load does not lock up | No freeze; no reflash needed | Load one scene, then another |
| D4 | Grid works after a scene load | Connect event re-fires | Grid responsive post-load |

## E. Build integrity

| # | Check | Expected |
|---|---|---|
| E1 | Clean build succeeds | no errors |
| E2 | `net.c` capacity check present | `net_add_param()` calls `dynamic_network_expand_params()` |
| E3 | `INITIAL_PARAMS_SIZE` = 128 in `dynamic_network.c` | 128 (header still says 64 — known macro conflict) |
| E4 | No NEW compiler warnings vs baseline | 42 warnings expected; any new one is a red flag |
| E5 | `.text`/`.data`/`.bss` vs baseline | `0x299d8 / 0x1c04 / 0x4031c5c` — a delta means a real code change |
| E6 | Reproducible build | Two clean builds → identical sha256 |

---

## Known-suspect areas (check these FIRST if something breaks)

1. **Grid size query** — our fork **deliberately skips** it (commit `c700eba`,
   "assume 128 grid, skip problematic query"). Upstream *does* query
   (commit `d173d0e`). The reason for skipping is **unverified**. If adopting
   upstream's version, this is the first thing to regress. Grid must still
   report 16×8 (128).
2. **ASCII vs UTF-16 string handling** (commit `60b60b1`) — CDC descriptors are
   plain ASCII; FTDI descriptors are UTF-16. Getting this wrong makes the grid
   invisible (fails A6/A7).
3. **Stale header** `avr32/src/usb/cdc/cdc.h` in the **Aleph repo** — it shadows
   libavr32's `cdc.h` (aleph's `avr32/src` precedes `$(LIB_AVR32)src` on the
   `-I` path) and mis-declares `cdc_write`/`cdc_read` signatures. Currently
   benign; a live trap.
4. **Driver order** — HID before CDC means the grid gets grabbed by the wrong
   driver (fails A3/A4).
5. **`cdc_setup()` / `cdc_disconnect()`** — upstream does not declare these;
   Aleph's `avr32/src/main.c` calls both. Any upstream migration must provide
   equivalents.
6. **Focus acquisition** — focus is grabbed by the grid-native operators
   (`op_step`, `op_life`, `op_life_classic`, `op_monome_grid_raw`,
   `op_monome_arc`) via `net_monome_set_focus()`, guarded by `!recallingScene`.
   It is **not** set by the connect path and **not** set by `scene.c`. Older
   project docs describe a "Bug 9 fix" that added a default grid op in
   `scene.c` when `monomeOpFocus == NULL` — **that code is not in the current
   tree** (verified: no `eOpMonomeGridClassic` reference in `apps/bees/src/scene.c`,
   and `git log --all -S "eOpMonomeGridClassic" -- apps/bees/src/scene.c` returns
   nothing). The current mechanism relies on the scene's grid-native operator
   grabbing focus. If grid focus is broken after a scene load, check this
   mechanism first — and note that a scene with **no** grid-native operator will
   have no focused grid controller at all.

## Record of results

Copy this table into your session notes when running the checklist.

| Item | Pre-change | Post-change | Notes |
|---|---|---|---|
| A1 | | | |
| A2 | | | |
| A3 | | | |
| A4 | | | |
| A5 | | | |
| A6 | | | |
| A7 | | | |
| A8 | | | |
| A9 | | | |
| A10 | | | |
| A11 | | | |
| B1 | | | |
| B2 | | | |
| B3 | | | |
| B4 | | | |
| C1 | | | |
| C2 | | | |
| C3 | | | |
| C4 | | | |
| C5 | | | |
| C5a | | | |
| C5b | | | |
| C6 | | | |
| D1 | | | |
| D2 | | | |
| D3 | | | |
| D4 | | | |
| E1 | | | |
| E2 | | | |
| E3 | | | |
| E4 | | | |
| E5 | | | |
| E6 | | | |

**Build sha256 (pre):**
**Build sha256 (post):**
**Tester / date:**
