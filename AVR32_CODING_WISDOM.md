# AVR32 / Aleph Coding Wisdom

## CDC USB Implementation — Patterns from Monome Eurorack

### Core Philosophy: Simplicity over Defensiveness

Monome's proven CDC implementation across Teletype, Ansible, and Aleph follows a consistent pattern: **minimal error handling, unconditional state clearing, timer-driven polling.**

### The Pattern

```c
// Timer callback (20ms interval)
void cdc_read(void) {
  if (rxBusy == false) {
    rxBytes = 0;
    rxBusy = true;
    if (!uhi_cdc_read(rxBuf, CDC_RX_BUF_SIZE, &cdc_rx_done)) {
      rxBusy = false;  // immediate retry on next timer tick
    }
  }
}

// USB completion callback (interrupt context)
static void cdc_rx_done(usb_add_t add, usb_ep_t ep,
                        uhd_trans_status_t stat, iram_size_t nb) {
  rxBytes = nb;

  // Process data regardless of error status
  // FIXME: if the buffer is full, it's a false receive
  if (rxBytes > 0 && rxBytes < CDC_RX_BUF_SIZE) {
    (*monome_read_serial)();  // parse in interrupt context
  }

  rxBytes = 0;
  rxBusy = false;  // ALWAYS clear, regardless of stat
}
```

### Key Principles

1. **Ignore transfer status codes** — `uhd_trans_status_t` received but never checked. Data bytes are valid even on "error" status.

2. **Unconditional re-arm** — `rxBusy = false` in every callback path. Next timer tick (20ms) will retry naturally.

3. **Full-buffer guard** — If `nb == CDC_RX_BUF_SIZE` (64 bytes), discard. Indicates truncation/overflow, not a complete message.

4. **Interrupt-context parsing** — `monome_read_serial()` runs in the USB ISR, not deferred to main loop. This is acceptable because:
   - Parser is state-machine based (no blocking)
   - 20ms interval provides natural spacing
   - Event posting (`event_post()`) from ISR is safe in this architecture

5. **No error backoff** — At 20ms polling on Full-Speed USB, natural retry is sufficient. Adding backoff counters or retry logic adds complexity without benefit.

### Anti-Patterns We Learned

| Anti-Pattern | Why It Failed | Correct Approach |
|-------------|---------------|-----------------|
| `rxErrors` counter with backoff | Once threshold hit, permanent stall | Unconditional `rxBusy = false` |
| Checking `stat == UHD_TRANS_NOERROR` before processing | Drops valid data on "error" status | Process all data regardless of status |
| Calling `monome_read_serial()` from both ISR and timer | Double-posts key events | ISR only |
| `cdc_read()` checking error state before arming | Prevents recovery after errors | Check `rxBusy` only |

### Boot Detection

Monome's modules use the same pattern we converged on:
- `cdc_change()` sets flag at enumeration time
- `check_startup()` checks flag after `app_launch()`
- Event re-posted if device was plugged during boot

This is necessary because `init_events()` wipes the event queue during startup.

### File Structure

| File | Purpose |
|------|---------|
| `cdc.c` / `cdc.h` | CDC transport layer |
| `uhi_cdc.c` / `uhi_cdc.h` | USB Host Interface (ASF wrapper) |
| `monome.c` / `monome.h` | Protocol parser (mext/series) |
| `app_timers.c` | Timer callbacks (20ms poll, 30ms refresh) |
| `main.c` | Event loop, boot sequence |

### Build Notes

- Docker builder: `aleph-builder` image with AVR32 toolchain
- Output: `aleph-bees.hex`
- Flash to SD card `app/` folder

---

## General Firmware Wisdom

### Event-Driven Architecture

The Aleph uses a cooperative event loop, not preemptive threading:
- Events posted to queue via `event_post()`
- Processed serially in `check_events()`
- Interrupts only set flags and post events — no heavy work in ISR

### State Machine Pattern

For protocol parsing (monome grid, MIDI, etc.):
- Use explicit state enum
- Process one byte per call
- Never block or loop waiting for data

### USB Callbacks

All UHC callbacks run in interrupt context:
- Keep them short
- Use `event_post()` to defer heavy work
- But for CDC reads, parsing is lightweight enough to do inline

### Debug Output

`print_dbg()` uses serial UART — slow and blocking:
- Remove debug prints in production
- Keep only critical error reporting
- Commented-out prints are fine for future debugging
