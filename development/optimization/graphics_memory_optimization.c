/* Graphics Memory Dynamic Allocation Optimization
 * 
 * PROBLEM: BIGNUM/BARS8 operators use static graphics buffers
 * - BIGNUM: 2,048 bytes per instance (64x32 pixels)
 * - BARS8: 8,192 bytes per instance (128x64 pixels)  
 * - Memory allocated even when operators are disabled/unused
 * - Comments: "FIXME: this should probly be alloc'd/freed from heap"
 * 
 * SOLUTION: Dynamic allocation on enable, deallocation on disable
 * - Allocate graphics buffer only when operator is enabled
 * - Free graphics buffer when operator is disabled or removed
 * - Expected 88% memory reduction for inactive graphics operators
 */

#include "memory.h"
#include "print_funcs.h"
#include "graphics_memory_optimization.h"

// Dynamic graphics buffer allocation for BIGNUM operator
u8* op_bignum_alloc_graphics_buffer(void) {
    u8* buffer = (u8*)alloc_mem(OP_BIGNUM_GFX_BYTES);
    if (buffer == NULL) {
        print_dbg("\r\n[GFX_OPT] BIGNUM: Failed to allocate graphics buffer");
        return NULL;
    }
    
    print_dbg("\r\n[GFX_OPT] BIGNUM: Allocated ");
    print_dbg_ulong(OP_BIGNUM_GFX_BYTES);
    print_dbg(" bytes graphics buffer");
    
    // Clear the buffer (equivalent to region_fill(&reg, 0))
    memset((void*)buffer, 0, OP_BIGNUM_GFX_BYTES);
    
    return buffer;
}

// Dynamic graphics buffer deallocation for BIGNUM operator
void op_bignum_free_graphics_buffer(u8* buffer) {
    if (buffer != NULL) {
        free_mem((heap_t)buffer);
        print_dbg("\r\n[GFX_OPT] BIGNUM: Freed graphics buffer");
    }
}

// Dynamic graphics buffer allocation for BARS8 operator
u8* op_bars8_alloc_graphics_buffer(void) {
    u8* buffer = (u8*)alloc_mem(OP_BARS8_GFX_BYTES);
    if (buffer == NULL) {
        print_dbg("\r\n[GFX_OPT] BARS8: Failed to allocate graphics buffer");
        return NULL;
    }
    
    print_dbg("\r\n[GFX_OPT] BARS8: Allocated ");
    print_dbg_ulong(OP_BARS8_GFX_BYTES);
    print_dbg(" bytes graphics buffer");
    
    // Clear the buffer
    memset((void*)buffer, 0, OP_BARS8_GFX_BYTES);
    
    return buffer;
}

// Dynamic graphics buffer deallocation for BARS8 operator
void op_bars8_free_graphics_buffer(u8* buffer) {
    if (buffer != NULL) {
        free_mem((heap_t)buffer);
        print_dbg("\r\n[GFX_OPT] BARS8: Freed graphics buffer");
    }
}

// Helper function to check if graphics memory optimization is enabled
// This allows gradual rollout and testing
u8 graphics_memory_optimization_enabled(void) {
    // For now, always enabled. Could be made configurable via build flag
    // #ifdef GRAPHICS_MEMORY_OPTIMIZATION_ENABLED
    return 1;
    // #else
    // return 0;
    // #endif
}