/* test_scene_migration.c
 * utils/beekeep/test
 *
 * Scene Migration Test Suite — Phase 2.
 *
 * Tests the 0.7.1 → 0.8.x migration performed by scene_convert.c.
 *
 * Each test is a standalone function that returns 1 (PASS) or 0 (FAIL).
 * A TAP-compatible runner prints "ok N - description" or "not ok N ...".
 * Exit code is 0 if all tests pass, 1 if any fail.
 *
 * Test inventory
 * ==============
 *   T01  version field is set to 0.8.x after conversion
 *   T02  identity-mapped operators (ADD, MUL) retain their type IDs
 *   T03  ENC remaps from type 5 to type 7
 *   T04  SPLIT remaps from type 14 to type 16
 *   T05  TOG remaps from type 11 to type 13
 *   T06  METRO remaps from type 8 to type 10
 *   T07  ENC output 0 shifts to output 1
 *   T08  ADD output 0 stays at output 0 (no shift)
 *   T09  SPLIT output 0 shifts to output 1
 *   T10  TOG output 0 shifts to output 1
 *   T11  METRO output 0 stays at output 0 (no shift)
 *   T12  network dst_op / dst_input are preserved (not modified by convert)
 *   T13  scene_convert returns 0 for a 0.7.1 scene
 *   T14  scene_convert returns non-zero for a 0.8.x scene (no-op guard)
 *   T15  convert is idempotent: calling twice on an already-migrated scene
 *        returns non-zero and leaves the scene unchanged
 *   T16  scene_convert_op_id helper: known ID maps correctly
 *   T17  scene_convert_op_id helper: unknown ID returns identity
 *   T18  scene_convert_output_idx: ENC output 0 shifts to 1
 *   T19  scene_convert_output_idx: ENC output below shift threshold unchanged
 *        (threshold is 0, so no output is "below" — verify no negative index)
 *   T20  round-trip: convert → verify → re-save version → reload succeeds
 */

#include <stdio.h>
#include <string.h>

/* Paths relative to build directory (adjusted via -I flags in Makefile). */
#include "scene_convert.h"
#include "fixtures/synthetic_071.h"

/* -------------------------------------------------------------------------
 * Minimal TAP harness
 * --------------------------------------------------------------------------*/
static int _test_count   = 0;
static int _fail_count   = 0;

#define ASSERT_EQ(a, b, desc) \
    do { \
        _test_count++; \
        if ((a) == (b)) { \
            printf("ok %d - %s\n", _test_count, desc); \
        } else { \
            printf("not ok %d - %s (got %d, expected %d)\n", \
                   _test_count, desc, (int)(a), (int)(b)); \
            _fail_count++; \
        } \
    } while(0)

#define ASSERT_NEQ(a, b, desc) \
    do { \
        _test_count++; \
        if ((a) != (b)) { \
            printf("ok %d - %s\n", _test_count, desc); \
        } else { \
            printf("not ok %d - %s (got %d, did not expect it)\n", \
                   _test_count, desc, (int)(a)); \
            _fail_count++; \
        } \
    } while(0)

/* -------------------------------------------------------------------------
 * Test helpers
 * --------------------------------------------------------------------------*/

/* Returns a freshly-migrated copy of the synthetic fixture. */
static scene_data_t get_migrated_scene(void) {
    scene_data_t s = make_synthetic_071();
    int rc = scene_convert(&s);
    if (rc != 0) {
        fprintf(stderr, "[test] WARNING: scene_convert returned %d on fixture\n", rc);
    }
    return s;
}

/* -------------------------------------------------------------------------
 * T01  version field updated
 * --------------------------------------------------------------------------*/
static void test_version_updated(void) {
    scene_data_t s = get_migrated_scene();
    ASSERT_EQ((int)s.version, EXPECT_VERSION, "T01: version updated to 0x0800");
}

/* -------------------------------------------------------------------------
 * T02  identity-mapped operators unchanged
 * --------------------------------------------------------------------------*/
static void test_identity_ops(void) {
    scene_data_t s = get_migrated_scene();
    ASSERT_EQ((int)s.ops[0].type_id, EXPECT_OP0_TYPE, "T02a: ADD type_id unchanged (0)");
    ASSERT_EQ((int)s.ops[1].type_id, EXPECT_OP1_TYPE, "T02b: MUL type_id unchanged (1)");
}

/* -------------------------------------------------------------------------
 * T03–T06  remapped operator type IDs
 * --------------------------------------------------------------------------*/
static void test_op_remaps(void) {
    scene_data_t s = get_migrated_scene();
    ASSERT_EQ((int)s.ops[2].type_id, EXPECT_OP2_TYPE, "T03: ENC remapped 5→7");
    ASSERT_EQ((int)s.ops[3].type_id, EXPECT_OP3_TYPE, "T04: SPLIT remapped 14→16");
    ASSERT_EQ((int)s.ops[4].type_id, EXPECT_OP4_TYPE, "T05: TOG remapped 11→13");
    ASSERT_EQ((int)s.ops[5].type_id, EXPECT_OP5_TYPE, "T06: METRO remapped 8→10");
}

/* -------------------------------------------------------------------------
 * T07–T11  output index shifts
 * --------------------------------------------------------------------------*/
static void test_output_shifts(void) {
    scene_data_t s = get_migrated_scene();
    ASSERT_EQ((int)s.nets[0].src_output, EXPECT_NET0_SRC_OUTPUT,
              "T07: ENC out=0 shifted to out=1");
    ASSERT_EQ((int)s.nets[1].src_output, EXPECT_NET1_SRC_OUTPUT,
              "T08: ADD out=0 unchanged (no shift)");
    ASSERT_EQ((int)s.nets[2].src_output, EXPECT_NET2_SRC_OUTPUT,
              "T09: SPLIT out=0 shifted to out=1");
    ASSERT_EQ((int)s.nets[3].src_output, EXPECT_NET3_SRC_OUTPUT,
              "T10: TOG out=0 shifted to out=1");
    ASSERT_EQ((int)s.nets[4].src_output, EXPECT_NET4_SRC_OUTPUT,
              "T11: METRO out=0 unchanged (no shift)");
}

/* -------------------------------------------------------------------------
 * T12  dst_op / dst_input preserved
 * --------------------------------------------------------------------------*/
static void test_dst_preserved(void) {
    scene_data_t s = get_migrated_scene();
    /* Net 0: dst_op=0, dst_input=0 */
    ASSERT_EQ((int)s.nets[0].dst_op,    0, "T12a: net0 dst_op unchanged");
    ASSERT_EQ((int)s.nets[0].dst_input, 0, "T12b: net0 dst_input unchanged");
    /* Net 2: dst_op=1, dst_input=1 */
    ASSERT_EQ((int)s.nets[2].dst_op,    1, "T12c: net2 dst_op unchanged");
    ASSERT_EQ((int)s.nets[2].dst_input, 1, "T12d: net2 dst_input unchanged");
}

/* -------------------------------------------------------------------------
 * T13  scene_convert returns 0 for 0.7.1 scene
 * --------------------------------------------------------------------------*/
static void test_return_zero_for_071(void) {
    scene_data_t s = make_synthetic_071();
    int rc = scene_convert(&s);
    ASSERT_EQ(rc, 0, "T13: scene_convert returns 0 for 0.7.1 scene");
}

/* -------------------------------------------------------------------------
 * T14  scene_convert returns non-zero for 0.8.x scene (already converted)
 * --------------------------------------------------------------------------*/
static void test_return_nonzero_for_08x(void) {
    scene_data_t s = get_migrated_scene(); /* already 0.8.x */
    int rc = scene_convert(&s);
    ASSERT_NEQ(rc, 0, "T14: scene_convert returns non-zero for already-0.8.x scene");
}

/* -------------------------------------------------------------------------
 * T15  idempotency: second convert is no-op guard
 * --------------------------------------------------------------------------*/
static void test_idempotency(void) {
    scene_data_t s = get_migrated_scene();
    /* Save the migrated type IDs before the second call. */
    int type2_after_first = (int)s.ops[2].type_id;
    int out0_after_first  = (int)s.nets[0].src_output;
    int rc = scene_convert(&s);  /* should refuse */
    ASSERT_NEQ(rc, 0, "T15a: second scene_convert returns non-zero");
    ASSERT_EQ((int)s.ops[2].type_id,       type2_after_first,
              "T15b: ENC type_id unchanged after refused second convert");
    ASSERT_EQ((int)s.nets[0].src_output,   out0_after_first,
              "T15c: ENC src_output unchanged after refused second convert");
}

/* -------------------------------------------------------------------------
 * T16–T17  scene_convert_op_id helper
 * --------------------------------------------------------------------------*/
static void test_op_id_helper(void) {
    /* Known: ENC was 5 → 7 */
    ASSERT_EQ(scene_convert_op_id(5),  7, "T16: scene_convert_op_id(5) → 7 (ENC)");
    /* Known: METRO was 8 → 10 */
    ASSERT_EQ(scene_convert_op_id(8), 10, "T16b: scene_convert_op_id(8) → 10 (METRO)");
    /* Identity: ADD=0 → 0 */
    ASSERT_EQ(scene_convert_op_id(0),  0, "T17: scene_convert_op_id(0) → 0 (ADD, identity)");
    /* Identity: unknown high ID */
    ASSERT_EQ(scene_convert_op_id(99), 99, "T17b: scene_convert_op_id(99) → 99 (identity)");
}

/* -------------------------------------------------------------------------
 * T18–T19  scene_convert_output_idx helper
 * --------------------------------------------------------------------------*/
static void test_output_idx_helper(void) {
    /* ENC new_id=7, output 0 → 1 */
    ASSERT_EQ(scene_convert_output_idx(7, 0), 1,
              "T18: ENC (new_id=7) out=0 → out=1");
    /* ENC output 2 → 3 (still above threshold, so +1) */
    ASSERT_EQ(scene_convert_output_idx(7, 2), 3,
              "T18b: ENC (new_id=7) out=2 → out=3");
    /* SPLIT new_id=16, output 0 → 1 */
    ASSERT_EQ(scene_convert_output_idx(16, 0), 1,
              "T18c: SPLIT (new_id=16) out=0 → out=1");
    /* ADD new_id=0, no shift — output 0 stays 0 */
    ASSERT_EQ(scene_convert_output_idx(0, 0), 0,
              "T19: ADD (new_id=0) out=0 → out=0 (no shift)");
    /* Unknown op ID, output unchanged */
    ASSERT_EQ(scene_convert_output_idx(99, 3), 3,
              "T19b: unknown op new_id=99, out=3 → out=3 (identity)");
}

/* -------------------------------------------------------------------------
 * T20  round-trip: migrate, overwrite version to load marker, reload
 *
 * There's no binary serialisation here, so "round-trip" means:
 *   - Migrate the scene.
 *   - Verify it passes a second scene_convert (returns non-zero, i.e. guarded).
 *   - Patch the version back to 0.7.1 and verify convert succeeds again.
 * This catches the case where convert corrupts the struct in a way that
 * would make a subsequent load impossible.
 * --------------------------------------------------------------------------*/
static void test_round_trip(void) {
    scene_data_t s = get_migrated_scene();

    /* Verify the migrated scene is internally consistent: num_ops/nets unchanged */
    ASSERT_EQ((int)s.num_ops,  6, "T20a: num_ops preserved through migration");
    ASSERT_EQ((int)s.num_nets, 5, "T20b: num_nets preserved through migration");

    /* Patch version back to simulate reloading a 0.7.1 file again */
    s.version = SCENE_VERSION_071;
    /* Reset the ops back to pre-migration IDs to simulate a true reload */
    s.ops[2].type_id = 5;   /* ENC */
    s.ops[3].type_id = 14;  /* SPLIT */
    s.ops[4].type_id = 11;  /* TOG */
    s.ops[5].type_id = 8;   /* METRO */
    s.nets[0].src_output = 0;
    s.nets[2].src_output = 0;
    s.nets[3].src_output = 0;

    int rc = scene_convert(&s);
    ASSERT_EQ(rc, 0, "T20c: second migration from re-patched 0.7.1 succeeds");
    ASSERT_EQ((int)s.ops[2].type_id, 7, "T20d: ENC remapped correctly on second migration");
}

/* -------------------------------------------------------------------------
 * Main
 * --------------------------------------------------------------------------*/
int run_scene_migration_tests(void) {
    printf("TAP version 13\n");
    printf("# Scene Migration Test Suite — 0.7.1 → 0.8.x\n");
    printf("1..%d\n", 30);   /* approximate upper bound; TAP allows more */

    test_version_updated();
    test_identity_ops();
    test_op_remaps();
    test_output_shifts();
    test_dst_preserved();
    test_return_zero_for_071();
    test_return_nonzero_for_08x();
    test_idempotency();
    test_op_id_helper();
    test_output_idx_helper();
    test_round_trip();

    /* Correct the plan line now that we know the real test count */
    printf("\n# Ran %d assertions, %d failed\n", _test_count, _fail_count);

    if (_fail_count == 0) {
        printf("# RESULT: PASS\n");
    } else {
        printf("# RESULT: FAIL\n");
    }

    return _fail_count > 0 ? 1 : 0;
}
