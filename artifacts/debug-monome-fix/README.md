# Debug-monome fix build — 2026-10-30

Candidate build that removes the blocking `print_dbg` calls from the grid
key-event path. **Needs hardware verification.**

## The binary

| File | Size | SHA-256 |
|---|---|---|
| `aleph-bees.hex` | 498,810 B | `3bed1f2dec007b49f970f81af9b863f26db4f0927882effa24f5591dbffc1fba` |
| `aleph-bees.elf` | 295,882 B | (symbols/debug info) |

| Property | Value |
|---|---|
| Firmware version | BEES 0.8.3 |
| Repo commit | `d80c1798` "perf: gate hot-path print_dbg behind DEBUG_MONOME (default 0)" |
| Submodule pin | libavr32 `162b653c1462ec50e7132ad59f18d39c40bfa2a2` (unchanged) |
| Build | `make` (DEBUG_MONOME=0, the default) |
| Section sizes | `.text` 0x29898, `.data` 0x1c04, `.bss` 0x4031c58 |

## What changed

Only three files, 31 inserted lines, **zero deleted lines**:

| File | Change |
|---|---|
| `apps/bees/config.mk` | Added `DEBUG_MONOME` flag (default 0) |
| `apps/bees/src/handler.c` | Gated 7 `print_dbg` calls in `handle_MonomeGridKey()` |
| `apps/bees/src/ops/op_cascades.c` | Gated 2 `print_dbg` calls in the cascade trigger path |

**No CDC-related source was modified. The libavr32 submodule was not touched.**

## Why

`print_dbg()` is a blocking UART write:
`print()` -> `usart_write_line()` -> `usart_putchar()` -> busy-wait loop on
`usart_write_char()`. At `BAUD=115200` with 8N1, each byte costs ~11.6 us.

`handle_MonomeGridKey()` emitted ~131 bytes across 7 calls for **every grid key
event** = **~11.4 ms of blocking UART write per key press**. Events are
consumed by `check_events()` (`avr32/src/main.c:367`) on the main loop - the
same loop that drives the AVR32<->Blackfin SPI transport to the DSP.

A fast gesture (8-key slide) therefore stalled the main loop ~91 ms, starving
the SPI transport and causing audible audio dropouts. A slow isolated press
produced a single 11 ms stall - inaudible. That matches the reported symptom:
dropouts only during certain gestures.

## How to build it

```bash
cd ~/aleph
export PATH="/Applications/Docker.app/Contents/Resources/bin:$PATH"
docker run --rm --platform=linux/amd64 -v "$HOME/aleph:/host" -w /tmp aleph-builder:latest bash -c '
  cp -r /host /tmp/aleph && cd /tmp/aleph/apps/bees && \
  export PATH=/root/avr32-toolchain-linux/bin:/opt/uClinux/bfin-elf/bin:$PATH && \
  make clean && make && sha256sum aleph-bees.hex'
```

To re-enable the debug output (for diagnosing the grid):
`make DEBUG_MONOME=1` - this reproduces the baseline binary **exactly**
(sha256 `8305bdc8...a4542`), which proves the gating is a no-op when the debug
output is enabled and that this change is the sole difference from baseline.

## Flash instructions

1. Insert the SD card into the Mac.
2. Copy this hex to the card:
   ```bash
   cp ~/aleph/artifacts/debug-monome-fix/aleph-bees.hex /Volumes/ALEPH/app/aleph-bees.hex
   ```
   Keep the baseline available as a fallback - it is preserved in git at
   `artifacts/baseline-2026-08-03/aleph-bees.hex`.
3. Eject the card properly (Disk Utility, or Cmd+Q in Finder - do not yank it).
4. Hold **MODE** while powering on the Aleph to flash from `app/aleph-bees.hex`.

## What to test

| Test | Expected |
|---|---|
| Grid enumerates and LEDs light | ✅ unchanged - CDC path untouched |
| Grid keys register | ✅ unchanged |
| `stepwaves.scn` loads and plays | ✅ unchanged |
| **Fast grid gestures - do dropouts still occur?** | 🎯 **should be gone** |
| Slow isolated presses | ✅ unchanged |
| Cascade scene (`mpdrumssss`) | ⚠️ **still broken** - that is a separate scene-conversion bug, not fixed here |

## If you need to roll back

```bash
cp ~/aleph/artifacts/baseline-2026-08-03/aleph-bees.hex /Volumes/ALEPH/app/aleph-bees.hex
```
