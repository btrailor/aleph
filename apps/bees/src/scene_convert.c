/*
  scene_convert.c
  
  Scene format conversion between BEES 0.7.1 and 0.8.x
  
  Converts 0.7.1 scene pickle data to 0.8.x format by remapping operator IDs
  and output indices that changed between versions.
  
  Created: 2026-01-10
*/

#include "scene_convert.h"
#include "OPERATOR_ID_MAPPING.h"
#include "OPERATOR_OUTPUT_CHANGES.h"
#include "pickle.h"
#include "print_funcs.h"

#include <string.h>

//==============================================================================
// Private State
//==============================================================================

static SceneConversionStats conversionStats;

//==============================================================================
// Helper Functions
//==============================================================================

/**
 * Calculate cumulative output shift up to a given operator index
 * 
 * Walks through operators and sums the output count changes for all operators
 * that came before the target operator.
 * 
 * @param operatorIds   Array of operator IDs from scene (0.7.1 format)
 * @param numOperators  Number of operators in scene
 * @param targetOpIndex Index of operator we're calculating shift for
 * 
 * @return Cumulative number of outputs added before this operator
 */
static u16 calculate_cumulative_output_shift(
  const op_id_t* operatorIds,
  u32 numOperators,
  u32 targetOpIndex
) {
  u16 cumulative_shift = 0;
  
  // Walk through all operators BEFORE the target
  for (u32 i = 0; i < targetOpIndex && i < numOperators; i++) {
    op_id_t op_id = operatorIds[i];
    
    // Check if this operator gained outputs
    for (u8 j = 0; j < NUM_OUTPUT_CHANGES; j++) {
      if (outputChanges[j].op_id_v07 == op_id || 
          outputChanges[j].op_id_v08 == op_id) {
        cumulative_shift += outputChanges[j].outputs_added;
        break;
      }
    }
  }
  
  return cumulative_shift;
}

/**
 * Remap a single output index based on cumulative shifts
 * 
 * @param oldOutputIdx  Output index from 0.7.1 scene
 * @param operatorIds   Array of operator IDs from scene
 * @param numOperators  Number of operators
 * @param targetOutput  Which output number of the operator (for debugging)
 * 
 * @return Remapped output index for 0.8.x
 */
static u16 remap_output_index(
  u16 oldOutputIdx,
  const op_id_t* operatorIds,
  u32 numOperators,
  u16 targetOutput
) {
  // Find which operator owns this output
  u16 currentOutputBase = 0;
  u16 cumulative_shift = 0;
  
  for (u32 i = 0; i < numOperators; i++) {
    op_id_t op_id = operatorIds[i];
    
    // Get output count for this operator in 0.7.1
    u8 numOutputs_v07 = 0;
    u8 outputsAdded = 0;
    
    // Check if this operator changed output count
    for (u8 j = 0; j < NUM_OUTPUT_CHANGES; j++) {
      if (outputChanges[j].op_id_v07 == op_id || 
          outputChanges[j].op_id_v08 == op_id) {
        numOutputs_v07 = outputChanges[j].num_outputs_v07;
        outputsAdded = outputChanges[j].outputs_added;
        break;
      }
    }
    
    // If we haven't found output counts, this operator didn't change
    // We need to look up actual output count (TODO: need operator registry)
    if (numOutputs_v07 == 0 && outputsAdded == 0) {
      // Assume some default for now - this needs proper lookup
      numOutputs_v07 = 2;  // FIXME: Need actual lookup
    }
    
    // Check if target output is in this operator's range
    if (oldOutputIdx >= currentOutputBase && 
        oldOutputIdx < currentOutputBase + numOutputs_v07) {
      // Found the owning operator, apply cumulative shift
      u16 newOutputIdx = oldOutputIdx + cumulative_shift;
      
      #ifdef PRINT_SCENE_CONVERT
      print_dbg("\r\n  Remap output: ");
      print_dbg_ulong(oldOutputIdx);
      print_dbg(" -> ");
      print_dbg_ulong(newOutputIdx);
      print_dbg(" (shift: ");
      print_dbg_ulong(cumulative_shift);
      print_dbg(")");
      #endif
      
      conversionStats.numOutputsShifted++;
      return newOutputIdx;
    }
    
    // Move to next operator's output range
    currentOutputBase += numOutputs_v07;
    
    // Accumulate shift if this operator gained outputs
    if (outputsAdded > 0) {
      cumulative_shift += outputsAdded;
    }
  }
  
  // Output not found in any operator range - return unchanged
  // (might be invalid connection, will be caught by validation)
  return oldOutputIdx;
}

//==============================================================================
// Public API Implementation
//==============================================================================

u8 scene_is_v07_format(const u8* pickle) {
  if (pickle == NULL) {
    return 0;
  }
  
  // The scene header contains version info - check if it's 0.7.x
  // This is a simplified check - actual implementation needs to parse
  // the scene descriptor properly
  
  // For now, rely on caller setting needsConnectionRemapping flag
  return 0;  // FIXME: Implement proper version detection
}

u8 scene_validate_converted(const u8* pickle) {
  if (pickle == NULL) {
    return 0;
  }
  
  // TODO: Implement validation
  // - Check operator IDs are in valid range
  // - Check connection indices are within bounds
  // - Verify no obviously corrupt data
  
  return 1;  // Placeholder: assume valid for now
}

u8 scene_convert_v07_to_v08(u8* pickle, u32 pickleSize) {
  if (pickle == NULL || pickleSize == 0) {
    print_dbg("\r\n [SCENE_CONVERT] Error: null pickle or zero size");
    return 0;
  }
  
  // Reset stats
  memset(&conversionStats, 0, sizeof(SceneConversionStats));
  
  print_dbg("\r\n");
  print_dbg("\r\n ====================================");
  print_dbg("\r\n  Scene Conversion: 0.7.1 -> 0.8.x");
  print_dbg("\r\n ====================================");
  
  // TODO: Implement actual conversion logic
  // Steps:
  // 1. Parse operator list from pickle
  // 2. Remap operator IDs (simple - mostly stable)
  // 3. Parse connection list
  // 4. Remap output indices (complex - cumulative shifts)
  // 5. Rewrite pickle data
  
  print_dbg("\r\n [SCENE_CONVERT] Conversion not yet implemented");
  print_dbg("\r\n [SCENE_CONVERT] This is a placeholder stub");
  
  conversionStats.hadErrors = 0;
  conversionStats.numOperatorsConverted = 0;
  conversionStats.numConnectionsRemapped = 0;
  
  return 1;  // Placeholder: return success for now
}

const SceneConversionStats* scene_get_conversion_stats(void) {
  return &conversionStats;
}

void scene_reset_conversion_stats(void) {
  memset(&conversionStats, 0, sizeof(SceneConversionStats));
}
