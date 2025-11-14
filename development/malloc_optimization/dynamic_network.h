/* dynamic_network.h
 * Dynamic Network Memory Management for BEES
 * 
 * Replaces fixed-size arrays with dynamic allocation to:
 * - Reduce memory waste for small networks
 * - Allow larger networks when memory available
 * - Maintain compatibility with existing code
 */

#ifndef _DYNAMIC_NETWORK_H_
#define _DYNAMIC_NETWORK_H_

#include "types.h"

//=====================================
//===== Configuration

// Enable dynamic network allocation
#define DYNAMIC_NETWORK_ENABLED 1

// Default initial sizes (can grow dynamically)
#define INITIAL_OPS_SIZE    16     // Start with 16 ops (vs 128 fixed)
#define INITIAL_INS_SIZE    64     // Start with 64 inputs (vs 256 fixed) 
#define INITIAL_OUTS_SIZE   64     // Start with 64 outputs (vs 256 fixed)
#define INITIAL_PARAMS_SIZE 64     // Start with 64 params (vs 256 fixed)

// Growth factors
#define GROWTH_FACTOR       2      // Double size when expanding
#define MAX_OPS_LIMIT       256    // Hard limit (double original)
#define MAX_INS_LIMIT       512    // Hard limit (double original)
#define MAX_OUTS_LIMIT      512    // Hard limit (double original)
#define MAX_PARAMS_LIMIT    512    // Hard limit (double original)

//=====================================
//===== Dynamic Network Structure

typedef struct _dynamic_ctlnet {
    // Dynamic arrays
    op_t** ops;           // Dynamic operator array
    inode_t* ins;         // Dynamic input nodes
    onode_t* outs;        // Dynamic output nodes  
    pnode_t* params;      // Dynamic parameter nodes
    
    // Current counts
    u16 numOps;
    u16 numIns;
    u16 numOuts;  
    u16 numParams;
    
    // Allocated capacities
    u16 maxOps;
    u16 maxIns;
    u16 maxOuts;
    u16 maxParams;
    
} dynamic_ctlnet_t;

//=====================================
//===== Function Declarations

// Initialize dynamic network
extern void dynamic_net_init(void);

// Cleanup dynamic network
extern void dynamic_net_deinit(void);

// Expand arrays when needed
extern int dynamic_net_expand_ops(u16 needed_size);
extern int dynamic_net_expand_ins(u16 needed_size);
extern int dynamic_net_expand_outs(u16 needed_size);
extern int dynamic_net_expand_params(u16 needed_size);

// Get memory usage statistics
extern u32 dynamic_net_memory_usage(void);
extern u32 dynamic_net_memory_saved(void);

// Compatibility functions (maintain existing API)
extern int dynamic_net_can_add_op(u16 ins_needed, u16 outs_needed);
extern void dynamic_net_get_stats(u16* ops_used, u16* ops_max, 
                                  u16* ins_used, u16* ins_max,
                                  u16* outs_used, u16* outs_max);

#endif // _DYNAMIC_NETWORK_H_