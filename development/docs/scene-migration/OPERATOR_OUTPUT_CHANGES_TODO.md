# Operator Output Changes - Critical Action Items

## Priority: CRITICAL 🔴

Yann Copier's warning on Lines forum:

> "All operators that didn't have an output before (SCREEN, BIGNUM for instance) have now a dummy output, which can't be used for anything but will for sure break scene compatibility if used before v0.8 and transferred directly."

## The Problem

SCREEN and BIGNUM are **examples** - there are likely MORE operators that gained outputs. We need to identify ALL of them.

## Why This Breaks Scenes Silently

When an operator gains outputs, all output indices after it shift:

```
0.7.1 Scene:
[OP_A outs: 0-2] [SCREEN: no outputs] [OP_B outs: 3-5]

0.8.x Same Scene:
[OP_A outs: 0-2] [SCREEN out: 3] [OP_B outs: 4-6]
                                ↑ NEW

Connection to old index 3 (OP_B:out0) now points to SCREEN:out0 ❌
```

This is a **silent failure** - scene loads, connections appear valid, but patch is completely broken.

## Action Required

### Step 1: Run the Analysis Script

```bash
cd /Users/brettgershon/Documents/01_Projects/02_signals/Audio_Development/aleph
./development/scripts/find_output_changes.sh
```

This script will:
1. Checkout 0.7.1 code
2. Extract output counts from all operators
3. Checkout 0.8.x code  
4. Extract output counts from all operators
5. Compare and highlight changes

### Step 2: Document Results

Create file: `OPERATOR_OUTPUT_CHANGES.h`

```c
typedef struct {
  op_id_t op_id_v07;
  op_id_t op_id_v08;
  const char* op_name;
  u8 outputs_v07;
  u8 outputs_v08;
  u8 outputs_gained;
} OpOutputChange;

// Generated from script output
const OpOutputChange g_opOutputChanges[] = {
  { eOpScreen, eOpScreen, "SCREEN", 0, 1, 1 },
  { eOpBignum, eOpBignum, "BIGNUM", 0, 1, 1 },
  // ... ADD ALL FROM SCRIPT OUTPUT
};

const u32 g_opOutputChangesCount = sizeof(g_opOutputChanges) / sizeof(OpOutputChange);
```

### Step 3: Update Scene Migration Code

The conversion logic MUST:
1. Track cumulative output additions as operators are processed
2. Adjust ALL output indices by the cumulative shift
3. Apply operator-by-operator, not globally

See `SCENE_MIGRATION_STRATEGY.md` for implementation details.

## Verification Needed

After running the script, manually verify any "unknown" entries by:

```bash
# For each unknown operator
git checkout <0.7.1-ref>
grep -A5 "numOutputs" apps/bees/src/ops/op_<name>.c

git checkout dev
grep -A5 "numOutputs" apps/bees/src/ops/op_<name>.c
```

## Expected Operators to Check

Based on Yann's comment, focus on:
- **Display operators**: SCREEN ✓, BIGNUM ✓, CPU(?), others
- **Control operators**: Anything that didn't produce output values
- **I/O operators**: SERIAL(?), HID_WORD(?), PARAM(?)
- **Utility operators**: Pure routing/control ops

## Timeline

- **Today**: Run script, get complete list
- **Tomorrow**: Document in OPERATOR_OUTPUT_CHANGES.h
- **This Week**: Implement output index adjustment in scene migration code

## Status

- [x] Script created: `find_output_changes.sh`
- [ ] Script executed with results
- [ ] Complete operator list documented
- [ ] Implementation in scene converter
- [ ] Testing with affected scenes

---

**DO NOT PROCEED** with scene migration implementation until this list is complete. Wrong output mappings = silent broken patches.
