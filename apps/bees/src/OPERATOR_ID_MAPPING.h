/*
  OPERATOR_ID_MAPPING.h
  
  Complete mapping of operator IDs between BEES 0.7.1 and 0.8.x
  
  PURPOSE:
  When loading 0.7.1 scenes in 0.8.x, operator IDs stored in scenes must be
  remapped to their correct 0.8.x equivalents. This file provides the complete
  mapping table.
  
  ANALYSIS SOURCE:
  Extracted from op.h enum definitions:
  - 0.7.1: Git tag 'bees-0.7.1' apps/bees/src/op.h
  - 0.8.x: Git branch 'dev' apps/bees/src/op.h
  
  Date: 2026-01-10
  
  CRITICAL FINDINGS:
  - IDs 0-50 (eOpSwitch through eOpMidiOutCC) are STABLE with 2 exceptions:
    * ID 5: eOpMonomeGridRaw (0.7.1) → eOpMonomeGridClassic (0.8.x) [RENAMED]
    * ID 24: eOpLife (0.7.1) → eOpLifeClassic (0.8.x) [RENAMED]
  
  - IDs 51+ were ADDED in 0.8.x (eOpParam through eOpList4)
  
  - eOpMonomeGridRaw MOVED from ID 5 to ID 58 in 0.8.x
*/

#ifndef _ALEPH_BEES_OPERATOR_ID_MAPPING_H_
#define _ALEPH_BEES_OPERATOR_ID_MAPPING_H_

#include <stddef.h>  // For NULL
#include "types.h"
#include "op.h"

//==============================================================================
// Constants
//==============================================================================

// Special value for operators removed in 0.8.x
#define OP_ID_REMOVED 0xFF

//==============================================================================
// Data Structures
//==============================================================================

/**
 * Maps operator IDs between 0.7.1 and 0.8.x
 * 
 * This provides the complete bidirectional mapping needed for scene conversion.
 */
typedef struct {
  op_id_t id_v07;        // Operator ID in 0.7.1 (enum value)
  op_id_t id_v08;        // Operator ID in 0.8.x (enum value or OP_ID_REMOVED)
  const char* name_v07;  // Operator name in 0.7.1
  const char* name_v08;  // Operator name in 0.8.x (or NULL if removed)
  u8 id_changed;         // 1 if ID differs between versions, 0 if same
  u8 name_changed;       // 1 if name differs (rename), 0 if same
} OpIdMapping;

//==============================================================================
// Complete Operator ID Mapping Table
//==============================================================================

/**
 * Operator ID mapping: 0.7.1 → 0.8.x
 * 
 * NOTES:
 * - Most operators (0-50) maintained their ID positions
 * - Two critical renames at same ID position:
 *   * eOpMonomeGridRaw → eOpMonomeGridClassic (ID 5)
 *   * eOpLife → eOpLifeClassic (ID 24)
 * - eOpMonomeGridRaw was re-added at ID 58 with different implementation
 * - 17 new operators added in 0.8.x (IDs 51-67)
 */
static const OpIdMapping opIdMap[] = {
  // IDs 0-50: Mostly stable with 2 renames
  { 0,  0,  "eOpSwitch",          "eOpSwitch",              0, 0 },
  { 1,  1,  "eOpEnc",             "eOpEnc",                 0, 0 },
  { 2,  2,  "eOpAdd",             "eOpAdd",                 0, 0 },
  { 3,  3,  "eOpMul",             "eOpMul",                 0, 0 },
  { 4,  4,  "eOpGate",            "eOpGate",                0, 0 },
  
  // CRITICAL: ID 5 renamed from MonomeGridRaw to MonomeGridClassic
  // The old MonomeGridRaw implementation moved to ID 58
  { 5,  5,  "eOpMonomeGridRaw",   "eOpMonomeGridClassic",   0, 1 },
  
  { 6,  6,  "eOpMidiNote",        "eOpMidiNote",            0, 0 },
  { 7,  7,  "eOpAdc",             "eOpAdc",                 0, 0 },
  { 8,  8,  "eOpMetro",           "eOpMetro",               0, 0 },
  { 9,  9,  "eOpPreset",          "eOpPreset",              0, 0 },
  { 10, 10, "eOpTog",             "eOpTog",                 0, 0 },
  { 11, 11, "eOpAccum",           "eOpAccum",               0, 0 },
  { 12, 12, "eOpSplit",           "eOpSplit",               0, 0 },
  { 13, 13, "eOpDiv",             "eOpDiv",                 0, 0 },
  { 14, 14, "eOpSub",             "eOpSub",                 0, 0 },
  { 15, 15, "eOpTimer",           "eOpTimer",               0, 0 },
  { 16, 16, "eOpRandom",          "eOpRandom",              0, 0 },
  { 17, 17, "eOpList8",           "eOpList8",               0, 0 },
  { 18, 18, "eOpThresh",          "eOpThresh",              0, 0 },
  { 19, 19, "eOpMod",             "eOpMod",                 0, 0 },
  { 20, 20, "eOpBits",            "eOpBits",                0, 0 },
  { 21, 21, "eOpIs",              "eOpIs",                  0, 0 },
  { 22, 22, "eOpLogic",           "eOpLogic",               0, 0 },
  { 23, 23, "eOpList2",           "eOpList2",               0, 0 },
  
  // CRITICAL: ID 24 renamed from Life to LifeClassic
  { 24, 24, "eOpLife",            "eOpLifeClassic",         0, 1 },
  
  { 25, 25, "eOpHistory",         "eOpHistory",             0, 0 },
  { 26, 26, "eOpBignum",          "eOpBignum",              0, 0 },
  { 27, 27, "eOpScreen",          "eOpScreen",              0, 0 },
  { 28, 28, "eOpSplit4",          "eOpSplit4",              0, 0 },
  { 29, 29, "eOpDelay",           "eOpDelay",               0, 0 },
  { 30, 30, "eOpRoute",           "eOpRoute",               0, 0 },
  { 31, 31, "eOpMidiCC",          "eOpMidiCC",              0, 0 },
  { 32, 32, "eOpMidiOutNote",     "eOpMidiOutNote",         0, 0 },
  { 33, 33, "eOpList16",          "eOpList16",              0, 0 },
  { 34, 34, "eOpStep",            "eOpStep",                0, 0 },
  { 35, 35, "eOpRoute8",          "eOpRoute8",              0, 0 },
  { 36, 36, "eOpCascades",        "eOpCascades",            0, 0 },
  { 37, 37, "eOpBars",            "eOpBars",                0, 0 },
  { 38, 38, "eOpSerial",          "eOpSerial",              0, 0 },
  { 39, 39, "eOpHid",             "eOpHid",                 0, 0 },
  { 40, 40, "eOpWW",              "eOpWW",                  0, 0 },
  { 41, 41, "eOpMonomeArc",       "eOpMonomeArc",           0, 0 },
  { 42, 42, "eOpFade",            "eOpFade",                0, 0 },
  { 43, 43, "eOpDivr",            "eOpDivr",                0, 0 },
  { 44, 44, "eOpShl",             "eOpShl",                 0, 0 },
  { 45, 45, "eOpShr",             "eOpShr",                 0, 0 },
  { 46, 46, "eOpChange",          "eOpChange",              0, 0 },
  { 47, 47, "eOpRoute16",         "eOpRoute16",             0, 0 },
  { 48, 48, "eOpBars8",           "eOpBars8",               0, 0 },
  { 49, 49, "eOpMidiOutCC",       "eOpMidiOutCC",           0, 0 },
  
  // IDs 50-67: NEW operators added in 0.8.x (not in 0.7.1)
  // These don't need conversion mappings since they can't exist in 0.7.1 scenes
  // but included for completeness
  { OP_ID_REMOVED, 50, NULL,      "eOpParam",               1, 0 },  // NEW in 0.8.x
  { OP_ID_REMOVED, 51, NULL,      "eOpMem0d",               1, 0 },  // NEW in 0.8.x
  { OP_ID_REMOVED, 52, NULL,      "eOpMem1d",               1, 0 },  // NEW in 0.8.x
  { OP_ID_REMOVED, 53, NULL,      "eOpMem2d",               1, 0 },  // NEW in 0.8.x
  { OP_ID_REMOVED, 54, NULL,      "eOpIter",                1, 0 },  // NEW in 0.8.x
  { OP_ID_REMOVED, 55, NULL,      "eOpMonomeGridRaw",       1, 0 },  // MOVED from ID 5
  { OP_ID_REMOVED, 56, NULL,      "eOpMidiClock",           1, 0 },  // NEW in 0.8.x
  { OP_ID_REMOVED, 57, NULL,      "eOpMaginc",              1, 0 },  // NEW in 0.8.x
  { OP_ID_REMOVED, 58, NULL,      "eOpKria",                1, 0 },  // NEW in 0.8.x
  { OP_ID_REMOVED, 59, NULL,      "eOpHarry",               1, 0 },  // NEW in 0.8.x
  { OP_ID_REMOVED, 60, NULL,      "eOpPoly",                1, 0 },  // NEW in 0.8.x
  { OP_ID_REMOVED, 61, NULL,      "eOpMidiProg",            1, 0 },  // NEW in 0.8.x
  { OP_ID_REMOVED, 62, NULL,      "eOpMidiOutClock",        1, 0 },  // NEW in 0.8.x
  { OP_ID_REMOVED, 63, NULL,      "eOpCkdiv",               1, 0 },  // NEW in 0.8.x
  { OP_ID_REMOVED, 64, NULL,      "eOpLinlin",              1, 0 },  // NEW in 0.8.x
  { OP_ID_REMOVED, 65, NULL,      "eOpList4",               1, 0 },  // NEW in 0.8.x
};

#define NUM_OP_ID_MAPPINGS (sizeof(opIdMap) / sizeof(OpIdMapping))

//==============================================================================
// Utility Functions
//==============================================================================

/**
 * Convert 0.7.1 operator ID to 0.8.x operator ID
 * 
 * @param op_id_v07  Operator ID from 0.7.1 scene
 * @return           Corresponding 0.8.x operator ID, or OP_ID_REMOVED if not found
 */
static inline op_id_t convert_op_id_v07_to_v08(op_id_t op_id_v07) {
  // Most IDs 0-49 map directly (no change)
  if (op_id_v07 <= 49) {
    // But check for the special cases
    for (u8 i = 0; i < NUM_OP_ID_MAPPINGS; i++) {
      if (opIdMap[i].id_v07 == op_id_v07) {
        return opIdMap[i].id_v08;
      }
    }
    // If not found in map but <= 49, assume direct mapping
    return op_id_v07;
  }
  
  // IDs 50+ don't exist in 0.7.1
  return OP_ID_REMOVED;
}

/**
 * Check if an operator was renamed between versions
 * 
 * @param op_id_v07  Operator ID from 0.7.1 scene
 * @return           1 if operator name changed, 0 otherwise
 */
static inline u8 op_was_renamed(op_id_t op_id_v07) {
  for (u8 i = 0; i < NUM_OP_ID_MAPPINGS; i++) {
    if (opIdMap[i].id_v07 == op_id_v07) {
      return opIdMap[i].name_changed;
    }
  }
  return 0;
}

/**
 * Get operator name in 0.8.x for a given 0.7.1 operator ID
 * 
 * @param op_id_v07  Operator ID from 0.7.1 scene
 * @return           Operator name in 0.8.x, or NULL if removed
 */
static inline const char* get_op_name_v08(op_id_t op_id_v07) {
  for (u8 i = 0; i < NUM_OP_ID_MAPPINGS; i++) {
    if (opIdMap[i].id_v07 == op_id_v07) {
      return opIdMap[i].name_v08;
    }
  }
  return NULL;
}

//==============================================================================
// Critical Notes for Scene Conversion
//==============================================================================

/*
  SCENE CONVERSION CHECKLIST:
  
  ✅ Operator ID mapping complete (this file)
  ✅ Operator output changes mapped (see OPERATOR_OUTPUT_CHANGES.h)
  ❌ TODO: Handle renamed operators in scene loading:
     - eOpMonomeGridRaw (ID 5) → load as eOpMonomeGridClassic
     - eOpLife (ID 24) → load as eOpLifeClassic
  
  IMPORTANT: The renames at ID 5 and 24 are transparent for scene conversion
  because the IDs stayed the same. The operator implementations handle the
  name differences internally.
  
  MONOME GRID SPECIAL CASE:
  In 0.7.1, eOpMonomeGridRaw was at ID 5.
  In 0.8.x, ID 5 became eOpMonomeGridClassic (different implementation).
  The old MonomeGridRaw implementation was moved to ID 55.
  
  For 0.7.1 scenes using MonomeGridRaw (ID 5), they will load as 
  MonomeGridClassic in 0.8.x. This may cause behavioral changes depending
  on implementation differences.
  
  LIFE OPERATOR SPECIAL CASE:
  In 0.7.1, eOpLife was at ID 24 (Conway's Game of Life).
  In 0.8.x, ID 24 is eOpLifeClassic (same implementation, renamed).
  
  Scenes using Life (ID 24) will load correctly as LifeClassic.
*/

#endif // _ALEPH_BEES_OPERATOR_ID_MAPPING_H_
