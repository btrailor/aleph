/* Net Operation Index Optimization Header
 * 
 * Provides O(1) lookup table optimization for net_op_in_idx function
 * Replaces linear search with direct array lookup
 */

#ifndef _NET_OPTIMIZATION_H_
#define _NET_OPTIMIZATION_H_

#include "types.h"
#include "net.h"

// Enable debug output for optimization
// #define NET_OPT_DEBUG 1

//=== Lookup Table Management ===

// Initialize lookup table from existing network state
extern void net_optimize_init_lookup_table(void);

// Update lookup table when operator is added
extern void net_optimize_op_added(uint16_t op_idx, uint16_t first_input_idx, uint16_t num_inputs);

// Update lookup table when operator is removed  
extern void net_optimize_op_removed(uint16_t op_idx);

// Rebuild lookup table after major network changes
extern void net_optimize_rebuild_lookup_table(void);

//=== Optimized Functions ===

// Optimized O(1) version of net_op_in_idx
extern uint16_t net_op_in_idx_optimized(const uint16_t op_idx, const uint16_t in_idx);

// Reference implementation for testing/comparison
extern uint16_t net_op_in_idx_reference(const uint16_t op_idx, const uint16_t in_idx);

//=== Testing & Verification ===

// Verify lookup table correctness against reference implementation
extern uint8_t net_optimize_verify_lookup_table(void);

#endif // _NET_OPTIMIZATION_H_