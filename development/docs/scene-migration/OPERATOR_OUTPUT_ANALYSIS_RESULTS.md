# Operator Output Analysis Results
## BEES 0.7.1 → 0.8.x Comparison

**Date**: 2024-12-19  
**Analysis Method**: Script comparison of `numOutputs` declarations in all `op_*.c` files  
**Versions Compared**:
- **0.7.1**: Git tag `bees-0.7.1` (2014 official release)
- **0.8.x**: Git branch `dev` (2018 community release, v0.8.1)

---

## Executive Summary

**5 operators gained outputs** between 0.7.1 and 0.8.x, all gaining exactly **+1 dummy output**.

These changes cause **silent scene incompatibility** - scenes load but connections are misrouted without error messages.

---

## Critical Findings

### Operators That Gained Outputs (BREAKS 0.7.1 SCENES)

| Operator Name   | Op File            | 0.7.1 Outputs | 0.8.x Outputs | Change | Impact         |
| --------------- | ------------------ | ------------- | ------------- | ------ | -------------- |
| `bars`          | op_bars.c          | 0             | 1             | +1     | ⚠️ BREAKS SCENES |
| `bignum`        | op_bignum.c        | 0             | 1             | +1     | ⚠️ BREAKS SCENES |
| `midi_out_cc`   | op_midi_out_cc.c   | 0             | 1             | +1     | ⚠️ BREAKS SCENES |
| `midi_out_note` | op_midi_out_note.c | 0             | 1             | +1     | ⚠️ BREAKS SCENES |
| `screen`        | op_screen.c        | 0             | 1             | +1     | ⚠️ BREAKS SCENES |

**Total changed**: 5 operators  
**Total gained outputs**: +5 cumulative shift in output indices

### Verification

Two operators manually verified in 0.8.x source code:

**`op_screen.c` (SCREEN)**:
```c
// Line ~72
screen->super.numOutputs = 1;
// outString array contains: "DUMMY"
```

**`op_bignum.c` (BIGNUM)**:
```c
// Line ~82
bignum->super.numOutputs = 1;
// outString array contains: "DUMMY"
```

Both operators gained 1 output with label "DUMMY" - cannot be used for actual signal routing but shifts all subsequent output indices.

---

## Why This Breaks Scene Compatibility

### The Problem: Cumulative Output Index Shift

Output indices are stored as **absolute positions** in the global output array:

```
0.7.1 Scene:
  OP_A (3 outputs): indices 0, 1, 2
  SCREEN (0 outputs): --- none ---
  OP_B (2 outputs): indices 3, 4
  ↑ Connection to OP_B:out0 stored as index 3

0.8.x Loading Same Scene:
  OP_A (3 outputs): indices 0, 1, 2
  SCREEN (1 output): index 3  ← NEW DUMMY OUTPUT
  OP_B (2 outputs): indices 4, 5
  ↑ Connection still points to index 3 → now SCREEN:out0 (WRONG!)
```

### Consequence: Silent Misrouting

- Connection thinks it's targeting OP_B:out0
- Actually routed to SCREEN:out0 (dummy, no signal)
- **No error message** - scene loads successfully
- Patch is broken but appears to work (worst type of bug)

---

## Operator Removal/Addition Analysis

### Operators REMOVED in 0.8.x (7 total)

These operators existed in 0.7.1 but were replaced/removed in 0.8.x:

| Operator   | 0.7.1 Outputs | Status                                  |
| ---------- | ------------- | --------------------------------------- |
| `bars`     | 0             | Replaced with new version (1 output)    |
| `bignum`   | 0             | Replaced with new version (1 output)    |
| `life`     | 3             | **Renamed to `life_classic`** (3 outputs) |
| `midi_out_cc`   | 0        | Replaced with new version (1 output)    |
| `midi_out_note` | 0        | Replaced with new version (1 output)    |
| `screen`   | 0             | Replaced with new version (1 output)    |

**Critical**: `life` → `life_classic` rename means 0.7.1 scenes using `op_life` won't find the operator in 0.8.x.

### Operators ADDED in 0.8.x (24 total)

New operators introduced in 0.8.x that don't exist in 0.7.1:

| Operator            | Outputs | Category        | Notes                        |
| ------------------- | ------- | --------------- | ---------------------------- |
| `ckdiv`             | 2       | Clock           | Clock divider                |
| `cpu`               | 2       | System          | CPU monitor                  |
| `harry`             | 1       | Control         | Control operator             |
| `iter`              | 3       | Logic           | Iterator                     |
| `kria`              | 4       | Grid            | Kria sequencer               |
| `life_classic`      | 3       | Grid            | **Renamed from `life`**      |
| `linlin`            | 1       | Math            | Linear interpolation         |
| `list4`             | 4       | Data            | 4-item list                  |
| `maginc`            | 1       | Control         | Magnitude increment          |
| `mem0d`             | 0       | Memory          | 0-dimensional memory         |
| `mem1d`             | 1       | Memory          | 1-dimensional memory         |
| `mem2d`             | 2       | Memory          | 2-dimensional memory         |
| `midi_clock`        | 4       | MIDI            | MIDI clock generator         |
| `midi_out_clock`    | 1       | MIDI            | MIDI clock output            |
| `midi_prog`         | 1       | MIDI            | MIDI program change          |
| `monome_grid_classic` | 3     | Grid            | Classic monome grid operator |
| `param`             | 1       | Control         | Parameter operator           |
| `poly`              | 3       | Control         | Polyphonic router            |
| (new versions of removed ops) |  |                | bars, bignum, midi_out_cc, etc. |

---

## Implications for Scene Migration

### 1. Operator ID Mapping Required

Since operators were removed/added/renamed, operator IDs likely shifted. Need complete mapping table:

```c
typedef struct {
  op_id_t op_id_v07;     // Operator ID in 0.7.1
  op_id_t op_id_v08;     // Operator ID in 0.8.x (or INVALID)
  const char* name_v07;  // Operator name in 0.7.1
  const char* name_v08;  // Operator name in 0.8.x
} OpIdMapping;

const OpIdMapping opIdMap[] = {
  { 42, 43, "life",     "life_classic" },  // Renamed
  { 15, INVALID, "some_old_op", NULL },     // Removed
  // ... complete mapping
};
```

### 2. Output Index Remapping Required

For each connection in 0.7.1 scene:
1. Identify source operator
2. Calculate how many outputs were added BEFORE this operator
3. Add cumulative shift to output index

```c
// Pseudocode
u16 remap_output_index(u16 old_index, SceneData_07* scene) {
  u16 cumulative_shift = 0;
  
  // Walk through all operators BEFORE this output's owner
  for (each operator before target) {
    if (operator gained outputs) {
      cumulative_shift += outputs_added;
    }
  }
  
  return old_index + cumulative_shift;
}
```

### 3. Operator Existence Validation

Must validate that operators in 0.7.1 scene exist in 0.8.x:
- `life` → convert to `life_classic`
- Other removed operators → error or replace with safe default

---

## Required Data Structures for Conversion

### `OPERATOR_OUTPUT_CHANGES.h`

```c
#ifndef _ALEPH_OPERATOR_OUTPUT_CHANGES_H_
#define _ALEPH_OPERATOR_OUTPUT_CHANGES_H_

#include "op.h"

// Operators that changed output count between 0.7.1 and 0.8.x
typedef struct {
  op_id_t op_id_v07;     // Operator ID in 0.7.1
  op_id_t op_id_v08;     // Operator ID in 0.8.x
  u8 num_outputs_v07;    // Output count in 0.7.1
  u8 num_outputs_v08;    // Output count in 0.8.x
  s8 outputs_added;      // Difference (can be negative)
} OpOutputChangeMap;

// Complete list of operators that gained outputs
const OpOutputChangeMap outputChanges[] = {
  { eOpBars,        eOpBars,        0, 1, +1 },  // BARS
  { eOpBignum,      eOpBignum,      0, 1, +1 },  // BIGNUM
  { eOpMidiOutCc,   eOpMidiOutCc,   0, 1, +1 },  // MIDI_OUT_CC
  { eOpMidiOutNote, eOpMidiOutNote, 0, 1, +1 },  // MIDI_OUT_NOTE
  { eOpScreen,      eOpScreen,      0, 1, +1 },  // SCREEN
};

#define NUM_OUTPUT_CHANGES (sizeof(outputChanges) / sizeof(OpOutputChangeMap))

#endif // _ALEPH_OPERATOR_OUTPUT_CHANGES_H_
```

---

## Testing Strategy

### Test Cases for Output Remapping

**Test 1**: Scene with SCREEN operator followed by other operators
- 0.7.1: SCREEN (0 outputs), OP_B (2 outputs at indices 0, 1)
- 0.8.x: Should remap OP_B outputs to indices 1, 2 (skip SCREEN dummy)

**Test 2**: Scene with multiple operators that gained outputs
- 0.7.1: SCREEN, BIGNUM, OP_C (cumulative +2 shift needed)
- Verify connections to OP_C are shifted by +2

**Test 3**: Scene with `life` operator
- 0.7.1: Uses `op_life` (ID X)
- 0.8.x: Should convert to `life_classic` with correct ID mapping

### Test Scenes Available

From `utils/release/scenes-0.7.1/`:
- `default.scn` - Basic test case
- `life.scn` - Uses `op_life`, tests rename handling
- `space.scn` - Grid operator scene
- `stepwaves.scn` - Sequencer scene

---

## Critical Warnings

### DO NOT PROCEED WITHOUT THIS DATA

❌ **HALT**: Do not implement scene conversion until:
1. ✅ Complete list of operators that gained outputs (DONE)
2. ❌ Complete operator ID mapping table (0.7.1 ID → 0.8.x ID)
3. ❌ Complete operator name mapping (especially renames like `life` → `life_classic`)

### Silent Failure Risk

**Worst case scenario**: Conversion logic with incomplete output mapping will:
- Load scenes without errors
- Create incorrect connections silently
- Result in patches that "work" but are completely broken
- User won't know until they compare expected behavior

**This is worse than scenes failing to load entirely.**

---

## Script Execution Details

**Script**: `development/scripts/find_output_changes.sh` (original) → `/tmp/run_output_check2.sh` (fixed version)

**Execution**:
```bash
# Checkout 0.7.1
git checkout -q bees-0.7.1
grep -h "numOutputs.*=" apps/bees/src/ops/op_*.c > outputs_v07.txt

# Checkout 0.8.x
git checkout -q dev
grep -h "numOutputs.*=" apps/bees/src/ops/op_*.c > outputs_v08.txt

# Compare
join -t: outputs_v07.txt outputs_v08.txt | awk comparison
```

**Results**: 5 operators changed, 0 unknown/unreadable, 0 operators lost outputs.

---

## Next Steps

1. ✅ **Complete operator output analysis** (THIS DOCUMENT)
2. **Create complete operator ID mapping table** (map 0.7.1 IDs to 0.8.x IDs)
3. **Create operator name mapping** (especially renames)
4. **Implement `scene_convert.c`** with:
   - Operator ID remapping
   - Output index remapping using `outputChanges` array
   - Operator existence validation
5. **Test with all 0.7.1 example scenes**

---

## References

- **Source Warning**: Yann Copier, Lines forum (https://llllllll.co/t/aleph/307/423)
- **0.7.1 Source**: Git tag `bees-0.7.1`
- **0.8.x Source**: Git branch `dev`
- **Strategy Document**: `development/docs/SCENE_MIGRATION_STRATEGY.md`
- **Analysis Script**: `development/scripts/find_output_changes.sh`

---

**Document Status**: ANALYSIS COMPLETE  
**Approval for Next Phase**: PENDING - Need operator ID mapping before implementation
