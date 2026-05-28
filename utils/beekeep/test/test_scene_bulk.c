/* test_scene_bulk.c
 * utils/beekeep/test
 *
 * Bulk scene loading test — validates all legacy 0.7.1 scenes.
 *
 * For each .scn file in ../release/scenes-0.7.1/:
 *   1. Load the scene
 *   2. Run scene_convert if needed
 *   3. Report success/failure with details
 *
 * This is a diagnostic tool, not a regression test — it tells us
 * which scenes are broken and why.
 */

#include <stdio.h>
#include <string.h>
#include <dirent.h>

#include "scene.h"
#include "scene_convert.h"
#include "types.h"
#include "files.h"

/* -------------------------------------------------------------------------
 * Results tracking
 * --------------------------------------------------------------------------*/
typedef struct {
    int total;
    int passed;
    int failed;
    int skipped;
} scene_bulk_stats_t;

static scene_bulk_stats_t stats = {0, 0, 0, 0};

/* -------------------------------------------------------------------------
 * Test a single scene file
 * --------------------------------------------------------------------------*/
static int test_scene_file(const char* path, const char* filename) {
    scene_data_t scene;
    int result;
    
    printf("  Testing: %s ... ", filename);
    
    /* Try to load the scene */
    result = scene_read_file(path, &scene);
    if (result != 0) {
        printf("FAIL (read error %d)\n", result);
        return 1;
    }
    
    /* Check version */
    if (scene.version == SCENE_VERSION_071) {
        printf("0.7.1 → converting ... ");
        result = scene_convert(&scene);
        if (result != 0) {
            printf("FAIL (conversion error %d)\n", result);
            return 1;
        }
        printf("converted → ");
    } else if (scene.version == SCENE_VERSION_08X) {
        printf("already 0.8.x → ");
    } else {
        printf("FAIL (unknown version %d)\n", scene.version);
        return 1;
    }
    
    /* Validate operator count */
    if (scene.num_ops == 0) {
        printf("WARN (0 ops — empty scene?)\n");
        return 0;  /* Not a failure, just empty */
    }
    
    /* Check for out-of-range operator IDs */
    int i;
    for (i = 0; i < scene.num_ops; i++) {
        if (scene.ops[i].type_id < 0 || scene.ops[i].type_id > 100) {
            printf("FAIL (op %d has invalid type_id %d)\n", i, scene.ops[i].type_id);
            return 1;
        }
    }
    
    /* Check network integrity */
    for (i = 0; i < scene.num_nets; i++) {
        if (scene.nets[i].src_op >= scene.num_ops) {
            printf("FAIL (net %d refs invalid src_op %d >= %d)\n", 
                   i, scene.nets[i].src_op, scene.num_ops);
            return 1;
        }
        if (scene.nets[i].dst_op >= scene.num_ops) {
            printf("FAIL (net %d refs invalid dst_op %d >= %d)\n", 
                   i, scene.nets[i].dst_op, scene.num_ops);
            return 1;
        }
    }
    
    printf("OK (%d ops, %d nets)\n", scene.num_ops, scene.num_nets);
    return 0;
}

/* -------------------------------------------------------------------------
 * Main
 * --------------------------------------------------------------------------*/
int run_scene_bulk_tests(const char* scenes_dir) {
    DIR* dir;
    struct dirent* entry;
    char path[512];
    int failures = 0;
    
    printf("# Bulk Scene Loading Test\n");
    printf("# Directory: %s\n\n", scenes_dir);
    
    dir = opendir(scenes_dir);
    if (dir == NULL) {
        printf("BAIL OUT: Cannot open directory: %s\n", scenes_dir);
        return 1;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        /* Skip non-.scn files */
        size_t len = strlen(entry->d_name);
        if (len < 4 || strcmp(entry->d_name + len - 4, ".scn") != 0) {
            continue;
        }
        
        /* Skip backup files (files with " 2.scn" suffix) */
        if (strstr(entry->d_name, " 2.scn") != NULL) {
            stats.skipped++;
            continue;
        }
        
        snprintf(path, sizeof(path), "%s/%s", scenes_dir, entry->d_name);
        
        stats.total++;
        if (test_scene_file(path, entry->d_name) == 0) {
            stats.passed++;
        } else {
            stats.failed++;
            failures++;
        }
    }
    
    closedir(dir);
    
    printf("\n# Results: %d total, %d passed, %d failed, %d skipped (backups)\n",
           stats.total, stats.passed, stats.failed, stats.skipped);
    
    if (failures == 0) {
        printf("# All scenes load successfully!\n");
    } else {
        printf("# %d scene(s) need attention.\n", failures);
    }
    
    return failures;
}

/* Standalone entry point for direct execution */
#ifdef SCENE_BULK_MAIN
int main(int argc, char* argv[]) {
    const char* dir = (argc > 1) ? argv[1] : "../release/scenes-0.7.1";
    return run_scene_bulk_tests(dir);
}
#endif
