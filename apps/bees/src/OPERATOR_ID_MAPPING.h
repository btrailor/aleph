/* OPERATOR_ID_MAPPING.h
 * apps/bees/src
 *
 * Operator ID remapping table for 0.7.1 → 0.8.x scene migration.
 *
 * Background
 * ==========
 * BEES 0.8.x reorganised operator IDs when several new operators were
 * inserted into the operator list.  A scene file stores operator type
 * IDs as integers.  When loading a 0.7.1 scene, scene_convert.c must
 * remap each operator's stored ID to its 0.8.x equivalent before
 * passing it to the network allocator.
 *
 * Table format
 * ============
 * Each entry is { old_id, new_id }.
 * Entries with old_id == -1 terminate the table.
 * If an old_id does not appear in the table, it is assumed to be
 * unchanged (identity mapping).
 *
 * Version history
 * ===============
 * 0.7.1 → 0.8.0: ADD(0)→0, MUL(1)→1, DIV(2)→2, SUB(3)→3, ACCUM(4)→4
 *   ENC was 5 in 0.7.1, became 7 in 0.8.x (BITS and LOGIC inserted at 5,6)
 *   GATE was 6 → 8
 *   TIMER was 7 → 9
 *   METRO was 8 → 10
 *   DELAY was 9 → 11
 *   THRESH was 10 → 12
 *   TOG was 11 → 13
 *   SW was 12 → 14
 *   ROUTE was 13 → 15
 *   SPLIT was 14 → 16
 *   SPLIT4 was 15 → 17
 *   LIST2 was 16 → 18
 *   LIST8 was 17 → 19
 *   LIST16 was 18 → 20
 *   HISTORY was 19 → 21
 *   RANDOM was 20 → 22
 *   LIFE was 21 → 23
 *   IS was 22 → 24
 *   BIGNUM was 23 → 25
 *   MIDI_NOTE was 24 → 26
 *   MIDI_CC was 25 → 27
 *   MIDI_OUT_NOTE was 26 → 28
 *   SCREEN was 27 → 29
 *   PRESET was 28 → 30
 *   MONOME_GRID_RAW was 29 → 31
 *   ADC was 30 → 32
 *   MOD was 31 → 33
 *   (PARAM was 32 → 34 in 0.8.x where DSP param ops were added)
 */

#ifndef _OPERATOR_ID_MAPPING_H_
#define _OPERATOR_ID_MAPPING_H_

typedef struct {
    int old_id;
    int new_id;
} op_id_remap_t;

/* Sentinel-terminated table.  Identity mappings (ADD=0→0, MUL=1→1,
 * DIV=2→2, SUB=3→3, ACCUM=4→4) are omitted — scene_convert applies
 * the identity for any ID not found here. */
static const op_id_remap_t kOpIdRemap_071_to_08x[] = {
    /* old  new  — operator name */
    {  5,   7  }, /* ENC          */
    {  6,   8  }, /* GATE         */
    {  7,   9  }, /* TIMER        */
    {  8,  10  }, /* METRO        */
    {  9,  11  }, /* DELAY        */
    { 10,  12  }, /* THRESH       */
    { 11,  13  }, /* TOG          */
    { 12,  14  }, /* SW           */
    { 13,  15  }, /* ROUTE        */
    { 14,  16  }, /* SPLIT        */
    { 15,  17  }, /* SPLIT4       */
    { 16,  18  }, /* LIST2        */
    { 17,  19  }, /* LIST8        */
    { 18,  20  }, /* LIST16       */
    { 19,  21  }, /* HISTORY      */
    { 20,  22  }, /* RANDOM       */
    { 21,  23  }, /* LIFE         */
    { 22,  24  }, /* IS           */
    { 23,  25  }, /* BIGNUM       */
    { 24,  26  }, /* MIDI_NOTE    */
    { 25,  27  }, /* MIDI_CC      */
    { 26,  28  }, /* MIDI_OUT_NOTE*/
    { 27,  29  }, /* SCREEN       */
    { 28,  30  }, /* PRESET       */
    { 29,  31  }, /* MONOME_GRID_RAW */
    { 30,  32  }, /* ADC          */
    { 31,  33  }, /* MOD          */
    { 32,  34  }, /* PARAM        */
    { -1,  -1  }  /* sentinel     */
};

#endif /* _OPERATOR_ID_MAPPING_H_ */
