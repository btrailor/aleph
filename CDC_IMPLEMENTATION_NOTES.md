# CDC Grid Implementation Notes — Aleph Bees

## Overview

This document describes the CDC (USB Communication Device Class) grid implementation for the Monome Aleph, added to support modern Monome grids (Arc, Grid 128/256) that use USB CDC serial instead of FTDI.

## Changes Summary

### Bug 1: Boot Detection Fix

**Problem**: Grid had to be plugged in AFTER boot to be detected. If plugged in before power-on, enumeration completed before app event handlers were assigned, and the connect event was wiped by `init_events()`.

**Solution**: Added `cdcPlugged` flag set in `cdc_change()` at enumeration time (before event queue wipe). `check_startup()` now checks `cdc_was_plugged()` instead of the event-derived `cdcConnect` flag.

**Files**: `cdc.c`, `cdc.h`, `main.c`

### Bug 2: Grid Input / Scene Lag Fix

**Problem**: Two overlapping issues:
1. `monome_read_serial()` was called from BOTH `cdc_rx_done()` interrupt callback AND `monome_poll_timer_callback()`, causing duplicate key events
2. `uhi_cdc_read()` returned status `0x07` (NOTRESPONDING) every transfer, creating a tight error loop that starved the system

**Solution**:
1. Removed `monome_read_serial()` from timer callback — only the interrupt callback processes data
2. Added `rxBytes = 0` after processing to prevent stale re-reads
3. Simplified error handling to match monome's proven pattern: ignore status codes, unconditionally clear `rxBusy`, let 20ms timer naturally retry

**Files**: `app_timers.c`, `cdc.c`

### Code Cleanup

1. Removed orphaned debug prints from `cdc.c` and `monome.c`
2. Commented out unused `uhi_cdc_get_nb_received()` / `uhi_cdc_read_buf()` declarations in `uhi_cdc.h`
3. Fixed recursive transport wrappers in `monome.c` (`ftdi_rx_*` instead of self-calls)
4. Removed duplicate `cdc_was_plugged()` declaration in `cdc.h`

## Architecture

### CDC Read Flow (Async Callback Model)

```
Timer (20ms) → monome_transport_read() → cdc_read()
                                              ↓
                                    if rxBusy == false:
                                       uhi_cdc_read(rxBuf, 64, &cdc_rx_done)
                                              ↓
                              USB HCD completes transfer
                                              ↓
                              cdc_rx_done(status, nbytes)
                                              ↓
                              if 0 < nbytes < 64: monome_read_serial()
                              rxBusy = false  ← always
```

Key points:
- `cdc_read()` is called from 20ms timer, only arms if `rxBusy == false`
- `cdc_rx_done()` ALWAYS clears `rxBusy` — unconditional re-arm on next timer tick
- Data processed regardless of `stat` — bytes are valid even on "error" status
- Full-buffer guard: if `nb == 64`, discard (likely truncation/overflow)
- `monome_read_serial()` runs in USB ISR context — parser is state-machine based, no blocking

### Boot Sequence

```
init_usb_host() → USB enumeration starts
init_monome()
app_init()
assign_main_event_handlers()
check_startup()
  init_events()  ← wipes queue (connect event lost here)
  app_launch()
  if cdc_was_plugged(): post kEventSerialConnect  ← flag set at enumeration time
```

## Files Modified

| File | Lines | Description |
|------|-------|-------------|
| `avr32_lib/src/usb/cdc/cdc.c` | 175 | CDC transport — boot flag, async read, error handling |
| `avr32_lib/src/usb/cdc/cdc.h` | 49 | Declarations for CDC API |
| `avr32_lib/src/main.c` | 427 | Boot sequence, event re-post logic |
| `avr32_lib/src/monome.c` | 999 | Protocol parser, transport wrappers |
| `apps/bees/src/app_timers.c` | 234 | Timer callbacks (20ms poll, 30ms refresh) |
| `avr32_lib/src/usb/cdc/uhi_cdc.h` | 51 | USB Host Interface declarations |

## Error Handling

We initially implemented elaborate error handling (`rxErrors` counter, backoff delays, status-code validation). This created regressions.

**Correct approach** (matching monome's proven implementation):
- Ignore `uhd_trans_status_t` status codes
- Always clear `rxBusy` in callback
- Let 20ms timer naturally retry
- Discard full 64-byte buffers (overflow guard)

## Consistency with Monome Eurorack

This implementation now aligns with `libavr32` used by Teletype and Ansible:
- Same async callback model
- Same 20ms timer interval
- Same unconditional re-arm strategy
- Same full-buffer discard guard
- Same ISR-context parsing

## Build

```bash
cd ~/aleph-repo/apps/bees
make
```

Or with Docker:
```bash
cd ~/aleph-repo
export PATH="/Applications/Docker.app/Contents/Resources/bin:$PATH"
docker run --rm --platform=linux/amd64 \
  -v "$(pwd):/aleph" -w /aleph/apps/bees \
  aleph-builder bash -c \
  "export PATH=/root/avr32-toolchain-linux/bin:\$PATH && make"
```

Output: `aleph-bees.hex`

## References

- `AVR32_CODING_WISDOM.md` — Cross-project principles extracted from this work
