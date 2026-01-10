/* Net Operation Index Optimization
 * 
 * PROBLEM: net_op_in_idx() uses linear search through all network inputs
 * - Current: O(n) complexity where n = total inputs (up to 256)
 * - Called frequently during network operations
 * - Marked "pretty slow" in multiple FIXME comments
 * 
 * SOLUTION: Add lookup table mapping operator index to first input index
 * - New: O(1) complexity for input lookups
 * - Maintain table during operator add/remove operations
 * - Expected 10x performance improvement
 */

#include "net_optimization.h"
#include "net_protected.h"
#include "print_funcs.h"

// Lookup table: operator index -> first input index
// -1 indicates operator has no inputs
static int16_t op_first_input_idx[NET_OPS_MAX];
static uint8_t lookup_table_initialized = 0;

// Initialize the lookup table by scanning existing network
void net_optimize_init_lookup_table(void) {
    uint16_t op_idx, in_idx;
    
    // Initialize all entries to -1 (no inputs)
    for (op_idx = 0; op_idx < NET_OPS_MAX; op_idx++) {
        op_first_input_idx[op_idx] = -1;
    }
    
    // Scan existing inputs to build lookup table
    for (in_idx = 0; in_idx < net->numIns; in_idx++) {
        op_idx = net->ins[in_idx].opIdx;
        
        // If this is the first input we've seen for this operator
        if (op_first_input_idx[op_idx] == -1) {
            op_first_input_idx[op_idx] = in_idx;
        }
    }
    
    lookup_table_initialized = 1;
    
    print_dbg("\r\n[NET_OPT] Lookup table initialized for ");
    print_dbg_ulong(net->numOps);
    print_dbg(" operators, ");
    print_dbg_ulong(net->numIns);
    print_dbg(" inputs");
}

// Update lookup table when operator is added
void net_optimize_op_added(uint16_t op_idx, uint16_t first_input_idx, uint16_t num_inputs) {
    if (!lookup_table_initialized) {
        net_optimize_init_lookup_table();
        return;
    }
    
    if (num_inputs > 0) {
        op_first_input_idx[op_idx] = first_input_idx;
    } else {
        op_first_input_idx[op_idx] = -1;
    }
    
#ifdef NET_OPT_DEBUG
    print_dbg("\r\n[NET_OPT] Op ");
    print_dbg_ulong(op_idx);
    print_dbg(" added, first_input_idx=");
    print_dbg_ulong(first_input_idx);
    print_dbg(", num_inputs=");
    print_dbg_ulong(num_inputs);
#endif
}

// Update lookup table when operator is removed
void net_optimize_op_removed(uint16_t op_idx) {
    if (!lookup_table_initialized) return;
    
    op_first_input_idx[op_idx] = -1;
    
#ifdef NET_OPT_DEBUG
    print_dbg("\r\n[NET_OPT] Op ");
    print_dbg_ulong(op_idx);
    print_dbg(" removed from lookup table");
#endif
}

// Rebuild lookup table after major network changes
void net_optimize_rebuild_lookup_table(void) {
    print_dbg("\r\n[NET_OPT] Rebuilding lookup table...");
    net_optimize_init_lookup_table();
}

// Optimized version of net_op_in_idx using lookup table
uint16_t net_op_in_idx_optimized(const uint16_t op_idx, const uint16_t in_idx) {
    if (!lookup_table_initialized) {
        net_optimize_init_lookup_table();
    }
    
    if (op_idx >= NET_OPS_MAX) {
        return 0;  // Invalid operator index
    }
    
    int16_t first_input = op_first_input_idx[op_idx];
    if (first_input == -1) {
        return 0;  // Operator has no inputs
    }
    
    return first_input + in_idx;
}

// Verify lookup table correctness (for testing)
uint8_t net_optimize_verify_lookup_table(void) {
    uint16_t op_idx, in_idx;
    uint8_t errors = 0;
    
    for (op_idx = 0; op_idx < net->numOps; op_idx++) {
        // Test first input (in_idx = 0)
        uint16_t reference_result = net_op_in_idx_reference(op_idx, 0);
        uint16_t optimized_result = net_op_in_idx_optimized(op_idx, 0);
        
        if (reference_result != optimized_result) {
            print_dbg("\r\n[NET_OPT] VERIFY FAILED: op=");
            print_dbg_ulong(op_idx);
            print_dbg(" ref=");
            print_dbg_ulong(reference_result);
            print_dbg(" opt=");
            print_dbg_ulong(optimized_result);
            errors++;
        }
    }
    
    if (errors == 0) {
        print_dbg("\r\n[NET_OPT] Lookup table verification PASSED");
    } else {
        print_dbg("\r\n[NET_OPT] Lookup table verification FAILED: ");
        print_dbg_ulong(errors);
        print_dbg(" errors");
    }
    
    return errors == 0;
}

// Reference implementation (original linear search)
uint16_t net_op_in_idx_reference(const uint16_t op_idx, const uint16_t in_idx) {
    uint16_t which;
    for (which = 0; which < net->numIns; which++) {
        if (net->ins[which].opIdx == op_idx) {
            return (which + in_idx);
        }
    }
    return 0; // get here if op has no inputs
}