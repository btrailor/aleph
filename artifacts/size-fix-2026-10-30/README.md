# Grid-width fix — 2026-10-30

Candidate build where the cascade and step operators take their grid width
from the **detected** grid instead of from the scene. **Needs hardware
verification.**

## The binary

| File | Size | SHA-256 |
|---|---|---|
| `aleph-bees.hex` | 499,170 B | `1436cffb5d3ed187771c70ede310724c1cafc6436a764ddcc173f35dc6ca3f0d` |
| `aleph-bees.elf` | 295,954 B | (symbols/debug info) |

| Property | Value |
|---|---|
| Firmware version | BEES 0.8.3 |
| Repo commit | `11b5d6c8` "fix(ops): grid width must come from the detected grid, not the scene" |
| Submodule pin | libavr32 `162b653c1462ec50e7132ad59f18d39c40bfa2a2` (unchanged) |
| Builds on top of | `d80c1798` (DEBUG_MONOME gate) — so it includes that fix too |

## What was wrong

`op_cascades` and `op_step` both restore a `size` field from the scene file,
then let the network **SIZE input** overwrite it.

In `mpdrumssss.scn` the SIZE input is fed by an ENC's VAL output, whose value
is **1254** — the scene's clock/trigger value, not a width. The operators read
that as "8 wide":

```c
static void op_cascades_in_size(op_cascades_t* op, const io_t v) {
  if(v < 9) op->size = 8;      // 1254 >= 9 -> size = 16 ... but the ENC bus
  else op->size = 16;          // value varies; the stale scene value (8)
                               // is what actually survives
  op->XSIZE = op->size;
}
```

The operator already knows the correct width — `monome_size_x()` returns the
detected grid's column count (16 on a 128 grid) and `op_cascades_init()`
already calls it. Both the scene's stored value and the network input were
overriding that correct value, clamping cascades to columns 0–7 on a 16-wide
grid.

## What changed

| File | Change |
|---|---|
| `apps/bees/src/ops/op_cascades.c` | `init()` sets `XSIZE` from `monome_size_x()`; `in_size()` only honours the input when it names a real width that agrees with the detected one |
| `apps/bees/src/ops/op_step.c` | `in_size()` gets the same treatment — `op_step` uses `op->size` for all of its sequencer bounds, so it had the identical problem |

## Why this is low-risk

Building a pristine `HEAD` checkout and the working tree and comparing **every
object file** shows exactly **two** objects differ:

```
CHANGED: src/ops/op_cascades.o
CHANGED: src/ops/op_step.o
```

Everything else is byte-identical, including all CDC-critical objects:

```
✅ src/net.o
✅ src/net_monome.o
✅ src/handler.o
✅ src/app_timers.o
✅ src/dynamic_network.o
✅ src/ops/op_monome_grid_classic.o
✅ src/ops/op_monome_grid_raw.o
```

The libavr32 submodule is untouched.

## Flash instructions

```bash
cp ~/aleph/artifacts/size-fix-2026-10-30/aleph-bees.hex /Volumes/ALEPH/app/aleph-bees.hex
```

Eject the card properly, then hold **MODE** while powering on the Aleph.

Rollback:
```bash
cp ~/aleph/artifacts/baseline-2026-08-03/aleph-bees.hex /Volumes/ALEPH/app/aleph-bees.hex
# or the intermediate build with only the print_dbg fix:
cp ~/aleph/artifacts/debug-monome-fix/aleph-bees.hex /Volumes/ALEPH/app/aleph-bees.hex
```

## What to test

| Test | Expected |
|---|---|
| Grid enumerates, LEDs light, keys register | ✅ unchanged |
| `stepwaves.scn` loads and plays | ✅ unchanged, but the sequencer should now use the full **16** columns instead of 8 |
| **`mpdrumssss.scn` cascades across the full width** | 🎯 **the fix** — a cascade started in column 12 should now travel to column 1 instead of stopping at column 8 |
| Cascade still fires for every row | ✅ (already fixed by `5168d802`) |
| Audio dropouts during gestures | ✅ still fixed (build includes `d80c1798`) |

## If the cascade still stops at column 8

Then the width is being clamped somewhere else. The next place to look is
`op_cascades_trigger()`, where `op->points[op->rule_dests[n]]` is bounded by
`op->XSIZE-1` — worth confirming `XSIZE` reaches 16 at runtime.
