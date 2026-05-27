/* test_network_integrity.c
 * beekeep / test
 * aleph
 *
 * Network integrity tests — verify op creation, connection, deletion,
 * feedback loops, and scene save/reload consistency.
 *
 * Called from harness.c via run_network_integrity_tests().
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/* avr32_sim */
#include "app.h"
#include "events.h"
#include "event_types.h"

/* bees */
#include "net.h"
#include "scene.h"
#include "op.h"
#include "files.h"

/* -------------------------------------------------------
 * Shared state from harness.c
 * -------------------------------------------------------*/
extern int s_verbose;
extern char workingDir[256];

/* -------------------------------------------------------
 * Test infrastructure — these forward to the harness counters.
 * We re-declare the counters as extern so this TU can update them,
 * and re-use the same macros pattern inline here.
 * -------------------------------------------------------*/
extern int s_passed;
extern int s_failed;
extern int s_total;
extern const char* s_current_group;
extern const char* s_current_test;

/* Forward: record a JSON result entry */
typedef struct { char group[64]; char test[128]; char msg[256]; int passed; } json_result_t;
extern json_result_t s_json_results[];
extern int s_json_count;
#define MAX_JSON_RESULTS 256

static void ni_record_json(int passed, const char* msg) {
    if (s_json_count >= MAX_JSON_RESULTS) return;
    json_result_t* r = &s_json_results[s_json_count++];
    strncpy(r->group, s_current_group, sizeof(r->group) - 1);
    strncpy(r->test,  s_current_test,  sizeof(r->test)  - 1);
    strncpy(r->msg,   msg,             sizeof(r->msg)   - 1);
    r->passed = passed;
}

static void ni_result(int passed, const char* msg,
                      const char* file, int line) {
    s_total++;
    if (passed) {
        s_passed++;
        if (s_verbose)
            printf("    [PASS] %s  (%s:%d)\n", msg, file, line);
        else
            printf("  [PASS] %s\n", msg);
    } else {
        s_failed++;
        printf("  [FAIL] %s  (%s:%d)\n", msg, file, line);
    }
    ni_record_json(passed, msg);
}

/* -------------------------------------------------------
 * Local assertion macros (same semantics as harness.c)
 * -------------------------------------------------------*/
#define NI_ASSERT_TRUE(cond) \
    ni_result(!!(cond), "ASSERT_TRUE(" #cond ")", __FILE__, __LINE__)

#define NI_ASSERT_EQ(a, b) do { \
    long long _a = (long long)(a), _b = (long long)(b); \
    int _ok = (_a == _b); \
    if (!_ok) printf("  [FAIL] ASSERT_EQ(" #a ", " #b "): %lld != %lld  (%s:%d)\n", \
                     _a, _b, __FILE__, __LINE__); \
    ni_result(_ok, "ASSERT_EQ(" #a ", " #b ")", __FILE__, __LINE__); \
} while(0)

#define NI_ASSERT_NE(a, b) do { \
    long long _a = (long long)(a), _b = (long long)(b); \
    int _ok = (_a != _b); \
    if (!_ok) printf("  [FAIL] ASSERT_NE(" #a ", " #b "): both == %lld  (%s:%d)\n", \
                     _a, __FILE__, __LINE__); \
    ni_result(_ok, "ASSERT_NE(" #a ", " #b ")", __FILE__, __LINE__); \
} while(0)

#define NI_PASS(msg) ni_result(1, msg, __FILE__, __LINE__)
#define NI_FAIL(msg) ni_result(0, msg, __FILE__, __LINE__)

/* Group / test markers */
#define NI_TEST_GROUP(name) do { \
    s_current_group = (name); \
    printf("\n┌─ GROUP: %s\n", name); \
} while(0)

#define NI_TEST(name) do { \
    s_current_test = (name); \
    printf("│  TEST: %s\n", name); \
} while(0)

/* -------------------------------------------------------
 * Set workingDir (mirrors harness helper)
 * -------------------------------------------------------*/
static void ni_set_working_dir(const char* fixtures_dir) {
    strncpy(workingDir, fixtures_dir, 255);
    workingDir[255] = '\0';
    int len = (int)strlen(workingDir);
    if (len > 0 && workingDir[len-1] != '/' && len < 255) {
        workingDir[len]   = '/';
        workingDir[len+1] = '\0';
    }
}

/* -------------------------------------------------------
 * NI-1: Create two ops, connect them, verify connection exists
 * -------------------------------------------------------*/
static void ni_test_create_and_connect(void) {
    NI_TEST_GROUP("network_integrity");
    NI_TEST("create_and_connect");
    printf("\n[NI-1] create two ops, connect, verify connection\n");

    /* Add two arithmetic ops */
    s16 op_a = net_add_op(eOpAdd);
    s16 op_b = net_add_op(eOpMul);

    NI_ASSERT_NE(op_a, -1);
    NI_ASSERT_NE(op_b, -1);

    if (op_a < 0 || op_b < 0) {
        NI_FAIL("ni-1: could not add ops, skipping connection check");
        return;
    }

    /* Get output index for op_a output 0, input index for op_b input 0 */
    u16 out_idx = net_op_out_idx((u16)op_a, 0);
    u16 in_idx  = net_op_in_idx((u16)op_b, 0);

    printf("  [info] op_a(ADD)=%d out[%u] -> op_b(MUL)=%d in[%u]\n",
           op_a, out_idx, op_b, in_idx);

    /* Connect op_a output 0 to op_b input 0 */
    net_connect(out_idx, (s32)in_idx);

    /* Verify target is set */
    s16 target = net_get_target(out_idx);
    printf("  [info] net_get_target(out=%u) = %d  (expected ~%u)\n",
           out_idx, target, in_idx);

    NI_ASSERT_EQ(target, (s16)in_idx);
}

/* -------------------------------------------------------
 * NI-2: Delete an op, verify its connections are cleaned up
 * -------------------------------------------------------*/
static void ni_test_delete_cleans_connections(void) {
    NI_TEST_GROUP("network_integrity");
    NI_TEST("delete_op_cleans_connections");
    printf("\n[NI-2] delete op, verify connections removed\n");

    /* Add two ops and connect them */
    s16 op_a = net_add_op(eOpAdd);
    s16 op_b = net_add_op(eOpSub);

    if (op_a < 0 || op_b < 0) {
        NI_FAIL("ni-2: could not add ops, skipping");
        return;
    }

    u16 out_idx = net_op_out_idx((u16)op_a, 0);
    u16 in_idx  = net_op_in_idx((u16)op_b, 0);

    net_connect(out_idx, (s32)in_idx);

    /* Verify connection established */
    s16 target_before = net_get_target(out_idx);
    NI_ASSERT_EQ(target_before, (s16)in_idx);

    /* Delete op_b — connections to/from it should be cleaned up */
    u16 ops_before = net_num_ops();
    net_remove_op((u16)op_b);
    u16 ops_after = net_num_ops();

    printf("  [info] ops: %u -> %u after removal\n", ops_before, ops_after);
    NI_ASSERT_EQ(ops_after, ops_before - 1);

    /* The output from op_a should now be disconnected (target == -1) */
    s16 target_after = net_get_target(out_idx);
    printf("  [info] net_get_target(out=%u) after delete = %d  (expected -1)\n",
           out_idx, target_after);
    NI_ASSERT_EQ(target_after, -1);
}

/* -------------------------------------------------------
 * NI-3: Create a feedback loop (op A -> op B -> op A), verify it works
 *
 * BEES supports feedback loops in the network — cycles are legal.
 * This test verifies we can create A->B and B->A without crashing.
 * -------------------------------------------------------*/
static void ni_test_feedback_loop(void) {
    NI_TEST_GROUP("network_integrity");
    NI_TEST("feedback_loop");
    printf("\n[NI-3] feedback loop A->B->A — verify no crash\n");

    s16 op_a = net_add_op(eOpAdd);
    s16 op_b = net_add_op(eOpAdd);

    if (op_a < 0 || op_b < 0) {
        NI_FAIL("ni-3: could not add ops, skipping");
        return;
    }

    /* BEES ops typically have multiple inputs.  Use output 0 of each
     * and connect to a different input of the other to avoid trivially
     * hitting the same net node. */
    u16 out_a = net_op_out_idx((u16)op_a, 0);
    u16 in_b  = net_op_in_idx((u16)op_b, 0);

    u16 out_b = net_op_out_idx((u16)op_b, 0);
    u16 in_a  = net_op_in_idx((u16)op_a, 1);

    printf("  [info] A-out=%u -> B-in=%u\n", out_a, in_b);
    printf("  [info] B-out=%u -> A-in=%u\n", out_b, in_a);

    /* Connect A -> B */
    net_connect(out_a, (s32)in_b);
    s16 t_ab = net_get_target(out_a);
    NI_ASSERT_EQ(t_ab, (s16)in_b);

    /* Connect B -> A (feedback) */
    net_connect(out_b, (s32)in_a);
    s16 t_ba = net_get_target(out_b);
    NI_ASSERT_EQ(t_ba, (s16)in_a);

    /* Network must still be consistent */
    NI_ASSERT_TRUE(net_num_ops() > 0);
    NI_PASS("feedback loop: both directions connected without crash");
}

/* -------------------------------------------------------
 * NI-4: Save scene, reload, verify network identical
 * -------------------------------------------------------*/
static void ni_test_save_reload_identical(void) {
    char path[512];
    const char* fixtures_dir = "test/fixtures";

    NI_TEST_GROUP("scene_migration");
    NI_TEST("save_reload_identical");
    printf("\n[NI-4] save 3-op scene, reload, verify network identical\n");

    /* Build a small network: ADD -> MUL -> SUB */
    s16 op_add = net_add_op(eOpAdd);
    s16 op_mul = net_add_op(eOpMul);
    s16 op_sub = net_add_op(eOpSub);

    if (op_add < 0 || op_mul < 0 || op_sub < 0) {
        NI_FAIL("ni-4: could not add ops, skipping");
        return;
    }

    /* Connect ADD-out0 -> MUL-in0, MUL-out0 -> SUB-in0 */
    u16 out_add = net_op_out_idx((u16)op_add, 0);
    u16 in_mul  = net_op_in_idx((u16)op_mul, 0);
    u16 out_mul = net_op_out_idx((u16)op_mul, 0);
    u16 in_sub  = net_op_in_idx((u16)op_sub, 0);

    net_connect(out_add, (s32)in_mul);
    net_connect(out_mul, (s32)in_sub);

    u16 ops_before  = net_num_ops();
    s16 t_add_before = net_get_target(out_add);
    s16 t_mul_before = net_get_target(out_mul);

    printf("  [info] before save: ops=%u  t_add=%d  t_mul=%d\n",
           ops_before, t_add_before, t_mul_before);

    scene_set_name("ni_roundtrip");
    scene_set_module_name("NONE");

    snprintf(path, sizeof(path), "%s/ni_roundtrip.scn", fixtures_dir);
    ni_set_working_dir(fixtures_dir);
    files_store_scene_name(path);

    /* Verify file written */
    struct stat st;
    NI_ASSERT_TRUE(stat(path, &st) == 0 && st.st_size > 0);

    /* Reload */
    u8 ret = files_load_scene_name("ni_roundtrip.scn");
    NI_ASSERT_TRUE(ret == 0 || ret == 1);

    u16 ops_after   = net_num_ops();
    s16 t_add_after = net_get_target(out_add);
    s16 t_mul_after = net_get_target(out_mul);

    printf("  [info] after reload: ops=%u  t_add=%d  t_mul=%d\n",
           ops_after, t_add_after, t_mul_after);

    NI_ASSERT_EQ(ops_after, ops_before);
    NI_ASSERT_EQ(t_add_after, t_add_before);
    NI_ASSERT_EQ(t_mul_after, t_mul_before);
}

/* -------------------------------------------------------
 * Entry point — called from harness.c
 * -------------------------------------------------------*/
void run_network_integrity_tests(void) {
    printf("\n════════════════════════════════════════════════\n");
    printf("  Network Integrity Tests\n");
    printf("════════════════════════════════════════════════\n");

    ni_test_create_and_connect();
    ni_test_delete_cleans_connections();
    ni_test_feedback_loop();
    ni_test_save_reload_identical();
}
