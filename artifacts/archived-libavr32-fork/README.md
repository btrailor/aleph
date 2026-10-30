# Archived: the btrailor/libavr32 fork

**The Aleph repo no longer uses a libavr32 fork.** As of 2026-10-30 the
submodule points at upstream `monome/libavr32` `main` (`5eca040`), and the
Aleph CDC grid support is upstream's.

This directory keeps the record of what the fork was and why it existed.

---

## What the fork was

`btrailor/libavr32`, branch `aleph-develop-substrate`, tip `162b653`
(2026-07-13, "aleph substrate: commit working CDC/monome/FTDI changes for
reproducible builds").

23 commits on top of the 2017 base `a1376d63`. It added, in order:

| Date | Commit | What |
|---|---|---|
| 2025-11-11 | `bc43976` | CDC wrapper implementation for modern monome grids |
| 2025-11-11 | `776f91b` | CDC events in the event system |
| 2025-11-11 | `3b502e0` | CDC in the USB host configuration |
| 2025-11-11 | `80ab775` | Transport abstraction in `monome.c` (`use_cdc` flag) |
| 2025-11-11 | `8b3f9ed` | VID `0x0483` for 2021+ STM32 grids |
| 2025-11-11 | `66ec7de` | USB driver order FTDI, CDC, HID |
| 2025-11-11 | `2421eb7` / `58a30bf` | HID monome-VID rejection removed so CDC can claim grids |
| 2025-11-13 | `7967d1d` | FTDI RX callback guard for CDC devices |
| 2025-11-14 | `60b60b1` | ASCII vs UTF-16 descriptor handling |
| 2025-11-14 | `c700eba` | Grid size query deliberately skipped ("assume 128") |
| 2026-07-13 | `162b653` | Consolidated tip for reproducible builds |

## Why it was retired

Upstream `monome/libavr32` gained Aleph CDC support independently:

- `98d6b60` (2021-06-03, brian crabtree) — official CDC grid support
- `d173d0e` (2023-07-29, tehn) — CDC grid size query
- `db219e1` (2025-02-23, tehn) — CDC arc
- `4979de6` (2025-08-25, Greg Wuller) — `aleph: enable cdc uhi`
- libavr32 **PR #82** (merged 2026-08-02) — brings the above into `main`

Comparing the two implementations line by line showed the fork's apparent
unique capabilities were not, in fact, unique:

| Fork capability | Upstream status |
|---|---|
| CDC line coding (115200) | **Never actually transmitted** — `uhi_cdc_open()` has the `SET_LINE_CODING` request commented out and the value is written to a static that is never read. Upstream has none either, so **zero bytes differ on the wire**. |
| VID `0x0483` | Upstream reads `vid`/`pid` but **never checks them** — it accepts any CDC-class device. |
| ASCII/UTF-16 descriptors | Only needed because the fork read USB string descriptors to find the `"cdc"` serial prefix. Upstream identifies the device with a live protocol query instead. |
| Dual FTDI + CDC transport | Upstream defaults the transport pointers to FTDI and `monome_setup_mext()` repoints them at CDC — functionally equivalent to the fork's `use_cdc` flag. |
| Driver order FTDI, CDC, HID | Not needed: the HID driver only claims HID-class interfaces, and a CDC grid presents CDC-class. |
| FTDI RX callback guard | Upstream lacks it; may be worth proposing upstream separately. |
| — | **Upstream additionally has** the grid size query and CDC arc support, which the fork lacked. |

The Aleph-side port to upstream was **7 insertions and 48 deletions across 3
files**, and was hardware-verified: an Aleph built against upstream
`monome/libavr32` `5eca040` enumerates the CDC grid, lights the LEDs and
registers keys.

## The fork's repository

**Archived, not deleted:** `https://github.com/btrailor/libavr32`

Protective tags, pushed so the work can never be lost:

| Tag | Commit | What |
|---|---|---|
| `aleph-cdc-substrate-2026-07-13` | `162b653` | the pin the Aleph repo used |
| `aleph-cdc-transport-2026-07-13` | `9468ee2` | the `cdc-transport` tip, which carries the `cdc_disconnect()` fix |
| `aleph-cdc-compat-02469a2` | `9d3043b` | the earlier function-pointer CDC attempt |

Nothing references the fork any more, but it is the historical record of how
Aleph CDC support was achieved, and of the investigation that led to upstream.

## Lessons from this fork

1. **A capability in a fork is not the same as a capability on the wire.**
   The line-coding code looked like a real feature and was carefully
   documented — and it was never transmitted. Verify that a feature actually
   reaches the device.
2. **Divergence is not automatically an advantage.** The fork was ~8 months
   *ahead* of upstream's Aleph work but implemented the same thing
   differently and less completely.
3. **Check whether the upstream project is converging on the same need.**
   The Aleph CDC work was requested repeatedly over the years; upstream
   eventually did it. A fork that duplicates a converging upstream effort
   becomes a maintenance liability.
4. **The upstream contributions are still available.** See
   `docs/CDC-REGRESSION-CHECKLIST.md` items F1–F8 for the behaviours that must
   not regress in any future upstream change.
