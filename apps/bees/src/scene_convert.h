/*
  scene_convert.h
  
  Scene format conversion between BEES 0.7.1 and 0.8.x
  
  PURPOSE:
  Provides conversion functions to load 0.7.1 scenes in 0.8.x by remapping
  operator IDs and output indices that changed between versions.
  
  USAGE:
  Called automatically by scene.c when needsConnectionRemapping flag is set
  (indicates 0.7.1 scene detected).
  
  Created: 2026-01-10
  Author: Scene migration analysis (GitHub Copilot)
*/

#ifndef _ALEPH_BEES_SCENE_CONVERT_H_
#define _ALEPH_BEES_SCENE_CONVERT_H_

#include "types.h"

/* -------------------------------------------------------------------------
 * Scene data structures (for simulator tests)
 * --------------------------------------------------------------------------*/

#define SCENE_MAX_OPS   64
#define SCENE_MAX_NETS  256

#define SCENE_VERSION_071  0x0701
#define SCENE_VERSION_08X  0x0800

typedef struct {
    s16 type_id;
    s32 params[8];
} scene_op_t;

typedef struct {
    s16 src_op;
    s16 src_output;
    s16 dst_op;
    s16 dst_input;
} scene_net_t;

typedef struct {
    u16        version;
    u16        num_ops;
    u16        num_nets;
    scene_op_t  ops[SCENE_MAX_OPS];
    scene_net_t nets[SCENE_MAX_NETS];
} scene_data_t;

/* -------------------------------------------------------------------------
 * Scene Conversion API
 * --------------------------------------------------------------------------*/

/**
 * Convert 0.7.1 scene pickle data to 0.8.x format
 * 
 * This function performs in-place conversion of scene data, remapping:
 * - Operator IDs (if needed - mostly stable)
 * - Output indices (for operators that gained outputs)
 * - Connection routing (based on cumulative output shifts)
 * 
 * @param pickle     Pointer to pickle data buffer (0.7.1 format)
 * @param pickleSize Size of pickle buffer in bytes
 * 
 * @return 1 on success, 0 on failure
 * 
 * NOTES:
 * - Modifies pickle data in-place
 * - Assumes pickle buffer is large enough for conversions
 * - Logs conversion details for debugging
 * - Safe to call multiple times (idempotent if already converted)
 */
extern u8 scene_convert_v07_to_v08(u8* pickle, u32 pickleSize);

/**
 * Check if scene pickle data is 0.7.1 format
 * 
 * Examines scene data to determine if it needs conversion.
 * 
 * @param pickle Pointer to pickle data buffer
 * 
 * @return 1 if 0.7.1 format detected, 0 otherwise
 */
extern u8 scene_is_v07_format(const u8* pickle, u32 pickleSize);

/**
 * Validate converted scene data
 * 
 * Performs sanity checks on converted scene to ensure:
 * - Operator IDs are valid
 * - Connection indices are within bounds
 * - No obvious corruption
 * 
 * @param pickle Pointer to converted pickle data
 * 
 * @return 1 if valid, 0 if corruption detected
 */
extern u8 scene_validate_converted(const u8* pickle, u32 pickleSize);

/* Direct scene conversion (for tests with structured data) */
extern int scene_convert(scene_data_t *scene);
extern int scene_convert_op_id(int old_id);
extern int scene_convert_output_idx(int op_new_id, int old_output);

//==============================================================================
// Conversion Statistics (for debugging)
//==============================================================================

typedef struct {
  u32 numOperatorsConverted;
  u32 numConnectionsRemapped;
  u32 numOutputsShifted;
  u32 conversionTimeMs;
  u8 hadErrors;
} SceneConversionStats;

/**
 * Get statistics from last conversion
 * 
 * @return Pointer to conversion stats structure
 */
extern const SceneConversionStats* scene_get_conversion_stats(void);

/**
 * Reset conversion statistics
 */
extern void scene_reset_conversion_stats(void);

#endif // _ALEPH_BEES_SCENE_CONVERT_H_
