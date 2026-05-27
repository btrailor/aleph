/* synthetic_071.h
 * utils/beekeep/test/fixtures
 *
 * Synthetic 0.7.1 scene fixture for migration tests.
 *
 * This fixture represents a minimal BEES 0.7.1 scene containing:
 *   Op 0: ADD     (type_id=0)  — identity mapped in 0.8.x
 *   Op 1: MUL     (type_id=1)  — identity mapped in 0.8.x
 *   Op 2: ENC     (type_id=5)  — remapped to 7 in 0.8.x, gains output
 *   Op 3: SPLIT   (type_id=14) — remapped to 16 in 0.8.x, gains output
 *   Op 4: TOG     (type_id=11) — remapped to 13 in 0.8.x, gains output
 *   Op 5: METRO   (type_id=8)  — remapped to 10 in 0.8.x, no output change
 *
 * Network connections (0.7.1 output indices):
 *   Net 0: ENC(op2) out=0 → ADD(op0) in=0
 *           After migration: ENC new_id=7, out shifts from 0 to 1 (ENC gained out at 0)
 *   Net 1: ADD(op0) out=0 → MUL(op1) in=0
 *           After migration: ADD unchanged (new_id=0, no shift)
 *   Net 2: SPLIT(op3) out=0 → MUL(op1) in=1
 *           After migration: SPLIT new_id=16, out shifts from 0 to 1
 *   Net 3: TOG(op4) out=0 → SPLIT(op3) in=0
 *           After migration: TOG new_id=13, out shifts from 0 to 1
 *   Net 4: METRO(op5) out=0 → TOG(op4) in=0
 *           After migration: METRO new_id=10, no output shift defined
 */

#ifndef _SYNTHETIC_071_H_
#define _SYNTHETIC_071_H_

#include "scene_convert.h"

/* Build the fixture into scene_data and return it initialised. */
static inline scene_data_t make_synthetic_071(void) {
    scene_data_t s;
    int i;

    /* Zero entire struct — clears params, unset nets, etc. */
    for (i = 0; i < (int)sizeof(s); i++) {
        ((char*)&s)[i] = 0;
    }

    s.version  = SCENE_VERSION_071;
    s.num_ops  = 6;
    s.num_nets = 5;

    /* Ops */
    s.ops[0].type_id = 0;   /* ADD   */
    s.ops[1].type_id = 1;   /* MUL   */
    s.ops[2].type_id = 5;   /* ENC   — 0.7.1 */
    s.ops[3].type_id = 14;  /* SPLIT — 0.7.1 */
    s.ops[4].type_id = 11;  /* TOG   — 0.7.1 */
    s.ops[5].type_id = 8;   /* METRO — 0.7.1 */

    /* Nets (0.7.1 output indices) */
    s.nets[0].src_op = 2;  s.nets[0].src_output = 0;  /* ENC out=0 */
    s.nets[0].dst_op = 0;  s.nets[0].dst_input  = 0;

    s.nets[1].src_op = 0;  s.nets[1].src_output = 0;  /* ADD out=0 */
    s.nets[1].dst_op = 1;  s.nets[1].dst_input  = 0;

    s.nets[2].src_op = 3;  s.nets[2].src_output = 0;  /* SPLIT out=0 */
    s.nets[2].dst_op = 1;  s.nets[2].dst_input  = 1;

    s.nets[3].src_op = 4;  s.nets[3].src_output = 0;  /* TOG out=0 */
    s.nets[3].dst_op = 3;  s.nets[3].dst_input  = 0;

    s.nets[4].src_op = 5;  s.nets[4].src_output = 0;  /* METRO out=0 */
    s.nets[4].dst_op = 4;  s.nets[4].dst_input  = 0;

    return s;
}

/* ==========================================================================
 * Expected values after migration
 * ========================================================================== */

/* Expected type_ids after migration */
#define EXPECT_OP0_TYPE 0   /* ADD:   unchanged      */
#define EXPECT_OP1_TYPE 1   /* MUL:   unchanged      */
#define EXPECT_OP2_TYPE 7   /* ENC:   5 → 7          */
#define EXPECT_OP3_TYPE 16  /* SPLIT: 14 → 16        */
#define EXPECT_OP4_TYPE 13  /* TOG:   11 → 13        */
#define EXPECT_OP5_TYPE 10  /* METRO: 8  → 10        */

/* Expected src_output indices after migration */
#define EXPECT_NET0_SRC_OUTPUT 1  /* ENC out=0 → out=1 (ENC shifted +1) */
#define EXPECT_NET1_SRC_OUTPUT 0  /* ADD out=0 → out=0 (no shift)        */
#define EXPECT_NET2_SRC_OUTPUT 1  /* SPLIT out=0 → out=1 (SPLIT shifted +1) */
#define EXPECT_NET3_SRC_OUTPUT 1  /* TOG out=0 → out=1 (TOG shifted +1)  */
#define EXPECT_NET4_SRC_OUTPUT 0  /* METRO out=0 → out=0 (no shift)      */

/* Expected version after migration */
#define EXPECT_VERSION SCENE_VERSION_08X

#endif /* _SYNTHETIC_071_H_ */
