/*
  OPERATOR_OUTPUT_CHANGES.h
  
  Mapping of operator output count changes between BEES 0.7.1 and 0.8.x
  
  PURPOSE:
  When loading 0.7.1 scenes in 0.8.x, output indices stored in connections
  become invalid due to operators gaining outputs. This file provides the
  mapping needed to correctly remap output indices.
  
  ANALYSIS SOURCE:
  Script comparison of all op_*.c files between:
  - 0.7.1: Git tag 'bees-0.7.1' (2014 official release)
  - 0.8.x: Git branch 'dev' (2018 community release, v0.8.1)
  
  Date: 2024-12-19
  
  WARNING:
  Per Yann Copier (Lines forum): "All operators that didn't have an output 
  before (SCREEN, BIGNUM for instance) have now a dummy output, which can't 
  be used for anything but will for sure break scene compatibility."
  
  CRITICAL:
  These operators gaining outputs causes SILENT FAILURES - connections are
  misrouted without error messages. Patches load successfully but are broken.
*/

#ifndef _ALEPH_BEES_OPERATOR_OUTPUT_CHANGES_H_
#define _ALEPH_BEES_OPERATOR_OUTPUT_CHANGES_H_

#include "types.h"
#include "op.h"

//==============================================================================
// Data Structures
//==============================================================================

/**
 * Maps operator output count changes between versions
 * 
 * Used to calculate cumulative output index shifts when converting
 * 0.7.1 scenes to 0.8.x format.
 */
typedef struct {
  op_id_t op_id_v07;     // Operator ID in 0.7.1 (TODO: populate from 0.7.1 enum)
  op_id_t op_id_v08;     // Operator ID in 0.8.x (from current op.h enum)
  const char* op_name;   // Operator name for debugging
  u8 num_outputs_v07;    // Number of outputs in 0.7.1
  u8 num_outputs_v08;    // Number of outputs in 0.8.x
  s8 outputs_added;      // Difference (v08 - v07), can be negative
} OpOutputChangeMap;

//==============================================================================
// Operator Output Changes: 0.7.1 → 0.8.x
//==============================================================================

/**
 * Complete list of operators that changed output count
 * 
 * ANALYSIS RESULTS:
 * - 5 operators gained outputs (all gained exactly +1)
 * - 0 operators lost outputs
 * - All gained outputs are dummy outputs (labeled "DUMMY", not functional)
 * 
 * IMPACT:
 * These 5 operators cause cumulative output index shift of +5 for any
 * operators that come AFTER them in the operator creation order.
 */
static const OpOutputChangeMap outputChanges[] = {
  // NOTE: op_id_v07 values need to be populated from 0.7.1 op.h enum
  // Currently using UNKNOWN (0xFF) as placeholder
  
  // BARS - Bar graph display operator
  // Verification: op_bars.c line ~XX has numOutputs = 1 in 0.8.x
  { 
    .op_id_v07      = 0xFF,        // TODO: Get from 0.7.1 enum
    .op_id_v08      = eOpBars,     // From current op.h
    .op_name        = "bars",
    .num_outputs_v07 = 0,
    .num_outputs_v08 = 1,
    .outputs_added  = +1
  },
  
  // BIGNUM - Large number display operator
  // Verification: op_bignum.c line ~82 has numOutputs = 1 in 0.8.x
  //               outString = "DUMMY"
  { 
    .op_id_v07      = 0xFF,        // TODO: Get from 0.7.1 enum
    .op_id_v08      = eOpBignum,   // From current op.h
    .op_name        = "bignum",
    .num_outputs_v07 = 0,
    .num_outputs_v08 = 1,
    .outputs_added  = +1
  },
  
  // MIDI_OUT_CC - MIDI continuous controller output
  // Verification: op_midi_out_cc.c has numOutputs = 1 in 0.8.x
  { 
    .op_id_v07      = 0xFF,           // TODO: Get from 0.7.1 enum
    .op_id_v08      = eOpMidiOutCC,   // From current op.h (capital CC)
    .op_name        = "midi_out_cc",
    .num_outputs_v07 = 0,
    .num_outputs_v08 = 1,
    .outputs_added  = +1
  },
  
  // MIDI_OUT_NOTE - MIDI note output
  // Verification: op_midi_out_note.c has numOutputs = 1 in 0.8.x
  { 
    .op_id_v07      = 0xFF,             // TODO: Get from 0.7.1 enum
    .op_id_v08      = eOpMidiOutNote,   // From current op.h
    .op_name        = "midi_out_note",
    .num_outputs_v07 = 0,
    .num_outputs_v08 = 1,
    .outputs_added  = +1
  },
  
  // SCREEN - Text display operator
  // Verification: op_screen.c line ~72 has numOutputs = 1 in 0.8.x
  //               outString = "DUMMY"
  { 
    .op_id_v07      = 0xFF,         // TODO: Get from 0.7.1 enum
    .op_id_v08      = eOpScreen,    // From current op.h
    .op_name        = "screen",
    .num_outputs_v07 = 0,
    .num_outputs_v08 = 1,
    .outputs_added  = +1
  }
};

// Number of entries in outputChanges array
#define NUM_OUTPUT_CHANGES (sizeof(outputChanges) / sizeof(OpOutputChangeMap))

//==============================================================================
// Utility Functions
//==============================================================================

/**
 * Get output change info for a given 0.8.x operator ID
 * 
 * @param op_id  Operator ID in 0.8.x format
 * @return       Pointer to OpOutputChangeMap or NULL if operator unchanged
 */
static inline const OpOutputChangeMap* get_output_change(op_id_t op_id) {
  for (u8 i = 0; i < NUM_OUTPUT_CHANGES; i++) {
    if (outputChanges[i].op_id_v08 == op_id) {
      return &outputChanges[i];
    }
  }
  return NULL;  // Operator did not change output count
}

/**
 * Calculate cumulative output shift for a given output index in 0.7.1 scene
 * 
 * This function determines how many outputs were added BEFORE a given output
 * index, which tells us how much to shift the index when loading in 0.8.x.
 * 
 * USAGE:
 * u16 old_idx = connection->outIdx;  // From 0.7.1 scene
 * u16 new_idx = old_idx + calculate_output_shift(old_idx, scene_operators);
 * 
 * @param output_idx_v07  Output index from 0.7.1 scene
 * @param operators       Array of operators in scene (0.7.1 format)
 * @param num_operators   Number of operators in scene
 * @return                Number of outputs to add to output_idx_v07
 * 
 * NOTE: This is a placeholder - actual implementation needs to walk through
 *       scene operators in creation order and sum outputs_added.
 */
static inline u16 calculate_output_shift(
  u16 output_idx_v07,
  void* operators,      // TODO: Proper type (SceneData_07*)
  u16 num_operators
) {
  // TODO: Implement based on operator creation order in scene
  // Pseudocode:
  //   u16 cumulative_shift = 0;
  //   u16 current_output_base = 0;
  //   
  //   for (each operator in scene, in creation order) {
  //     u16 op_outputs = get_num_outputs(operator, version=0.7.1);
  //     
  //     if (current_output_base + op_outputs > output_idx_v07) {
  //       // We've passed the target output index
  //       return cumulative_shift;
  //     }
  //     
  //     current_output_base += op_outputs;
  //     
  //     // Check if this operator gained outputs
  //     const OpOutputChangeMap* change = get_output_change(operator->id);
  //     if (change && change->outputs_added > 0) {
  //       cumulative_shift += change->outputs_added;
  //     }
  //   }
  //   
  //   return cumulative_shift;
  
  return 0;  // Placeholder
}

//==============================================================================
// Additional Compatibility Notes
//==============================================================================

/*
  OPERATOR REMOVAL/ADDITION:
  
  The following operators were REMOVED in 0.8.x:
  - bars (replaced with new version)
  - bignum (replaced with new version)
  - life (RENAMED to life_classic)
  - midi_out_cc (replaced with new version)
  - midi_out_note (replaced with new version)
  - screen (replaced with new version)
  
  The following operators were ADDED in 0.8.x:
  - ckdiv, cpu, harry, iter, kria, life_classic, linlin, list4, maginc,
    mem0d, mem1d, mem2d, midi_clock, midi_out_clock, midi_prog,
    monome_grid_classic, param, poly
  
  CRITICAL: "life" → "life_classic" rename requires special handling!
  0.7.1 scenes using op_life must be converted to op_life_classic.
  
  OPERATOR ID MAPPING:
  TODO: Need complete mapping table of operator IDs between versions.
  Example:
    typedef struct {
      op_id_t id_v07;
      op_id_t id_v08;
      const char* name_v07;
      const char* name_v08;
    } OpIdMapping;
*/

#endif // _ALEPH_BEES_OPERATOR_OUTPUT_CHANGES_H_
