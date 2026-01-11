#ifndef _BEEKEEP_VERSION_COMPAT_H_
#define _BEEKEEP_VERSION_COMPAT_H_

#include "op.h"

// Version compatibility mappings for BEES scene conversion

// Structure to describe operator changes between versions
typedef struct {
    op_id_t op_id;
    const char* op_name;
    
    // v0.7 structure
    int v07_inputs;
    int v07_outputs;
    
    // v0.8 structure  
    int v08_inputs;
    int v08_outputs;
    
    // Description of what changed
    const char* change_description;
} op_version_diff_t;

// Operators that changed between v0.7 and v0.8
static const op_version_diff_t version_diffs[] = {
    {
        .op_id = eOpMidiOutNote,
        .op_name = "MOUTNO",
        .v07_inputs = 5,  // CABLE, CHAN, NUM, VEL, PITCH
        .v07_outputs = 0,
        .v08_inputs = 6,  // Added PROG input
        .v08_outputs = 1, // Added DUMMY output
        .change_description = "Added PROG input and DUMMY output"
    },
    {
        .op_id = eOpMidiOutClock,
        .op_name = "MOUT_CLK", 
        .v07_inputs = 5,  // CABLE, TICK, START, CONT, STOP
        .v07_outputs = 0,
        .v08_inputs = 5,  // Same inputs
        .v08_outputs = 1, // Added DUMMY output
        .change_description = "Added DUMMY output"
    },
    {
        .op_id = eOpMidiClock,
        .op_name = "MIDICLK",
        .v07_inputs = 0,  // No inputs
        .v07_outputs = 4, // TICK, START, CONT, STOP
        .v08_inputs = 1,  // Added DUMMY input
        .v08_outputs = 4, // Same outputs
        .change_description = "Added DUMMY input"
    },
    {
        .op_id = eOpCascades,
        .op_name = "CASCADES",
        .v07_inputs = 3,  // FOCUS, SIZE, STEP (but STEP was broken)
        .v07_outputs = 8, // a,b,c,d,e,f,g,h
        .v08_inputs = 3,  // Same, but STEP now works with dummy
        .v08_outputs = 8, // Same outputs
        .change_description = "Fixed STEP input with dummy variable"
    }
};

static const int num_version_diffs = sizeof(version_diffs) / sizeof(version_diffs[0]);

// Function to get version differences for an operator
const op_version_diff_t* get_op_version_diff(op_id_t op_id);

// Function to check if operator changed between versions
int op_changed_between_versions(op_id_t op_id);

// Functions to get operator structure for specific version
int get_op_inputs_for_version(op_id_t op_id, int major, int minor);
int get_op_outputs_for_version(op_id_t op_id, int major, int minor);

#endif