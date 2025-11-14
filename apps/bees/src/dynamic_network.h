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

// Forward declarations - avoid circular dependencies
struct _inode;
struct _onode;  
struct _pnode;
struct _op;

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

// Function declarations that work with ctlnet_t (defined in net_protected.h)
struct _ctlnet; // Forward declaration

// Initialize dynamic network 
extern struct _ctlnet* dynamic_network_init(void);

// Cleanup dynamic network
extern void dynamic_network_deinit(struct _ctlnet* net);

// Expand arrays when needed
extern int dynamic_network_expand_ops(struct _ctlnet* net);
extern int dynamic_network_expand_ins(struct _ctlnet* net, u16 needed_size);
extern int dynamic_network_expand_outs(struct _ctlnet* net, u16 needed_size);
extern int dynamic_network_expand_params(struct _ctlnet* net, u16 needed_size);

// Get memory usage statistics
extern u32 dynamic_network_memory_usage(const struct _ctlnet* net);

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