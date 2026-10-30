# Upstream-aligned build — 2026-10-30

Aleph firmware built against **upstream `monome/libavr32` main** (`5eca040`)
instead of our fork. **Needs hardware verification.**

## The binary

| File | Size | SHA-256 |
|---|---|---|
| `aleph-bees.hex` | 497,423 B | `008854dc4027bf863cdc15ff131e42009ed0d09679e693253b224b9e1d419acd` |
| `aleph-bees.elf` | — | (symbols) |

| Property | Value |
|---|---|
| Firmware version | BEES 0.8.3 |
| Repo commit | `ff473b6c` "align: move libavr32 to upstream monome main (5eca040)" |
| Branch | `feature/align-upstream-libavr32` |
| libavr32 | upstream `monome/libavr32` `5eca040` (**not** our fork) |
| `.text` | 169,412 (vs 170,136 on our fork — slightly smaller) |
| Aleph-side diff | **7 insertions, 48 deletions** across 3 files |

## What changed

| File | Change |
|---|---|
| `avr32/src/main.c` | `handler_CdcConnect()` calls `monome_setup_mext()` directly instead of our `cdc_setup()`; `handler_CdcDisconnect()` no longer calls `cdc_disconnect()` (upstream declares neither) |
| `apps/bees/src/app_timers.c` | polls `serial_read()` (upstream's transport function pointer) instead of `monome_transport_read()` |
| `avr32/src/usb/cdc/cdc.h` | **removed** — it shadowed upstream's `cdc.h` and mis-declared `cdc_write`/`cdc_read` |
| `libavr32` submodule | → upstream `5eca040` |

**Not changed:** all Aleph-side work — the grid-width fix (`11b5d6c8`), the
cascade scene fix (`5168d802`), the `DEBUG_MONOME` gate (`d80c1798`).

**Gained from upstream:** the grid size query and CDC arc support, neither of
which our fork had.

## Why our fork's CDC work turned out to be unnecessary

Verified in the sources, not assumed:

- **Line coding:** our `uhi_cdc_open()` has the `SET_LINE_CODING` request
  **commented out**; the value is written to a static and never read for
  transmission. Upstream has no line coding at all → **zero bytes differ on
  the wire**.
- **VID `0x0483`:** upstream reads `vid`/`pid` but **never checks them** — it
  accepts any CDC-class device. Our explicit check was unnecessary.
- **ASCII/UTF-16 descriptors:** ours exists only because we read USB string
  descriptors to find the `"cdc"` prefix. Upstream identifies the device with
  a live protocol query instead.
- **Dual FTDI/CDC:** upstream defaults the transport pointers to FTDI and
  `monome_setup_mext()` repoints them at CDC — equivalent to our `use_cdc` flag.

## Driver order is not a risk

Upstream's Aleph config lists HID first (`UHI_HID, UHI_FTDI, UHI_MIDI,
UHI_MCDC`), which looked like a problem. It isn't: the HID driver only claims
**HID-class** interfaces, and a CDC grid presents **CDC-class** — so HID
declines it and the CDC driver handles the device.

## Flash instructions

```bash
cp ~/aleph/artifacts/align-upstream-2026-10-30/aleph-bees.hex /Volumes/ALEPH/app/aleph-bees.hex
```

Eject the card properly, then hold **MODE** while powering on the Aleph.

**Rollback — two options, both one command:**
```bash
# (a) back to our fork (the previous known-good):
cd ~/aleph && git checkout develop
cp ~/aleph/artifacts/size-fix-2026-10-30/aleph-bees.hex /Volumes/ALEPH/app/aleph-bees.hex

# (b) back to the original Aug 3 baseline:
cp ~/aleph/artifacts/baseline-2026-08-03/aleph-bees.hex /Volumes/ALEPH/app/aleph-bees.hex
```

Note: option (a) changes the repo's checked-out branch. The **hex on the SD
card** is what determines what the Aleph runs, so you can flash (a) without
touching the repo at all — the `cp` is enough.

## What to test

| Test | Expected |
|---|---|
| **Grid enumerates** | ✅ grid appears, LEDs light |
| **Grid keys register** | ✅ |
| `stepwaves.scn` loads and plays | ✅ unchanged |
| `mpdrumssss.scn` cascades | ✅ unchanged (scene fix is Aleph-side) |
| No audio dropouts during gestures | ✅ unchanged |
| Grid size detection | 🎯 upstream does a **live size query** — the grid should report 16×8 correctly |

### If the grid does NOT connect

The debug log will say where it stopped. Most likely causes, in order:

1. **The size query.** Upstream's `monome_setup_mext()` issues a size request
   and waits 500 µs for a reply. If the grid doesn't answer in time, the
   width stays at the 16×8 default (which is still correct for your grid) but
   the connect may not complete. Check for `received:` in the debug output.
2. **Driver ordering** — ruled out above, but the log would show the HID
   driver claiming the device if I was wrong.
3. **Descriptor strings.** Upstream doesn't read them, so this shouldn't
   matter — but if the grid isn't identified at all, this is where I'd look.

Capture the debug log if anything misbehaves:
```bash
cat /dev/tty.usbmodem* > /tmp/aleph-align.log   # 57600 baud
```

## After this passes

If the upstream-aligned build works on hardware, the fork can be retired:

1. Merge `feature/align-upstream-libavr32` → `develop`.
2. Re-point `.gitmodules` at `monome/libavr32` + `branch = main` (currently
   `acc2f878` points it at our fork).
3. Keep the fork repo archived with its protective tags — don't delete it.
4. The `events.c` fixes (PR #83) remain the contribution path for future
   libavr32 changes.
