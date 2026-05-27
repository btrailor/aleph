/* make_fixtures.c
 * beekeep / test
 * aleph
 *
 * Fixture generator: boots BEES, constructs minimal in-memory scenes,
 * and writes them as valid .scn binary files into test/fixtures/.
 *
 * Run: ./make_fixtures  (from utils/beekeep/)
 * Output: test/fixtures/empty.scn
 *         test/fixtures/two_op_network.scn
 *
 * Design notes:
 *   - Links against the same objects as beekeep-headless so the scene
 *     format is guaranteed to match what beekeep-headless loads.
 *   - Does NOT modify any BEES app logic; only calls the public API.
 *   - Writing a fixture = call app_init(), mutate the net, call
 *     scene_write_buf(), fwrite(sceneData, sizeof, 1, f).
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
#include "files.h"   /* files_store_scene_name */

/* workingDir is normally defined in ui_files.c (GTK) or main_headless.c.
 * In the fixture generator, we define it here. */
char workingDir[256] = "";

/* -------------------------------------------------------
 * Helpers
 * -------------------------------------------------------*/

static void ensure_dir(const char* path) {
    struct stat st;
    if (stat(path, &st) != 0) {
        /* Try to create it */
#ifdef _WIN32
        _mkdir(path);
#else
        mkdir(path, 0755);
#endif
    }
}

/* Reset network to a clean state without re-running full app_init.
 * We call app_init() once, then between fixtures we re-init just
 * the network. */
static void reset_net(void) {
    net_deinit();
    net_init();
    /* Clear scene name */
    scene_set_name("fixture");
    scene_set_module_name("NONE");
}

/* Write sceneData to <dir>/<name>.scn. Returns 0 on success. */
static int write_fixture(const char* dir, const char* name) {
    char path[512];
    FILE* f;
    size_t written;

    snprintf(path, sizeof(path), "%s/%s.scn", dir, name);
    f = fopen(path, "wb");
    if (!f) {
        fprintf(stderr, "[make_fixtures] ERROR: could not open %s for writing\n", path);
        return 1;
    }

    /* Serialise current BEES network state into sceneData blob */
    scene_write_buf();

    written = fwrite((const void*)sceneData, sizeof(sceneData_t), 1, f);
    fclose(f);

    if (written != 1) {
        fprintf(stderr, "[make_fixtures] ERROR: fwrite failed for %s\n", path);
        return 1;
    }

    printf("[make_fixtures] wrote %s  (%zu bytes)\n", path, sizeof(sceneData_t));
    return 0;
}

/* -------------------------------------------------------
 * Fixture: empty scene
 *   - No operators, no connections.
 *   - Module name "NONE".
 * -------------------------------------------------------*/
static int make_empty(const char* dir) {
    reset_net();
    scene_set_name("empty");
    return write_fixture(dir, "empty");
}

/* -------------------------------------------------------
 * Fixture: two_op_network
 *   - Two operators: eOpAdd (idx 0) and eOpMul (idx 1).
 *   - One connection: out[0] of ADD → in[0] of MUL.
 * -------------------------------------------------------*/
static int make_two_op_network(const char* dir) {
    s16 op0_idx, op1_idx;
    u16 out0_global, in1_global;

    reset_net();
    scene_set_name("two_op_network");
    scene_set_module_name("NONE");

    /* Add operators */
    op0_idx = net_add_op(eOpAdd);
    op1_idx = net_add_op(eOpMul);

    if (op0_idx < 0 || op1_idx < 0) {
        fprintf(stderr, "[make_fixtures] ERROR: net_add_op failed "
                "(op0=%d op1=%d)\n", op0_idx, op1_idx);
        return 1;
    }

    printf("[make_fixtures] two_op_network: op0=ADD@%d  op1=MUL@%d\n",
           op0_idx, op1_idx);
    printf("[make_fixtures]   numOps=%u  numOuts=%u  numIns=%u\n",
           net_num_ops(), net_num_outs(), net_num_ins());

    /* Connect first output of op0 to first input of op1.
     * net_op_out_idx(opIdx, localOutIdx) → global output index
     * net_op_in_idx(opIdx, localInIdx)  → global input index  */
    if (net_num_outs() > 0 && net_num_ins() > 0) {
        out0_global = net_op_out_idx((u16)op0_idx, 0);
        in1_global  = net_op_in_idx((u16)op1_idx,  0);
        net_connect(out0_global, in1_global);
        printf("[make_fixtures]   connected out[%u] → in[%u]\n",
               out0_global, in1_global);
    } else {
        fprintf(stderr, "[make_fixtures] WARNING: no outs/ins to connect\n");
    }

    return write_fixture(dir, "two_op_network");
}

/* -------------------------------------------------------
 * Fixture: legacy_0.7.1
 *   We don't generate a synthetic v0.7.1 binary because the
 *   exact on-disk format requires matching an older pickle
 *   layout that is not exposed through the current API.
 *   Instead, copy an existing 0.7.1 scene from utils/release/
 *   if available, otherwise skip gracefully.
 * -------------------------------------------------------*/
static int make_legacy_071(const char* fixtures_dir,
                            const char* repo_root) {
    char src[512], dst[512];
    FILE *in_f, *out_f;
    unsigned char buf[4096];
    size_t nr;
    int found = 0;

    /* Try a couple of known 0.7.1 scene files */
    const char* candidates[] = {
        "utils/release/scenes-0.7.1/cvtest.scn",
        "utils/release/scenes-0.7.1/space.scn",
        NULL
    };

    for (int i = 0; candidates[i]; i++) {
        snprintf(src, sizeof(src), "%s/%s", repo_root, candidates[i]);
        in_f = fopen(src, "rb");
        if (in_f) {
            snprintf(dst, sizeof(dst), "%s/legacy_0.7.1.scn", fixtures_dir);
            out_f = fopen(dst, "wb");
            if (!out_f) {
                fclose(in_f);
                fprintf(stderr, "[make_fixtures] ERROR: cannot write legacy fixture\n");
                return 1;
            }
            while ((nr = fread(buf, 1, sizeof(buf), in_f)) > 0) {
                fwrite(buf, 1, nr, out_f);
            }
            fclose(in_f);
            fclose(out_f);
            printf("[make_fixtures] copied legacy scene from %s → %s\n",
                   candidates[i], dst);
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("[make_fixtures] NOTE: no 0.7.1 source scene found, "
               "skipping legacy_0.7.1.scn\n");
    }
    return 0;
}

/* -------------------------------------------------------
 * main
 * -------------------------------------------------------*/
int main(int argc, char** argv) {
    const char* repo_root = (argc >= 2) ? argv[1] : "../..";
    /* fixtures dir is relative to beekeep/ */
    const char* fixtures_dir = "test/fixtures";
    int errors = 0;

    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    printf("[make_fixtures] === Aleph BEES Fixture Generator ===\n");
    printf("[make_fixtures] repo_root: %s\n", repo_root);
    printf("[make_fixtures] output:    %s\n", fixtures_dir);

    /* Ensure output directory exists */
    ensure_dir("test");
    ensure_dir(fixtures_dir);

    /* Set working dir so files.c is happy (not strictly required here
     * since we call fwrite directly, but keep it consistent). */
    strncpy(workingDir, fixtures_dir, sizeof(workingDir) - 1);
    workingDir[sizeof(workingDir) - 1] = '\0';
    /* Append trailing slash */
    {
        int wdlen = (int)strlen(workingDir);
        if (wdlen > 0 && workingDir[wdlen-1] != '/') {
            workingDir[wdlen] = '/';
            workingDir[wdlen+1] = '\0';
        }
    }

    /* Boot BEES once */
    printf("[make_fixtures] calling app_init()...\n");
    app_init();
    printf("[make_fixtures] calling app_launch()...\n");
    app_launch(1);

    /* Generate fixtures */
    printf("\n[make_fixtures] --- fixture: empty ---\n");
    errors += make_empty(fixtures_dir);

    printf("\n[make_fixtures] --- fixture: two_op_network ---\n");
    errors += make_two_op_network(fixtures_dir);

    printf("\n[make_fixtures] --- fixture: legacy_0.7.1 ---\n");
    errors += make_legacy_071(fixtures_dir, repo_root);

    /* Summary */
    printf("\n[make_fixtures] === done (errors=%d) ===\n", errors);
    return errors ? 1 : 0;
}
