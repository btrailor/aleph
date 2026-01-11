/*
 * Connection compatibility mappings for BEES scene v0.7 → v0.8 conversion
 * This addresses the core issue where operator I/O changes cause connection index shifts
 */

#ifndef _BEEKEEP_CONNECTION_COMPAT_H_
#define _BEEKEEP_CONNECTION_COMPAT_H_

#include "op.h"
#include "version_compat.h"

// Connection remapping for operators that changed between versions
typedef struct {
    op_id_t op_id;
    
    // Input remapping: v0.7_input_index → v0.8_input_index (-1 = no mapping)
    int input_remap[8];  // Max 8 inputs for any operator
    int v07_input_count;
    int v08_input_count;
    
    // Output remapping: v0.7_output_index → v0.8_output_index (-1 = no mapping)  
    int output_remap[8]; // Max 8 outputs for any operator
    int v07_output_count;
    int v08_output_count;
} connection_remap_t;

// Remapping table for operators that changed I/O layout
static const connection_remap_t connection_remaps[] = {
    {
        // MIDIOUTNO: Added PROG input at index 5, added DUMMY output at index 0
        .op_id = eOpMidiOutNote,
        .v07_input_count = 5,
        .v08_input_count = 6,
        .input_remap = {0, 1, 2, 3, 4, -1, -1, -1}, // CABLE,CHAN,NUM,VEL,PITCH → same positions, PROG new
        .v07_output_count = 0,
        .v08_output_count = 1,
        .output_remap = {-1, -1, -1, -1, -1, -1, -1, -1} // No outputs in v0.7
    },
    {
        // MIDIOUTCLOCK: Added DUMMY output at index 0  
        .op_id = eOpMidiOutClock,
        .v07_input_count = 5,
        .v08_input_count = 5,
        .input_remap = {0, 1, 2, 3, 4, -1, -1, -1}, // All inputs stay same
        .v07_output_count = 0,
        .v08_output_count = 1,
        .output_remap = {-1, -1, -1, -1, -1, -1, -1, -1} // No outputs in v0.7
    },
    {
        // MIDICLK: Added DUMMY input at index 0, outputs stay same
        .op_id = eOpMidiClock,
        .v07_input_count = 0,
        .v08_input_count = 1,
        .input_remap = {-1, -1, -1, -1, -1, -1, -1, -1}, // No inputs in v0.7
        .v07_output_count = 4,
        .v08_output_count = 4,
        .output_remap = {0, 1, 2, 3, -1, -1, -1, -1} // TICK,START,CONT,STOP stay same
    },
    {
        // CASCADES: Fixed STEP input behavior but same I/O count
        .op_id = eOpCascades,
        .v07_input_count = 3,
        .v08_input_count = 3,
        .input_remap = {0, 1, 2, -1, -1, -1, -1, -1}, // All inputs stay same
        .v07_output_count = 8,
        .v08_output_count = 8,
        .output_remap = {0, 1, 2, 3, 4, 5, 6, 7} // All outputs stay same
    }
};

static const int num_connection_remaps = sizeof(connection_remaps) / sizeof(connection_remaps[0]);

// Function to get connection remapping for an operator
const connection_remap_t* get_connection_remap(op_id_t op_id);

// Function to remap input index from v0.7 to v0.8
int remap_input_index(op_id_t op_id, int v07_index);

// Function to remap output index from v0.7 to v0.8  
int remap_output_index(op_id_t op_id, int v07_index);

#endif