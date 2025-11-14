#include "version_compat.h"
#include <stdio.h>

// Get version differences for a specific operator
const op_version_diff_t* get_op_version_diff(op_id_t op_id) {
    for (int i = 0; i < num_version_diffs; i++) {
        if (version_diffs[i].op_id == op_id) {
            return &version_diffs[i];
        }
    }
    return NULL;
}

// Check if operator changed between versions
int op_changed_between_versions(op_id_t op_id) {
    return get_op_version_diff(op_id) != NULL;
}

// Get number of inputs for operator in specific version
int get_op_inputs_for_version(op_id_t op_id, int major, int minor) {
    const op_version_diff_t* diff = get_op_version_diff(op_id);
    
    if (diff == NULL) {
        // Operator didn't change, use current value
        // This would need to be implemented with current operator registry
        return -1; // Indicate "use current"
    }
    
    if (major == 0 && minor == 7) {
        return diff->v07_inputs;
    } else if (major == 0 && minor >= 8) {
        return diff->v08_inputs;
    }
    
    // Default to current
    return -1;
}

// Get number of outputs for operator in specific version  
int get_op_outputs_for_version(op_id_t op_id, int major, int minor) {
    const op_version_diff_t* diff = get_op_version_diff(op_id);
    
    if (diff == NULL) {
        // Operator didn't change, use current value
        return -1; // Indicate "use current"
    }
    
    if (major == 0 && minor == 7) {
        return diff->v07_outputs;
    } else if (major == 0 && minor >= 8) {
        return diff->v08_outputs;
    }
    
    // Default to current
    return -1;
}