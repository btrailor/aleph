/* filesystem_host.c
 * avr32_sim
 * aleph
 *
 * Host-backed filesystem layer for the headless/sim build.
 * See filesystem_host.h for the public API.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "filesystem_host.h"
#include "print_funcs.h"

/* -------------------------------------------------------
 * Internal state
 * -------------------------------------------------------*/

static char s_root[FS_HOST_ROOT_MAX] = "";

/* -------------------------------------------------------
 * Configuration
 * -------------------------------------------------------*/

void fs_host_set_root(const char* dir) {
    if (dir == NULL || dir[0] == '\0') {
        s_root[0] = '\0';
    } else {
        strncpy(s_root, dir, FS_HOST_ROOT_MAX - 1);
        s_root[FS_HOST_ROOT_MAX - 1] = '\0';
        /* Strip trailing slash if present */
        int len = (int)strlen(s_root);
        if (len > 0 && s_root[len - 1] == '/') {
            s_root[len - 1] = '\0';
        }
    }
    print_dbg("\r\n [fs_host] root set to: ");
    print_dbg(s_root[0] ? s_root : "(cwd)");
}

const char* fs_host_get_root(void) {
    return s_root;
}

/* -------------------------------------------------------
 * Helpers
 * -------------------------------------------------------*/

char* fs_host_resolve(char* out, const char* relpath) {
    if (s_root[0] == '\0') {
        /* No root set — use relpath as-is */
        strncpy(out, relpath, FS_HOST_ROOT_MAX - 1);
        out[FS_HOST_ROOT_MAX - 1] = '\0';
    } else {
        /* Prepend root, inserting a '/' separator */
        snprintf(out, FS_HOST_ROOT_MAX, "%s/%s", s_root, relpath);
    }
    return out;
}

FILE* fs_host_fopen(const char* relpath, const char* mode) {
    char fullpath[FS_HOST_ROOT_MAX];
    fs_host_resolve(fullpath, relpath);
    return fopen(fullpath, mode);
}

u8 fs_host_exists(const char* relpath) {
    char fullpath[FS_HOST_ROOT_MAX];
    struct stat st;
    fs_host_resolve(fullpath, relpath);
    return (stat(fullpath, &st) == 0) ? 1 : 0;
}

long fs_host_size(const char* relpath) {
    char fullpath[FS_HOST_ROOT_MAX];
    struct stat st;
    fs_host_resolve(fullpath, relpath);
    if (stat(fullpath, &st) != 0) {
        return -1L;
    }
    return (long)st.st_size;
}

/* -------------------------------------------------------
 * Compatibility shim: replaces fat_init() in headless build
 * -------------------------------------------------------*/

int fs_host_init(const char* root_dir) {
    struct stat st;

    fs_host_set_root(root_dir);

    /* Verify the root directory exists and is a directory */
    if (root_dir && root_dir[0] != '\0') {
        if (stat(root_dir, &st) != 0) {
            print_dbg("\r\n [fs_host] ERROR: root directory not found: ");
            print_dbg(root_dir);
            return 1;
        }
        if (!S_ISDIR(st.st_mode)) {
            print_dbg("\r\n [fs_host] ERROR: root path is not a directory: ");
            print_dbg(root_dir);
            return 1;
        }
    }

    print_dbg("\r\n [fs_host] filesystem_host initialised OK");
    return 0;
}
