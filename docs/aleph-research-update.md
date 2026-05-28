# Aleph Research Update — Output Shift Table Verified

## Finding: OPERATOR_OUTPUT_CHANGES.h is CORRECT

The IDs in the output shift table (7, 13, 16, 17) are NOT enum values — they're
conversion-specific IDs returned by `scene_convert_op_id()`. This is a valid and
consistent design.

### Verified Mapping:
- ENC old=5 → new=7 → get_output_shift(7, 0) returns 1 ✓
- TOG old=11 → new=13 → get_output_shift(13, 0) returns 1 ✓
- SPLIT old=14 → new=16 → get_output_shift(16, 0) returns 1 ✓
- SPLIT4 old=15 → new=17 → get_output_shift(17, 0) returns 1 ✓

### Conversion Logic is Sound:
1. Pass 1: Remap operator type IDs using `remap_op_id()`
2. Pass 2: Shift network output indices using `get_output_shift()` with new IDs

## What I Can Do Autonomously

1. **Periodic scene testing** — Load all 12 scenes, verify no crashes
2. **Network topology dump** — For each scene, show operator connections
3. **Identify scenes using shifted operators** — Find which scenes use ENC/SPLIT/SPLIT4/TOG
4. **Report anomalies** — Any scene that fails to load or has invalid connections

## What I Need From Brett

- If a scene loads but operators behave incorrectly, I need expected vs actual behavior
- For grid interaction testing, need to know which scenes use monome grid ops
- For preset recall, need to know if presets should restore specific values

## Next Actions (Autonomous)

1. Create network topology dumper for loaded scenes
2. Identify which of the 12 scenes use shifted-output operators
3. Verify their connections are valid post-conversion
4. Set up cron job for periodic regression testing
