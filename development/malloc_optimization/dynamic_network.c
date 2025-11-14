/* dynamic_network.c
 * Dynamic Network Memory Management Implementation
 * 
 * Provides dynamic allocation for network arrays while maintaining
 * compatibility with existing BEES network code.
 */

#include "dynamic_network.h"
#include "memory.h"
#include "print_funcs.h"
#include "net_protected.h"
#include <string.h>

//=====================================
//===== Static Variables

static dynamic_ctlnet_t* dynamic_net = NULL;
static u8 dynamic_network_initialized = 0;

// Statistics tracking
static u32 total_reallocations = 0;
static u32 memory_saved_bytes = 0;

//=====================================
//===== Implementation

void dynamic_net_init(void) {
    if (dynamic_network_initialized) {
        print_dbg("\r\n ERROR: Dynamic network already initialized");
        return;
    }
    
    print_dbg("\r\n Initializing dynamic network allocation...");
    
    // Allocate main structure
    dynamic_net = (dynamic_ctlnet_t*)alloc_mem(sizeof(dynamic_ctlnet_t));
    if (!dynamic_net) {
        print_dbg("\r\n ERROR: Failed to allocate dynamic network structure");
        return;
    }
    
    // Initialize counts
    dynamic_net->numOps = 0;
    dynamic_net->numIns = 0;
    dynamic_net->numOuts = 0;
    dynamic_net->numParams = 0;
    
    // Allocate initial arrays
    dynamic_net->maxOps = INITIAL_OPS_SIZE;
    dynamic_net->ops = (op_t**)alloc_mem(sizeof(op_t*) * INITIAL_OPS_SIZE);
    
    dynamic_net->maxIns = INITIAL_INS_SIZE;
    dynamic_net->ins = (inode_t*)alloc_mem(sizeof(inode_t) * INITIAL_INS_SIZE);
    
    dynamic_net->maxOuts = INITIAL_OUTS_SIZE;
    dynamic_net->outs = (onode_t*)alloc_mem(sizeof(onode_t) * INITIAL_OUTS_SIZE);
    
    dynamic_net->maxParams = INITIAL_PARAMS_SIZE;
    dynamic_net->params = (pnode_t*)alloc_mem(sizeof(pnode_t) * INITIAL_PARAMS_SIZE);
    
    // Check allocations
    if (!dynamic_net->ops || !dynamic_net->ins || !dynamic_net->outs || !dynamic_net->params) {
        print_dbg("\r\n ERROR: Failed to allocate dynamic network arrays");
        dynamic_net_deinit();
        return;
    }
    
    // Initialize arrays
    memset(dynamic_net->ops, 0, sizeof(op_t*) * INITIAL_OPS_SIZE);
    memset(dynamic_net->ins, 0, sizeof(inode_t) * INITIAL_INS_SIZE);
    memset(dynamic_net->outs, 0, sizeof(onode_t) * INITIAL_OUTS_SIZE);
    memset(dynamic_net->params, 0, sizeof(pnode_t) * INITIAL_PARAMS_SIZE);
    
    // Calculate memory saved vs. fixed allocation
    u32 fixed_memory = (NET_OPS_MAX * sizeof(op_t*)) +
                      (NET_INS_MAX * sizeof(inode_t)) + 
                      (NET_OUTS_MAX * sizeof(onode_t)) +
                      (NET_PARAMS_MAX * sizeof(pnode_t));
                      
    u32 dynamic_memory = (INITIAL_OPS_SIZE * sizeof(op_t*)) +
                        (INITIAL_INS_SIZE * sizeof(inode_t)) +
                        (INITIAL_OUTS_SIZE * sizeof(onode_t)) +
                        (INITIAL_PARAMS_SIZE * sizeof(pnode_t));
                        
    memory_saved_bytes = fixed_memory - dynamic_memory;
    
    dynamic_network_initialized = 1;
    
    print_dbg("\r\n Dynamic network initialized:");
    print_dbg("\r\n   Initial ops capacity: ");
    print_dbg_ulong(INITIAL_OPS_SIZE);
    print_dbg("\r\n   Initial ins capacity: ");
    print_dbg_ulong(INITIAL_INS_SIZE);
    print_dbg("\r\n   Initial outs capacity: ");
    print_dbg_ulong(INITIAL_OUTS_SIZE);
    print_dbg("\r\n   Initial params capacity: ");
    print_dbg_ulong(INITIAL_PARAMS_SIZE);
    print_dbg("\r\n   Memory saved: ");
    print_dbg_ulong(memory_saved_bytes);
    print_dbg(" bytes");
}

void dynamic_net_deinit(void) {
    if (!dynamic_network_initialized || !dynamic_net) {
        return;
    }
    
    print_dbg("\r\n Deinitializing dynamic network...");
    
    // Free arrays
    if (dynamic_net->ops) free_mem((heap_t)dynamic_net->ops);
    if (dynamic_net->ins) free_mem((heap_t)dynamic_net->ins);
    if (dynamic_net->outs) free_mem((heap_t)dynamic_net->outs);  
    if (dynamic_net->params) free_mem((heap_t)dynamic_net->params);
    
    // Free main structure
    free_mem((heap_t)dynamic_net);
    dynamic_net = NULL;
    
    dynamic_network_initialized = 0;
    
    print_dbg("\r\n Dynamic network deinitialized");
}

int dynamic_net_expand_ops(u16 needed_size) {
    if (!dynamic_network_initialized || !dynamic_net) {
        return -1;
    }
    
    if (needed_size <= dynamic_net->maxOps) {
        return 0; // Already sufficient
    }
    
    u16 new_size = dynamic_net->maxOps * GROWTH_FACTOR;
    while (new_size < needed_size) {
        new_size *= GROWTH_FACTOR;
    }
    
    if (new_size > MAX_OPS_LIMIT) {
        new_size = MAX_OPS_LIMIT;
        if (needed_size > MAX_OPS_LIMIT) {
            print_dbg("\r\n ERROR: Ops limit exceeded");
            return -1;
        }
    }
    
    print_dbg("\r\n Expanding ops array from ");
    print_dbg_ulong(dynamic_net->maxOps);
    print_dbg(" to ");
    print_dbg_ulong(new_size);
    
    // Allocate new array
    op_t** new_ops = (op_t**)alloc_mem(sizeof(op_t*) * new_size);
    if (!new_ops) {
        print_dbg("\r\n ERROR: Failed to expand ops array");
        return -1;
    }
    
    // Copy existing data
    memcpy(new_ops, dynamic_net->ops, sizeof(op_t*) * dynamic_net->maxOps);
    memset(new_ops + dynamic_net->maxOps, 0, sizeof(op_t*) * (new_size - dynamic_net->maxOps));
    
    // Free old array and update
    free_mem((heap_t)dynamic_net->ops);
    dynamic_net->ops = new_ops;
    dynamic_net->maxOps = new_size;
    
    total_reallocations++;
    return 0;
}

int dynamic_net_expand_ins(u16 needed_size) {
    if (!dynamic_network_initialized || !dynamic_net) {
        return -1;
    }
    
    if (needed_size <= dynamic_net->maxIns) {
        return 0;
    }
    
    u16 new_size = dynamic_net->maxIns * GROWTH_FACTOR;
    while (new_size < needed_size) {
        new_size *= GROWTH_FACTOR;
    }
    
    if (new_size > MAX_INS_LIMIT) {
        new_size = MAX_INS_LIMIT;
        if (needed_size > MAX_INS_LIMIT) {
            print_dbg("\r\n ERROR: Ins limit exceeded");
            return -1;
        }
    }
    
    print_dbg("\r\n Expanding ins array from ");
    print_dbg_ulong(dynamic_net->maxIns);
    print_dbg(" to ");
    print_dbg_ulong(new_size);
    
    inode_t* new_ins = (inode_t*)alloc_mem(sizeof(inode_t) * new_size);
    if (!new_ins) {
        print_dbg("\r\n ERROR: Failed to expand ins array");
        return -1;
    }
    
    memcpy(new_ins, dynamic_net->ins, sizeof(inode_t) * dynamic_net->maxIns);
    memset(new_ins + dynamic_net->maxIns, 0, sizeof(inode_t) * (new_size - dynamic_net->maxIns));
    
    free_mem((heap_t)dynamic_net->ins);
    dynamic_net->ins = new_ins;
    dynamic_net->maxIns = new_size;
    
    total_reallocations++;
    return 0;
}

int dynamic_net_expand_outs(u16 needed_size) {
    if (!dynamic_network_initialized || !dynamic_net) {
        return -1;
    }
    
    if (needed_size <= dynamic_net->maxOuts) {
        return 0;
    }
    
    u16 new_size = dynamic_net->maxOuts * GROWTH_FACTOR;
    while (new_size < needed_size) {
        new_size *= GROWTH_FACTOR;
    }
    
    if (new_size > MAX_OUTS_LIMIT) {
        new_size = MAX_OUTS_LIMIT;
        if (needed_size > MAX_OUTS_LIMIT) {
            print_dbg("\r\n ERROR: Outs limit exceeded");
            return -1;
        }
    }
    
    print_dbg("\r\n Expanding outs array from ");
    print_dbg_ulong(dynamic_net->maxOuts);
    print_dbg(" to ");
    print_dbg_ulong(new_size);
    
    onode_t* new_outs = (onode_t*)alloc_mem(sizeof(onode_t) * new_size);
    if (!new_outs) {
        print_dbg("\r\n ERROR: Failed to expand outs array");
        return -1;
    }
    
    memcpy(new_outs, dynamic_net->outs, sizeof(onode_t) * dynamic_net->maxOuts);
    memset(new_outs + dynamic_net->maxOuts, 0, sizeof(onode_t) * (new_size - dynamic_net->maxOuts));
    
    free_mem((heap_t)dynamic_net->outs);
    dynamic_net->outs = new_outs;
    dynamic_net->maxOuts = new_size;
    
    total_reallocations++;
    return 0;
}

int dynamic_net_expand_params(u16 needed_size) {
    if (!dynamic_network_initialized || !dynamic_net) {
        return -1;
    }
    
    if (needed_size <= dynamic_net->maxParams) {
        return 0;
    }
    
    u16 new_size = dynamic_net->maxParams * GROWTH_FACTOR;
    while (new_size < needed_size) {
        new_size *= GROWTH_FACTOR;
    }
    
    if (new_size > MAX_PARAMS_LIMIT) {
        new_size = MAX_PARAMS_LIMIT;
        if (needed_size > MAX_PARAMS_LIMIT) {
            print_dbg("\r\n ERROR: Params limit exceeded");
            return -1;
        }
    }
    
    print_dbg("\r\n Expanding params array from ");
    print_dbg_ulong(dynamic_net->maxParams);
    print_dbg(" to ");
    print_dbg_ulong(new_size);
    
    pnode_t* new_params = (pnode_t*)alloc_mem(sizeof(pnode_t) * new_size);
    if (!new_params) {
        print_dbg("\r\n ERROR: Failed to expand params array");
        return -1;
    }
    
    memcpy(new_params, dynamic_net->params, sizeof(pnode_t) * dynamic_net->maxParams);
    memset(new_params + dynamic_net->maxParams, 0, sizeof(pnode_t) * (new_size - dynamic_net->maxParams));
    
    free_mem((heap_t)dynamic_net->params);
    dynamic_net->params = new_params;
    dynamic_net->maxParams = new_size;
    
    total_reallocations++;
    return 0;
}

int dynamic_net_can_add_op(u16 ins_needed, u16 outs_needed) {
    if (!dynamic_network_initialized || !dynamic_net) {
        return 0;
    }
    
    // Check if we can add one more op
    if (dynamic_net->numOps + 1 > MAX_OPS_LIMIT) {
        return 0;
    }
    
    // Check if we can add required ins/outs
    if (dynamic_net->numIns + ins_needed > MAX_INS_LIMIT) {
        return 0;
    }
    
    if (dynamic_net->numOuts + outs_needed > MAX_OUTS_LIMIT) {
        return 0;
    }
    
    return 1;
}

u32 dynamic_net_memory_usage(void) {
    if (!dynamic_network_initialized || !dynamic_net) {
        return 0;
    }
    
    return (dynamic_net->maxOps * sizeof(op_t*)) +
           (dynamic_net->maxIns * sizeof(inode_t)) +
           (dynamic_net->maxOuts * sizeof(onode_t)) +
           (dynamic_net->maxParams * sizeof(pnode_t)) +
           sizeof(dynamic_ctlnet_t);
}

u32 dynamic_net_memory_saved(void) {
    return memory_saved_bytes;
}

void dynamic_net_get_stats(u16* ops_used, u16* ops_max, 
                          u16* ins_used, u16* ins_max,
                          u16* outs_used, u16* outs_max) {
    if (!dynamic_network_initialized || !dynamic_net) {
        *ops_used = *ops_max = 0;
        *ins_used = *ins_max = 0;
        *outs_used = *outs_max = 0;
        return;
    }
    
    *ops_used = dynamic_net->numOps;
    *ops_max = dynamic_net->maxOps;
    *ins_used = dynamic_net->numIns;
    *ins_max = dynamic_net->maxIns;
    *outs_used = dynamic_net->numOuts;
    *outs_max = dynamic_net->maxOuts;
}

// Accessor functions for external compatibility
dynamic_ctlnet_t* get_dynamic_net(void) {
    return dynamic_net;
}

u8 is_dynamic_network_enabled(void) {
    return dynamic_network_initialized;
}