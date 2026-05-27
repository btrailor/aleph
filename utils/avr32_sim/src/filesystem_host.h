/* filesystem_host.h
 * avr32_sim
 * aleph
 *
 * Host-backed filesystem layer for the headless/sim build.
 * Wraps POSIX fopen/fclose/fread/fwrite so upper-level BEES code
 * (files.c) can access scene files on the host directory tree
 * without a FAT layer or SD card.
 *
 * Usage:
 *   1. Call fs_host_set_root(dir) early in main() to point at the
 *      directory that contains your scenes/ subdirectory.
 *   2. The BEES files.c already uses fopen() directly (no FAT calls),
 *      so no further wiring is needed.  This module exists primarily
 *      to document the design and to expose helpers used by the
 *      test harness.
 */

#ifndef _ALEPH_FILESYSTEM_HOST_H_
#define _ALEPH_FILESYSTEM_HOST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "types.h"

/* Maximum length for a host filesystem root path */
#define FS_HOST_ROOT_MAX 512

/* -------------------------------------------------------
 * Configuration
 * -------------------------------------------------------*/

/** Set the root directory used when resolving relative paths.
 *  @param dir  Absolute path to the root directory (no trailing slash).
 *              Pass NULL or "" to reset to the process working directory.
 */
void fs_host_set_root(const char* dir);

/** Return the currently configured root directory (never NULL). */
const char* fs_host_get_root(void);

/* -------------------------------------------------------
 * Helpers
 * -------------------------------------------------------*/

/** Build an absolute path from root + relative sub-path.
 *  @param out      Output buffer (caller-provided, at least FS_HOST_ROOT_MAX bytes).
 *  @param relpath  Relative path such as "data/bees/scenes/myscene.scn".
 *  @return out (for convenience).
 */
char* fs_host_resolve(char* out, const char* relpath);

/** Open a file relative to the configured root.
 *  Equivalent to fopen(fs_host_resolve(buf, relpath), mode).
 *  @return FILE* or NULL on failure.
 */
FILE* fs_host_fopen(const char* relpath, const char* mode);

/** Return 1 if the file at relpath exists under the root, 0 otherwise. */
u8 fs_host_exists(const char* relpath);

/** Return the size in bytes of a file, or -1 on error. */
long fs_host_size(const char* relpath);

/* -------------------------------------------------------
 * Compatibility shim
 * -------------------------------------------------------*/

/** Called instead of fat_init() in the headless build.
 *  Verifies that the root directory is accessible.
 *  Returns 0 on success, non-zero on failure.
 */
int fs_host_init(const char* root_dir);

#ifdef __cplusplus
}
#endif

#endif /* _ALEPH_FILESYSTEM_HOST_H_ */
