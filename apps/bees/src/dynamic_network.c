/* dynamic_network.c
 * Dynamic Network Memory Management Implementation
 */

#include "dynamic_network.h"
#include "net_protected.h"
#include "memory.h"
#include "print_funcs.h"
#include <string.h>

// Constants
#define INITIAL_OPS_SIZE    16     
#define INITIAL_INS_SIZE    64     
#define INITIAL_OUTS_SIZE   64     
#define INITIAL_PARAMS_SIZE 64     
#define GROWTH_FACTOR       2      
#define MAX_OPS_LIMIT       256    
#define MAX_INS_LIMIT       512    
#define MAX_OUTS_LIMIT      512    
#define MAX_PARAMS_LIMIT    512    

ctlnet_t* dynamic_network_init(void) {
    ctlnet_t* net = (ctlnet_t*)alloc_mem(sizeof(ctlnet_t));
    if (!net) return NULL;
    
    net->numOps = 0;
    net->numIns = 0;
    net->numOuts = 0;
    net->numParams = 0;
    net->opsCapacity = INITIAL_OPS_SIZE;
    net->insCapacity = INITIAL_INS_SIZE;
    net->outsCapacity = INITIAL_OUTS_SIZE;
    net->paramsCapacity = INITIAL_PARAMS_SIZE;
    
    net->ops = (op_t**)alloc_mem(sizeof(op_t*) * INITIAL_OPS_SIZE);
    net->ins = (inode_t*)alloc_mem(sizeof(inode_t) * INITIAL_INS_SIZE);
    net->outs = (onode_t*)alloc_mem(sizeof(onode_t) * INITIAL_OUTS_SIZE);
    net->params = (pnode_t*)alloc_mem(sizeof(pnode_t) * INITIAL_PARAMS_SIZE);
    
    if (!net->ops || !net->ins || !net->outs || !net->params) {
        if (net->ops) free_mem((heap_t)net->ops);
        if (net->ins) free_mem((heap_t)net->ins);
        if (net->outs) free_mem((heap_t)net->outs);
        if (net->params) free_mem((heap_t)net->params);
        free_mem((heap_t)net);
        return NULL;
    }
    
    memset(net->ops, 0, sizeof(op_t*) * INITIAL_OPS_SIZE);
    memset(net->ins, 0, sizeof(inode_t) * INITIAL_INS_SIZE);
    memset(net->outs, 0, sizeof(onode_t) * INITIAL_OUTS_SIZE);
    memset(net->params, 0, sizeof(pnode_t) * INITIAL_PARAMS_SIZE);
    
    return net;
}

void dynamic_network_deinit(ctlnet_t* net) {
    if (!net) return;
    if (net->ops) free_mem((heap_t)net->ops);
    if (net->ins) free_mem((heap_t)net->ins);
    if (net->outs) free_mem((heap_t)net->outs);
    if (net->params) free_mem((heap_t)net->params);
    free_mem((heap_t)net);
}

int dynamic_network_expand_ops(ctlnet_t* net) {
    if (!net) return -1;
    u16 new_capacity = net->opsCapacity * GROWTH_FACTOR;
    if (new_capacity > MAX_OPS_LIMIT) new_capacity = MAX_OPS_LIMIT;
    if (new_capacity <= net->opsCapacity) return -1;
    
    op_t** new_ops = (op_t**)alloc_mem(sizeof(op_t*) * new_capacity);
    if (!new_ops) return -1;
    
    memcpy(new_ops, net->ops, sizeof(op_t*) * net->opsCapacity);
    memset(new_ops + net->opsCapacity, 0, sizeof(op_t*) * (new_capacity - net->opsCapacity));
    
    free_mem((heap_t)net->ops);
    net->ops = new_ops;
    net->opsCapacity = new_capacity;
    return 0;
}

int dynamic_network_expand_ins(ctlnet_t* net, u16 needed) {
    if (!net) return -1;
    u16 new_capacity = net->insCapacity;
    while (new_capacity < needed && new_capacity < MAX_INS_LIMIT) new_capacity *= GROWTH_FACTOR;
    if (new_capacity > MAX_INS_LIMIT) new_capacity = MAX_INS_LIMIT;
    if (new_capacity < needed || new_capacity <= net->insCapacity) return (new_capacity < needed) ? -1 : 0;
    
    inode_t* new_ins = (inode_t*)alloc_mem(sizeof(inode_t) * new_capacity);
    if (!new_ins) return -1;
    
    memcpy(new_ins, net->ins, sizeof(inode_t) * net->insCapacity);
    memset(new_ins + net->insCapacity, 0, sizeof(inode_t) * (new_capacity - net->insCapacity));
    
    free_mem((heap_t)net->ins);
    net->ins = new_ins;
    net->insCapacity = new_capacity;
    return 0;
}

int dynamic_network_expand_outs(ctlnet_t* net, u16 needed) {
    if (!net) return -1;
    u16 new_capacity = net->outsCapacity;
    while (new_capacity < needed && new_capacity < MAX_OUTS_LIMIT) new_capacity *= GROWTH_FACTOR;
    if (new_capacity > MAX_OUTS_LIMIT) new_capacity = MAX_OUTS_LIMIT;
    if (new_capacity < needed || new_capacity <= net->outsCapacity) return (new_capacity < needed) ? -1 : 0;
    
    onode_t* new_outs = (onode_t*)alloc_mem(sizeof(onode_t) * new_capacity);
    if (!new_outs) return -1;
    
    memcpy(new_outs, net->outs, sizeof(onode_t) * net->outsCapacity);
    memset(new_outs + net->outsCapacity, 0, sizeof(onode_t) * (new_capacity - net->outsCapacity));
    
    free_mem((heap_t)net->outs);
    net->outs = new_outs;
    net->outsCapacity = new_capacity;
    return 0;
}

int dynamic_network_expand_params(ctlnet_t* net, u16 needed) {
    if (!net) return -1;
    u16 new_capacity = net->paramsCapacity;
    while (new_capacity < needed && new_capacity < MAX_PARAMS_LIMIT) new_capacity *= GROWTH_FACTOR;
    if (new_capacity > MAX_PARAMS_LIMIT) new_capacity = MAX_PARAMS_LIMIT;
    if (new_capacity < needed || new_capacity <= net->paramsCapacity) return (new_capacity < needed) ? -1 : 0;
    
    pnode_t* new_params = (pnode_t*)alloc_mem(sizeof(pnode_t) * new_capacity);
    if (!new_params) return -1;
    
    memcpy(new_params, net->params, sizeof(pnode_t) * net->paramsCapacity);
    memset(new_params + net->paramsCapacity, 0, sizeof(pnode_t) * (new_capacity - net->paramsCapacity));
    
    free_mem((heap_t)net->params);
    net->params = new_params;
    net->paramsCapacity = new_capacity;
    return 0;
}

u32 dynamic_network_memory_usage(const ctlnet_t* net) {
    if (!net) return 0;
    return sizeof(ctlnet_t) + 
           sizeof(op_t*) * net->opsCapacity +
           sizeof(inode_t) * net->insCapacity +
           sizeof(onode_t) * net->outsCapacity +
           sizeof(pnode_t) * net->paramsCapacity;
}
