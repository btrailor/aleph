/* OPERATOR_OUTPUT_CHANGES.h
 * apps/bees/src
 *
 * Operator output count changes for 0.7.1 → 0.8.x scene migration.
 *
 * Background
 * ==========
 * Some operators gained additional outputs in 0.8.x.  A network
 * connection stores (src_op, src_output, dst_op, dst_input).  When
 * loading a 0.7.1 scene, the stored src_output index is relative to
 * the OLD operator definition.  scene_convert.c must shift those
 * indices if the 0.8.x operator inserted new outputs *before* the
 * 0.7.1 outputs.
 *
 * Table format
 * ============
 * Each entry: { op_new_id, first_shifted_output, shift_amount }
 *   op_new_id           : operator ID in 0.8.x numbering (post remapping)
 *   first_shifted_output: the lowest OLD output index that must be shifted
 *   shift_amount        : how many positions to add to that index
 *
 * If an operator's old output index is >= first_shifted_output, the
 * converted index = old_index + shift_amount.
 * If old_index < first_shifted_output, the index is unchanged.
 *
 * Entries with op_new_id == -1 terminate the table.
 *
 * Example: ENC (new_id=7) gained a "delta" output at index 0 in 0.8.x.
 * The 0.7.1 scene stored the single output as index 0 (meaning the
 * absolute encoder value).  In 0.8.x that output moved to index 1.
 * So: { 7, 0, 1 } — for ENC, all outputs from index 0 upward shift +1.
 *
 * Example: SPLIT (new_id=16) gained a "pass" output at index 0.
 * Outputs that were 0,1,2,3 in 0.7.1 become 1,2,3,4 in 0.8.x.
 * So: { 16, 0, 1 }.
 */

#ifndef _OPERATOR_OUTPUT_CHANGES_H_
#define _OPERATOR_OUTPUT_CHANGES_H_

typedef struct {
    int op_new_id;
    int first_shifted_output;
    int shift_amount;
} op_output_shift_t;

static const op_output_shift_t kOpOutputShift_071_to_08x[] = {
    /* op_new_id  first_shifted  shift  — operator / reason */
    {  7,         0,             1  }, /* ENC:   added raw-delta out at 0    */
    {  16,        0,             1  }, /* SPLIT: added pass-through out at 0 */
    {  17,        0,             1  }, /* SPLIT4: same pattern               */
    {  13,        0,             1  }, /* TOG:   added current-state out at 0*/
    { -1,         0,             0  }  /* sentinel                           */
};

#endif /* _OPERATOR_OUTPUT_CHANGES_H_ */
