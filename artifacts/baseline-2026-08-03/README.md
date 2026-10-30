# Known-good firmware baseline — 2026-08-03

This directory preserves the exact firmware binary that was **hardware-verified**
against a real monome grid. It is the rollback target and the comparison
baseline for any CDC-related change.

## Why this exists

The `.hex`/`.elf` outputs are gitignored (`*.hex`, `*.elf`), so the binary that
passed hardware testing existed only as a build artifact on one machine. If the
substrate were ever rebuilt from a different revision, the known-good binary
would have been unrecoverable.

`artifacts/` is explicitly un-ignored in `.gitignore` so these baselines ARE
tracked.

## The binary

| File | Size | SHA-256 |
|---|---|---|
| `aleph-bees.hex` | 499,723 B | `8305bdc882cb4d6e53da155bf4ba8197a6813411c11ca5afac7f9f3e5e3a4542` |
| `aleph-bees.elf` | 295,954 B | (symbols/debug info) |

| Property | Value |
|---|---|
| Firmware version | BEES 0.8.3 (`apps/bees/version.mk`: maj=0, min=8, rev=3) |
| Repo commit | `2e3189f8` "fix: dynamic network params overflow + convert all 0.7.1 scenes to 0.8.x format" |
| Built | 2026-08-03 18:49 EDT |
| Toolchain | `aleph-builder:latest` (linux/amd64), AVR32 GCC 4.4.7 |
| Submodule pin | libavr32 `162b653c1462ec50e7132ad59f18d39c40bfa2a2` |
| Reproducible? | **Yes** — a clean build from `develop` at this commit reproduces this exact SHA-256 |

Note the binary was built at 18:49 and the commit landed at 19:39 the same
evening; the binary *does* contain the `net.c` fix (verified via
`strings aleph-bees.elf | grep "failed to expand params array"`).

## How to verify a build matches this baseline

```bash
cd ~/aleph
docker run --rm --platform=linux/amd64 -v "$HOME/aleph:/host" -w /tmp aleph-builder:latest bash -c '
  cp -r /host /tmp/aleph && cd /tmp/aleph/apps/bees && \
  export PATH=/root/avr32-toolchain-linux/bin:/opt/uClinux/bfin-elf/bin:$PATH && \
  make clean && make && sha256sum aleph-bees.hex'
# Expect: 8305bdc882cb4d6e53da155bf4ba8197a6813411c11ca5afac7f9f3e5e3a4542
```

If the SHA differs, something in the build environment or source tree changed.
Diff the `.text`/`.data`/`.bss` sizes against the values below before assuming
the change is harmless.

## Section sizes at this baseline

```
text   data    bss     dec    hex
0x299d8  0x1c04  0x4031c5c  67490360  405d238
```

(`.text` 0x299d8, `.data` 0x1c04, `.bss` 0x4031c5c)

## Hardware validation status

| Test | Result | Date |
|---|---|---|
| `stepwaves.scn` load, grid sequencer, pitch/mod connections | ✅ pass | 2026-08-03 |
| `life.scn` load and function | ✅ pass | 2026-08-03 |
| Remaining 10 scenes | ⚠️ structurally validated only, never hardware-tested | — |
| **CDC grid enumeration + LED response** | ⚠️ **not re-verified against this binary since Aug 3** | — |

The CDC path carries its own known-good reference: see
`docs/CDC-REGRESSION-CHECKLIST.md` for the behaviors that must survive, and
`artifacts/baseline-2026-08-03/` for the binary to roll back to.

## Rollback

```bash
cd ~/aleph
cp artifacts/baseline-2026-08-03/aleph-bees.hex apps/bees/aleph-bees.hex
# then flash to the Aleph and confirm the grid works
```
