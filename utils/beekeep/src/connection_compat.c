#include "connection_compat.h"
#include <stdio.h>

// Get connection remapping for a specific operator
const connection_remap_t* get_connection_remap(op_id_t op_id) {
    for (int i = 0; i < num_connection_remaps; i++) {
        if (connection_remaps[i].op_id == op_id) {
            return &connection_remaps[i];
        }
    }
    return NULL;
}

// Remap input index from v0.7 to v0.8
int remap_input_index(op_id_t op_id, int v07_index) {
    const connection_remap_t* remap = get_connection_remap(op_id);
    
    if (remap == NULL) {
        // Operator didn't change, use same index
        return v07_index;
    }
    
    if (v07_index < 0 || v07_index >= 8) {
        // Invalid index
        return -1;
    }
    
    return remap->input_remap[v07_index];
}

// Remap output index from v0.7 to v0.8
int remap_output_index(op_id_t op_id, int v07_index) {
    const connection_remap_t* remap = get_connection_remap(op_id);
    
    if (remap == NULL) {
        // Operator didn't change, use same index
        return v07_index;
    }
    
    if (v07_index < 0 || v07_index >= 8) {
        // Invalid index
        return -1;
    }
    
    return remap->output_remap[v07_index];
}