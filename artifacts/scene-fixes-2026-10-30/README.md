# Cascade scene fix — 2026-10-30

Corrected scene files for the `eOpCascades` state bug. **No firmware change
needed** — the fix is entirely in the converted scene data.

## The files

| File | SHA-256 |
|---|---|
| `scenes/default.scn` | `2eec8eeccd8292ed7af199fb0b0d7ecbd4081ecbb5173e870b375ce96aa3d8fe` |
| `scenes/mpdrumssss.scn` | `ca82859123869588258f455c5cabce99550a08327eb0da68c0d209b6c79a8127` |

Both are 262,200 bytes. They are also committed in the repo at
`utils/release/scenes-0.7.1/`.

## What was wrong

The `eOpCascades` operator (id 36) never serialized its internal state in
0.7.1 — its pickle function ended with the literal comment `/// no state...???`.
That state lived in file-scope statics which `init()` never reset, so after
loading a 0.7.1 scene the runtime state was always the file-scope defaults:

```
positions   = {3,1,2,2,3,3,5,7}
points      = {3,1,2,2,3,3,5,7}
points_save = {3,1,2,2,3,3,5,7}
trig_dests  = {0,0,0,0,0,0,0,0}
rules       = {0,0,0,0,0,0,0,0}
rule_dests  = {0,1,2,3,4,5,6,7}
```

0.8.x moved the same state into the per-operator struct with **identical**
defaults and made it serializable (focus + size + 14 u32 = 60 bytes).

The converter zero-padded that expansion, so the converted scene overwrote the
init defaults with an all-zero state:

```
rule_dests = [0,0,0,0,0,0,0,0]   -> every row's rule target resolved to row 0
points     = [0,0,0,0,0,0,0,0]   -> the countdown could not advance for any row but 0
```

That produced exactly the reported symptom: **row 1 cascaded; rows 2–8 lit up
on press but produced no cascade effect.**

## The fix

The converter now fills the expansion with the 0.8.x init defaults instead of
zeros, reproducing the 0.7.1 runtime state exactly.

Verified:
- The packed default bytes match a struct dump of `op_cascades_init()`'s
  defaults, byte for byte.
- Regenerating all 12 scenes from `originals-backup/` with the fixed converter:
  **10 of 12 are byte-identical** to the previously shipped scenes. Only
  `default.scn` and `mpdrumssss.scn` change (31 bytes each), and both contain
  an `eOpCascades` operator.
- No operator IDs, connections, or input nodes changed — only the cascade
  operator's expanded state region.

## Install to the SD card

The scenes live in `/Volumes/ALEPH/data/bees/scenes/` on the card. Copy these
two over:

```bash
cp ~/aleph/artifacts/scene-fixes-2026-10-30/scenes/default.scn \
   /Volumes/ALEPH/data/bees/scenes/default.scn
cp ~/aleph/artifacts/scene-fixes-2026-10-30/scenes/mpdrumssss.scn \
   /Volumes/ALEPH/data/bees/scenes/mpdrumssss.scn
```

Eject the card properly afterwards (Disk Utility, or Cmd+Q in Finder).

## What to test

| Test | Expected |
|---|---|
| Load `mpdrumssss.scn` | Cascade rows 2–8 now fire like row 1 |
| Press in row N | The cascade runs from the press column and hits column 1, firing that row's sound each time it lands |
| Grid LEDs | Unchanged |
| Load `default.scn` | Unchanged behaviour, but cascade state is now correct |

## If it still doesn't cascade

The next thing to check is the `size` field. The scene stores `size=8` (it was
written for an 8-wide grid), while the actual grid is 16 wide. `op->XSIZE` is
set from `op->size` during init but is **not** restored by unpickle — the
operator's own comment says so:

```c
// FIXME: auto-detect size, this doesn't get set in unpickling
op->XSIZE = op->size;
```

If the cascade fires but the horizontal positions look wrong (e.g. it wraps at
column 8 instead of 16), that is the cause, and the fix is a separate one.
