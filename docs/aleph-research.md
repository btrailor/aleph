# Aleph 0.7.1 → 0.8.x Scene Migration Research

## Research Date: 2026-05-27
## Sources: git history, GitHub issues #303, PR #301, operator source code

---

## Key Findings

### 1. Operators That Gained REAL Outputs (Output Index Shifts)

These operators gained functional outputs that shift existing outputs forward.
Network connections TO these outputs need remapping.

| Operator | Change | Current Enum |
|----------|--------|-------------|
| **ENC** | Added DELTA at index 0 → VAL moved to index 1 | eOpEnc=1 |
| **SPLIT** | Added pass-through at index 0 → A/B moved to 1/2 | eOpSplit=12 |
| **SPLIT4** | Added pass-through at index 0 → A/B/C/D moved to 1/2/3/4 | eOpSplit4=28 |
| **TOG** | Added current-state at index 0 → VAL moved to index 1 | eOpTog=10 |

**Note:** The `OPERATOR_OUTPUT_CHANGES.h` table uses IDs that DON'T match current enum values.
The table has: `{ 7, 0, 1 }` for ENC, but `eOpEnc = 1`. This needs investigation.

### 2. Operators That Gained DUMMY Outputs (0 → 1 output)

These operators went from 0 outputs to 1 DUMMY output. This does NOT affect
network connections (no outputs to connect from in 0.7.1), but DOES affect:
- Memory layout (operator struct size increased)
- Preset recall (if presets store output values)

| Operator | Commit | Current Enum |
|----------|--------|-------------|
| SCREEN | 1e06d544 | eOpScreen=27 |
| BIGNUM | 1e06d544 | eOpBignum=26 |
| BARS | 1e06d544 | eOpBars=37 |
| BARS8 | 1e06d544 | eOpBars8=48 |
| MIDI_OUT_CC | 1e06d544 | eOpMidiOutCC=49 |
| MIDI_OUT_CLOCK | ffa5c1cc | eOpMidiOutClock=62 |
| MIDI_OUT_NOTE | 273ade8b | eOpMidiOutNote=32 |

**Note:** MIDI_OUT_NOTE also gained a PROG input (not just dummy output).

### 3. SERIAL — Special Case

SERIAL went from 0 outputs to 2 REAL outputs (ADDR, DATA) in commit 08e8c4d8.
This is NOT a dummy output — these are functional outputs.

### 4. Operators That DID NOT Change Output Count

Most operators kept the same outputs:
- ADD (1), MUL (1), DIV (1), SUB (1), ACCUM (2)
- GATE (1), METRO (1), DELAY (1), THRESH (2)
- SW (1), ROUTE (4), MOD (1), RANDOM (1)
- IS (4), LIFE (3), HISTORY (8)
- MIDI_NOTE (3), MIDI_CC (1)
- ADC (4), MONOME_GRID_RAW (3), etc.

---

## Scene Conversion Status

### What's Working
- ✅ Operator ID remapping (34 test assertions pass)
- ✅ All 12 legacy scenes load without crashes
- ✅ DSP modules load (aleph-lines, aleph-dsyn)

### What Might Be Broken
- ⚠️ Output index shifting — table IDs don't match enum values
- ⚠️ Network connections from shifted operators might point to wrong outputs
- ⚠️ MIDI_OUT_NOTE gained PROG input — pickles with 5 inputs now need 6

### What Needs Verification
- [ ] Load each scene and dump network topology
- [ ] Verify no connections to invalid output indices
- [ ] Verify operators with shifted outputs connect correctly

---

## Git History of Key Changes

```
1e06d544 — add dummy outputs to all zero-output BEES ops
           (SCREEN, BIGNUM, BARS, BARS8, MIDI_OUT_CC)

08e8c4d8 — add address/data output stubs to serial op
           (SERIAL went 0→2 real outputs)

273ade8b — add prog input & dummy output to midi_note_out
           (MIDI_OUT_NOTE gained PROG input + DUMMY output)

ffa5c1cc — fix midi_clk ops (zero ins/outs explode on deinit)
           (MIDI_OUT_CLOCK gained DUMMY output)
```

## Recommendations

1. **Fix OPERATOR_OUTPUT_CHANGES.h** — Verify table IDs match actual enum values
2. **Add network integrity test** — Load each scene, verify all connections valid
3. **Test grid interaction** — Verify scenes with monome ops work correctly
4. **Document known working scenes** — 12/12 load; need to verify runtime behavior

## Next Steps

1. Write network topology dumper (show all connections per scene)
2. Identify which scenes use ENC/SPLIT/SPLIT4/TOG
3. Verify their connections point to correct outputs post-conversion
4. Report any broken connections to Brett
