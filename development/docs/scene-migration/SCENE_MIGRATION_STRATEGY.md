# Scene Migration Strategy: 0.7.1 → 0.8.x

## Status Update: Phase 1 COMPLETE ✅

**Date**: 2026-01-10

**Analysis Phase Complete**:

- ✅ Operator output changes identified (5 operators, +5 cumulative shift)
- ✅ Operator ID mapping extracted (remarkably stable, 48/50 unchanged)
- ✅ Data structures created (OPERATOR_ID_MAPPING.h, OPERATOR_OUTPUT_CHANGES.h)
- ✅ All documentation complete

**Ready for**: Implementation phase (scene_convert.c on feature branch)

---

## Executive Summary

BEES 0.8.x introduced **breaking scene format changes** that make 0.7.1 scenes incompatible. This document outlines the comprehensive strategy for enabling 0.7.1 scenes to load in 0.8.x, which is a **prerequisite** for CDC grid compatibility work.

**Status**: Analysis complete, ready for implementation  
**Priority**: Critical - blocks CDC grid testing with existing scenes

---

## Problem Statement

### Why This Matters

1. **Testing Blocker**: Cannot test CDC grid changes with existing 0.7.1 scenes
2. **User Adoption**: Users lose their patch libraries when upgrading
3. **Community Value**: Lines forum has many 0.7.1 scenes shared
4. **Development Speed**: Faster iteration with real-world test scenes

### Current Situation

The code has a **placeholder** for 0.7.x scene compatibility:

```c
// apps/bees/src/scene.c line 211
needsConnectionRemapping = (sceneData->desc.beesVersion.maj == 0 &&
                           sceneData->desc.beesVersion.min == 7) ? 1 : 0;
```

**Problem**: The `needsConnectionRemapping` flag is **SET but NEVER USED**. The actual remapping logic was never implemented.

---

## Root Causes of Incompatibility

### 1. Memory Management Overhaul (PR #267)

**0.7.x System**:

```c
// Static operator pool
op_t operators[MAX_OPS];  // Fixed allocation
```

**0.8.x System**:

```c
// Dynamic allocation via malloc
op_t* op = malloc(sizeof(op_t));
// ... use operator ...
free(op);  // Can be deleted mid-patch
```

**Impact**: Operator indices and memory layout completely changed

### 2. Operator Architecture Changes

- New operators added in 0.8.x
- Some operators removed or merged
- Operator IDs shifted
- Input/output orderings changed
- **Critical**: Some operators gained dummy outputs (SCREEN, BIGNUM) - breaks scene compatibility

### 3. Preset System Overhaul

**0.7.x Issues**:

- Buggy preset storage
- Max 32 presets
- OUTPUT routings NOT in presets

**0.8.x Improvements**:

- Reliable storage/recall
- Expandable preset count
- OUTPUT routings included
- Different binary format

### 4. Parameter Storage Format

- Parameter indices changed
- Scaler format differences
- Different serialization structure

---

## Investigation Findings

### What's Implemented ✅

1. **Version Detection** - Works correctly:

   ```c
   sceneData->desc.beesVersion.min = *src;
   sceneData->desc.beesVersion.maj = *src;
   ```

2. **Flag Setting** - Sets the remapping flag:

   ```c
   needsConnectionRemapping = (maj == 0 && min == 7) ? 1 : 0;
   ```

3. **Module Name Compatibility** - Handles old module names:
   ```c
   if(strncmp(moduleName, "aleph-", 6) == 0) {
     // Strip prefix and try again
     strcpy(sceneData->desc.moduleName, &(moduleName[6]));
     files_load_dsp_name(sceneData->desc.moduleName);
   }
   ```

### What's Missing ❌

1. **No remapping logic** - `needsConnectionRemapping` is never checked
2. **No operator ID mapping** - Old IDs don't translate to new IDs
3. **No connection index adjustment** - Input/output indices don't remap
4. **No preset format conversion** - Old presets fail to load
5. **No error reporting** - Silent failures when loading 0.7.x scenes

---

## Technical Analysis

### Scene File Structure

Both 0.7.x and 0.8.x use binary "pickle" format:

```
Scene File Layout:
├── Scene Descriptor
│   ├── Scene Name (24 bytes)
│   ├── Module Name (variable)
│   ├── Module Version (maj, min, rev)
│   └── BEES Version (maj, min, rev) ← Detection point
├── Network Data
│   ├── Operator Count
│   ├── Operator Array
│   │   ├── Operator ID ← INCOMPATIBLE
│   │   ├── Operator State (pickled)
│   │   └── Input/Output indices ← INCOMPATIBLE
│   └── Connection Array ← INCOMPATIBLE
└── Preset Data ← INCOMPATIBLE
```

### Specific Incompatibilities

#### A. Operator ID Changes

Example operator ID shifts:

| Operator         | 0.7.x ID | 0.8.x ID | Status   |
| ---------------- | -------- | -------- | -------- |
| ADD              | 0        | 0        | Same     |
| SUB              | 1        | 1        | Same     |
| MUL              | 2        | 2        | Same     |
| ...              | ...      | ...      | ...      |
| WW (White Whale) | ?        | 71       | Added    |
| STEP             | ?        | 69       | Changed? |
| KRIA             | ?        | 54       | Changed? |

**Need**: Complete mapping table of all operator IDs

#### B. Input/Output Index Shifts

When operators were added/removed, all I/O indices shifted:

```
0.7.x: [OP_A inputs][OP_B inputs][OP_C inputs]...
        0-3          4-7          8-11

0.8.x: [OP_A inputs][NEW_OP inputs][OP_B inputs][OP_C inputs]...
        0-3          4-7            8-11         12-15
                     ↑ NEW
```

Connections stored as indices become invalid.

#### C. Operators Gaining Outputs (Critical!)

**Source**: Yann Copier on Lines forum re: 0.8.x release

**ANALYSIS COMPLETE** (2024-12-19): Script executed comparing bees-0.7.1 tag vs dev branch.

**Issue**: Several operators that had NO outputs in 0.7.x now have dummy outputs in 0.8.x.

**COMPLETE LIST OF OPERATORS THAT GAINED OUTPUTS (5 total):**

1. **`bars`** (BARS): 0 outputs → 1 output (+1 dummy)
2. **`bignum`** (BIGNUM): 0 outputs → 1 output (+1 dummy)
3. **`midi_out_cc`** (MIDI_OUT_CC): 0 outputs → 1 output (+1 dummy)
4. **`midi_out_note`** (MIDI_OUT_NOTE): 0 outputs → 1 output (+1 dummy)
5. **`screen`** (SCREEN): 0 outputs → 1 output (+1 dummy)

**Verification Examples:**

- `op_screen.c` (line ~72): `screen->super.numOutputs = 1;` with outString = "DUMMY"
- `op_bignum.c` (line ~82): `bignum->super.numOutputs = 1;` with outString = "DUMMY"

**Impact on Scene Compatibility**:

```
0.7.x Scene:
Operator Network: [OP_A outputs][SCREEN (no outputs)][OP_B outputs]
                   0-3                                 4-7

0.8.x Same Scene:
Operator Network: [OP_A outputs][SCREEN outputs][OP_B outputs]
                   0-3            4              5-8
                                  ↑ NEW dummy output
```

All output indices after operators that gained outputs are now **off by N** (where N = number of new outputs).

**Consequence**: Connections to outputs become misrouted:

- A connection to old output index 4 (OP_B:out0) now points to index 4 (SCREEN:out0)
- This is a **silent failure** - wrong routing, no error message
- Can cause completely broken patches that appear to load successfully

**Additional Incompatibilities Found:**

- **7 operators REMOVED** in 0.8.x: Old versions of bars, bignum, life, midi_out_cc, midi_out_note, screen
- **24 operators ADDED** in 0.8.x: Including ckdiv, cpu, harry, iter, kria, life_classic, linlin, list4, maginc, mem0d, mem1d, mem2d, midi_clock, midi_out_clock, midi_prog, monome_grid_classic, param, poly
- **Key rename**: `life` → `life_classic` (0.7.1 scenes using op_life won't find it in 0.8.x)

**Detection Strategy**:

1. Must track which operators gained outputs between versions
2. When converting 0.7.x scene, count outputs before each operator
3. Adjust all output indices based on cumulative output additions

**Required Data Structure**:

```c
typedef struct {
  op_id_t op_id_v07;
  op_id_t op_id_v08;
  u8 num_outputs_v07;  // How many outputs in 0.7.x
  u8 num_outputs_v08;  // How many outputs in 0.8.x
  u8 outputs_added;    // Difference (v08 - v07)
} OpOutputChangeMap;

// Complete mapping from analysis:
const OpOutputChangeMap outputChanges[] = {
  { eOpBars,         eOpBars,         0, 1, 1 },  // BARS gained 1 output
  { eOpBignum,       eOpBignum,       0, 1, 1 },  // BIGNUM gained 1 output
  { eOpMidiOutCc,    eOpMidiOutCc,    0, 1, 1 },  // MIDI_OUT_CC gained 1 output
  { eOpMidiOutNote,  eOpMidiOutNote,  0, 1, 1 },  // MIDI_OUT_NOTE gained 1 output
  { eOpScreen,       eOpScreen,       0, 1, 1 },  // SCREEN gained 1 output
};
```

#### D. Preset Format Changes

0.7.x presets don't include:

- OUTPUT routing states
- Extended preset slots
- New parameter indices

---

## Migration Strategy

### Phase 1: Analysis & Mapping (Week 1-2)

#### Tasks:

1. **Extract 0.7.1 Scene Data**

   - Get sdcard-0.7.1.zip scenes
   - Parse with debug prints
   - Document structure

2. **Extract 0.8.1 Scene Data**

   - Get sdcard-0.8.1.zip scenes
   - Parse same scenes (if available)
   - Document structure differences

3. **Create Mapping Tables**

   ```c
   // Operator ID mapping
   const op_id_t opIdMap_v07_to_v08[NUM_OPS_V07] = {
     [eOpAdd] = eOpAdd,      // Same
     [eOpSub] = eOpSub,      // Same
     // ... map all operators
   };

   // Operator output count changes (CRITICAL!)
   typedef struct {
     op_id_t op_id_v07;
     u8 num_outputs_v07;
     u8 num_outputs_v08;
   } OpOutputChange;

   const OpOutputChange opOutputChanges[] = {
     { eOpScreen, 0, 1 },    // SCREEN: 0→1 outputs
     { eOpBignum, 0, 1 },    // BIGNUM: 0→1 outputs
     // ... identify ALL operators that changed output count
   };
   ```

4. **Document Format Differences**
   - Binary offset changes
   - Size differences
   - New/removed fields

#### Deliverables:

- `SCENE_FORMAT_07_ANALYSIS.md`
- `SCENE_FORMAT_08_ANALYSIS.md`
- `OPERATOR_ID_MAPPING.h`
- `OPERATOR_OUTPUT_CHANGES.h` - **Critical: Track output count changes**
- `scene_format_comparison.xlsx`

### Phase 2: Implement Conversion Logic (Week 3-4)

#### A. Create Scene Converter Module

New file: `apps/bees/src/scene_convert.c`

```c
// Scene conversion functions
typedef struct {
  u8 sourceVersion_maj;
  u8 sourceVersion_min;
  u8 targetVersion_maj;
  u8 targetVersion_min;
} SceneVersionInfo;

// Main conversion entry point
u8* scene_convert_07_to_08(const u8* src_07, u8* dst_08);

// Sub-conversions
static op_id_t convert_operator_id(op_id_t old_id);
static u32 convert_input_index(u32 old_idx, /* context */);
static u32 convert_output_index(u32 old_idx, /* context */);
static u8* convert_preset_data(const u8* src, u8* dst);
```

#### B. Implement Operator ID Remapping

```c
static op_id_t convert_operator_id(op_id_t old_id) {
  if (old_id >= NUM_OPS_V07) {
    // Invalid operator for 0.7.x
    print_dbg("\r\n WARNING: Unknown operator ID in 0.7 scene");
    return eOpInvalid;
  }

  op_id_t new_id = opIdMap_v07_to_v08[old_id];

  if (new_id == eOpInvalid) {
    print_dbg("\r\n WARNING: Operator removed in 0.8.x: ");
    print_dbg_ulong(old_id);
  }

  return new_id;
}
```

#### C. Implement Connection Remapping (with Output Count Tracking)

```c
// Track index shifts as we convert operators
typedef struct {
  u32 old_in_idx;
  u32 new_in_idx;
  u32 old_out_idx;
  u32 new_out_idx;
  u32 output_shift;  // Cumulative output additions before this op
} IndexMapEntry;

static IndexMapEntry indexMap[NET_INS_MAX];
static u32 indexMapCount = 0;
static u32 cumulativeOutputShift = 0;  // Track total output additions

static u32 convert_input_index(u32 old_idx) {
  for (u32 i = 0; i < indexMapCount; i++) {
    if (indexMap[i].old_in_idx == old_idx) {
      return indexMap[i].new_in_idx;
    }
  }
  // Not found - may be invalid
  return -1;
}

static u32 convert_output_index(u32 old_idx) {
  // CRITICAL: Must account for operators that gained outputs
  // As we iterate through operators, track cumulative output additions

  for (u32 i = 0; i < indexMapCount; i++) {
    if (indexMap[i].old_out_idx == old_idx) {
      // Apply the cumulative shift from all previous operators
      return indexMap[i].new_out_idx + indexMap[i].output_shift;
    }
  }

  // If not found in map, still need to apply cumulative shift
  return old_idx + cumulativeOutputShift;
}

// Called during operator conversion
static void track_operator_conversion(op_id_t old_id, op_id_t new_id,
                                      u32 old_out_idx, u32 new_out_idx) {
  IndexMapEntry* entry = &indexMap[indexMapCount++];
  entry->old_out_idx = old_out_idx;
  entry->new_out_idx = new_out_idx;
  entry->output_shift = cumulativeOutputShift;

  // Check if this operator gained outputs
  u8 outputs_added = get_outputs_added(old_id, new_id);
  if (outputs_added > 0) {
    print_dbg("\r\n Operator gained outputs: ");
    print_dbg_ulong(outputs_added);
    cumulativeOutputShift += outputs_added;
  }
}
```

#### D. Implement Preset Conversion

```c
static u8* convert_preset_data(const u8* src_07, u8* dst_08) {
  // 0.7.x preset structure
  typedef struct {
    u32 enabled;
    u32 target;
    s32 value;
  } preset_out_07_t;

  // Convert to 0.8.x structure (includes outputs)
  // May need to synthesize default OUTPUT values

  // ... conversion logic ...
}
```

### Phase 3: Integration (Week 5)

#### Modify `scene_read_buf()` in scene.c:

```c
// After detecting version
if (needsConnectionRemapping) {
  print_dbg("\r\n Converting 0.7.x scene to 0.8.x format...");

  // Allocate temporary buffer for converted scene
  u8* convertedData = (u8*)malloc(SCENE_PICKLE_SIZE);

  if (convertedData == NULL) {
    print_dbg("\r\n ERROR: Cannot allocate conversion buffer");
    return;
  }

  // Convert the scene data
  u8* converted_end = scene_convert_07_to_08(src, convertedData);

  if (converted_end == NULL) {
    print_dbg("\r\n ERROR: Scene conversion failed");
    free(convertedData);
    return;
  }

  // Continue loading with converted data
  src = convertedData;
  needsConnectionRemapping = 0; // Prevent re-conversion

  // ... continue normal loading ...

  // Clean up conversion buffer after scene is loaded
  // (need to defer until after net_unpickle())
}
```

### Phase 4: Testing (Week 6)

#### Test Suite:

1. **Unit Tests**

   - Test operator ID mapping
   - Test index remapping
   - Test preset conversion

2. **Integration Tests**

   - Load all 0.7.1 example scenes
   - Verify operators created correctly
   - Verify connections preserved
   - Verify presets work

3. **Real-World Tests**

   ```
   Test scenes:
   - utils/release/scenes-0.7.1/default.scn
   - utils/release/scenes-0.7.1/life.scn
   - utils/release/scenes-0.7.1/space.scn
   - utils/release/scenes-0.7.1/stepwaves.scn
   - Community scenes from lines forum
   ```

4. **Grid Operator Tests** (Critical for CDC work)
   ```
   Scenes with grid operators:
   - life.scn (op_life_classic)
   - Any scene with STEP operator
   - Any scene with WW operator
   - Any scene with KRIA operator
   ```

---

## Implementation Priority

### Must-Have (Critical Path)

1. ✅ Version detection (already works)
2. ❌ **Operator ID mapping table**
3. ❌ **Basic operator conversion**
4. ❌ **Connection index remapping**
5. ❌ **Scene loading with conversion**

### Should-Have (Important)

6. ❌ Preset data conversion
7. ❌ Error reporting and logging
8. ❌ Validation of converted scenes

### Nice-to-Have (Future)

9. ⬜ Beekeep GUI conversion tool
10. ⬜ Batch converter script
11. ⬜ Conversion report generation
12. ⬜ 0.8.x → 1.0 migration path

---

## Success Criteria

### Minimum Viable Conversion (Critical for CDC work)

- [ ] Can load 0.7.1 default.scn in 0.8.x BEES
- [ ] Operators are created (even if disconnected)
- [ ] Module loads correctly
- [ ] No crashes or hangs

### Full Conversion Success

- [ ] All 0.7.1 example scenes load
- [ ] All connections preserved correctly
- [ ] All operator states preserved
- [ ] Presets work (even if recreated)
- [ ] Grid operators work correctly ← **Critical for CDC**

---

## Risk Assessment

### High Risk ⚠️

1. **Unknown Operator Mapping**: Some 0.7.x operators may not exist in 0.8.x

   - **Mitigation**: Document missing ops, provide fallback/warning

2. **Memory Allocation**: Conversion needs temporary buffer

   - **Mitigation**: Check malloc success, graceful failure

3. **Index Calculation**: Complex logic prone to off-by-one errors
   - **Mitigation**: Extensive unit tests, debug logging

### Medium Risk ⚡

4. **Preset Format**: May be too different to convert

   - **Mitigation**: Accept preset loss, focus on network/operators

5. **Parameter Values**: Scalers may have changed
   - **Mitigation**: Document parameter differences, use defaults

### Low Risk ✓

6. **Module Loading**: Already has compatibility for old names
7. **Basic Structure**: Binary format is documented

---

## Dependencies

### Before Starting

- [ ] Access to 0.7.1 firmware and scenes
- [ ] Access to 0.8.1 firmware and scenes
- [ ] Development environment set up
- [ ] Ability to test on hardware or simulator

### Blocked By

- None - can start immediately

### Blocks

- **CDC Grid Testing**: Cannot test CDC changes with 0.7.1 grid scenes
- **Community Testing**: Cannot ask users to test without migration
- **Full Release**: Cannot release 1.0 without migration path

---

## Next Steps

### Immediate Actions (This Week)

1. **CRITICAL PRIORITY: Identify ALL Operators That Gained Outputs**

   This is the MOST CRITICAL step - Yann Copier indicated SCREEN and BIGNUM are just examples.

   ```bash
   # Create script: scripts/find_output_changes.sh

   #!/bin/bash
   # Find all operators that changed output counts between 0.7.1 and 0.8.x

   echo "Comparing operator outputs: 0.7.1 vs 0.8.x"

   # Save current branch
   current_branch=$(git branch --show-current)

   # Checkout 0.7.1 (find the exact commit/tag)
   git checkout <0.7.1-commit>

   # Extract output counts from 0.7.1
   for file in apps/bees/src/ops/op_*.c; do
     if [ -f "$file" ]; then
       opname=$(basename "$file" .c | sed 's/op_//')
       outputs=$(grep -h "->numOutputs.*=" "$file" | head -1 | grep -o '[0-9]\+' || echo "0")
       echo "$opname:$outputs"
     fi
   done | sort > /tmp/outputs_v07.txt

   # Checkout 0.8.x
   git checkout dev

   # Extract output counts from 0.8.x
   for file in apps/bees/src/ops/op_*.c; do
     if [ -f "$file" ]; then
       opname=$(basename "$file" .c | sed 's/op_//')
       outputs=$(grep -h "->numOutputs.*=" "$file" | head -1 | grep -o '[0-9]\+' || echo "0")
       echo "$opname:$outputs"
     fi
   done | sort > /tmp/outputs_v08.txt

   # Compare and show differences
   echo ""
   echo "=== OPERATORS WITH OUTPUT COUNT CHANGES ==="
   join -t: /tmp/outputs_v07.txt /tmp/outputs_v08.txt | \
     awk -F: '$2 != $3 { print $1 ": " $2 " -> " $3 " (+" ($3-$2) " outputs)" }'

   # Restore original branch
   git checkout "$current_branch"
   ```

   **Run this script FIRST** - we need the complete list before proceeding.

2. **Document Output Changes in Spreadsheet**

   - Create `OPERATOR_OUTPUT_CHANGES.xlsx`
   - Columns: Operator Name, v0.7.1 Outputs, v0.8.x Outputs, Change, Notes
   - Mark SCREEN and BIGNUM as confirmed
   - Fill in all other operators from script output

3. **Inventory 0.7.1 Scenes**

   ```bash
   ls -la utils/release/scenes-0.7.1/
   ```

4. **Create Test Script**

   ```bash
   # Script to attempt loading each 0.7.1 scene
   # Log results: success/fail/crash
   ```

5. **Generate Operator ID Map**

   - Compare `op.h` from 0.7.1 vs 0.8.1 branches
   - Document in spreadsheet
   - Create C header with mapping

6. **Implement Version Detection Test**
   ```c
   // Add debug logging to confirm version detection works
   printf("Scene version: %d.%d.%d\n", maj, min, rev);
   printf("needsConnectionRemapping: %d\n", needsConnectionRemapping);
   ```

### Week 1 Goals

- [ ] Complete operator ID mapping table
- [ ] **CRITICAL: Generate operator output count mapping** (SCREEN, BIGNUM, others)
- [ ] Document scene format differences
- [ ] Create `scene_convert.c` skeleton
- [ ] Implement basic operator ID conversion

### Week 2 Goals

- [ ] Implement connection index remapping
- [ ] Integrate into scene.c
- [ ] Test with simplest 0.7.1 scene (e.g., default.scn)
- [ ] Debug and fix crashes

---

## Resources

### Code References

- `apps/bees/src/scene.c` - Scene loading logic
- `apps/bees/src/scene.h` - Scene structures, needsConnectionRemapping
- `apps/bees/src/net.c` - Network unpickling
- `apps/bees/src/op.h` - Operator ID enum
- `apps/bees/src/preset.c` - Preset system

### Documentation

- `development/docs/analysis/bees-0.8-analysis.md` - What changed
- `development/docs/beekeep-development-plan.md` - Scene migration plan
- Lines forum: 0.8.1 release thread (January 2018)

### Test Scenes

- `utils/release/scenes-0.7.1/*.scn` - Official 0.7.1 scenes
- Lines forum: Community-shared 0.7.x scenes

---

## Appendix A: Operator ID Research

### How to Generate Mapping

1. **Checkout 0.7.1 code**:

   ```bash
   git checkout <0.7.1-tag>
   grep -n "eOp" apps/bees/src/op.h > op_ids_07.txt
   ```

2. **Checkout 0.8.1 code**:

   ```bash
   git checkout dev  # 0.8.x is on dev branch
   grep -n "eOp" apps/bees/src/op.h > op_ids_08.txt
   ```

3. **Compare and map**:

   - Create spreadsheet with side-by-side comparison
   - Note additions, removals, ID shifts
   - Generate C array mapping

4. **Generate output count mapping** (CRITICAL!):

   ```bash
   # For each operator in both versions, check op_*.c files
   # Look for numOutputs in op init functions

   # Example for SCREEN:
   # 0.7.x: ops/op_screen.c - check numOutputs
   # 0.8.x: ops/op_screen.c - check numOutputs
   # Record difference

   # Focus on operators mentioned by Yann Copier:
   # - SCREEN
   # - BIGNUM
   # - Any others that show output count changes
   ```

### Initial Observations

From 0.8.x analysis, these operators were **added**:

- White Whale (WW) - Grid sequencer
- Meadowphysics variants
- Additional list/route operators

These may have caused ID shifts for all operators after them.

### Operators That Gained Outputs (From Yann Copier)

**Confirmed in 0.8.x** (verified in source code):

- **SCREEN** - Has `numOutputs = 1` with "DUMMY" output string (was 0 in 0.7.x)
- **BIGNUM** - Has `numOutputs = 1` with "DUMMY" output string (was 0 in 0.7.x)

**Confirmed in 0.8.x** (unchanged - for comparison):

- **PRESET** - Has `numOutputs = 0` (still no outputs)

**CRITICAL: Need to Complete Full Audit**:

Yann Copier mentioned SCREEN and BIGNUM as examples, indicating there are MORE operators that gained outputs. Must systematically compare ALL operators between 0.7.1 and 0.8.x:

**Systematic Audit Required** (create checklist):

1. **Display/UI Operators** (HIGH PRIORITY - likely candidates):

   - [x] SCREEN ✓ (confirmed: gained 1 dummy output)
   - [x] BIGNUM ✓ (confirmed: gained 1 dummy output)
   - [ ] CPU (check if it gained output)
   - [ ] Any other UI/display operators

2. **I/O Operators** (check these):

   - [ ] SERIAL
   - [ ] HID_WORD
   - [ ] PARAM
   - [ ] Any operator that interacts with hardware but doesn't generate data values

3. **Utility Operators** (check these):

   - [x] PRESET (confirmed: still 0 outputs)
   - [ ] Any pure-control operators

4. **All Other Operators** (systematic check needed):
   - [ ] Complete audit of every operator in both versions

**Method to Identify (SCRIPT THIS)**:

```bash
#!/bin/bash
# Script to find all operators that gained outputs

echo "=== Comparing Operator Outputs: 0.7.1 vs 0.8.x ==="
echo ""

# Checkout 0.7.1
git checkout <0.7.1-tag-or-commit>
echo "Scanning 0.7.1 operators..."
for file in apps/bees/src/ops/op_*.c; do
  opname=$(basename "$file" .c | sed 's/op_//')
  outputs=$(grep -h "numOutputs.*=" "$file" | head -1 | grep -o '[0-9]\+' || echo "?")
  echo "v0.7.1 $opname: $outputs outputs"
done > /tmp/outputs_07.txt

# Checkout 0.8.x
git checkout dev
echo "Scanning 0.8.x operators..."
for file in apps/bees/src/ops/op_*.c; do
  opname=$(basename "$file" .c | sed 's/op_//')
  outputs=$(grep -h "numOutputs.*=" "$file" | head -1 | grep -o '[0-9]\+' || echo "?")
  echo "v0.8.x $opname: $outputs outputs"
done > /tmp/outputs_08.txt

# Compare and find differences
echo ""
echo "=== OPERATORS THAT CHANGED OUTPUT COUNTS ==="
diff /tmp/outputs_07.txt /tmp/outputs_08.txt | grep "^[<>]" | sort
```

**Why Dummy Outputs Were Added**:

- Likely for architecture consistency
- Possibly all operators expected to have at least 1 output
- Simplifies internal routing logic
- BUT: Breaks scene compatibility silently (worst kind of breakage)

**Documentation Format Needed**:

```c
// OPERATOR_OUTPUT_CHANGES.h
typedef struct {
  op_id_t op_id_v07;
  op_id_t op_id_v08;
  const char* op_name;
  u8 outputs_v07;
  u8 outputs_v08;
  u8 outputs_added;
  const char* notes;
} OpOutputChange;

const OpOutputChange g_opOutputChanges[] = {
  { eOpScreen, eOpScreen, "SCREEN", 0, 1, 1, "Gained DUMMY output" },
  { eOpBignum, eOpBignum, "BIGNUM", 0, 1, 1, "Gained DUMMY output" },
  // ... COMPLETE LIST NEEDED - this is CRITICAL for correct conversion
};
```

---

## Appendix B: Connection Remapping Algorithm

### Challenge

Connections are stored as flat indices into combined input array:

```
[Sys Inputs | Op1 Inputs | Op2 Inputs | ... | Parameters]
 0-3         4-7          8-11         ...  X-Y
```

When an operator is inserted/removed in 0.8.x, all indices shift.

### Solution Approach

Build index translation table during operator conversion:

```c
typedef struct {
  u32 op_idx_old;        // Operator index in 0.7.x
  u32 op_idx_new;        // Operator index in 0.8.x
  u32 first_in_old;      // First input index in 0.7.x
  u32 first_in_new;      // First input index in 0.8.x
  u8  num_ins;           // Number of inputs
} OpIndexMap;

OpIndexMap opMap[MAX_OPS];
```

As each operator is converted, record its old/new indices. Then when converting connections, look up the mapping:

```c
u32 convert_connection_target(u32 old_target) {
  // Find which operator this target belongs to in 0.7.x
  for (int i = 0; i < numOpsConverted; i++) {
    if (old_target >= opMap[i].first_in_old &&
        old_target < opMap[i].first_in_old + opMap[i].num_ins) {
      // Found the operator
      u32 input_offset = old_target - opMap[i].first_in_old;
      return opMap[i].first_in_new + input_offset;
    }
  }
  return -1; // Invalid
}
```

---

## Document History

- **2026-01-10**: Initial strategy document created
- **Status**: Draft - needs review and approval before implementation

---

**Next Document**: `OPERATOR_ID_MAPPING.md` - Complete operator mapping table
