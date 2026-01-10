/* Graphics Memory Dynamic Allocation Optimization Header
 * 
 * Provides dynamic allocation for graphics operators' buffers
 * Replaces static allocation with on-demand allocation/deallocation
 */

#ifndef _GRAPHICS_MEMORY_OPTIMIZATION_H_
#define _GRAPHICS_MEMORY_OPTIMIZATION_H_

#include "types.h"
#include <string.h>  // for memset

// Graphics buffer sizes (from original headers)
#define OP_BIGNUM_PX_W 64
#define OP_BIGNUM_PX_H 32
#define OP_BIGNUM_GFX_BYTES (OP_BIGNUM_PX_W * OP_BIGNUM_PX_H)

#define OP_BARS8_PX_W 128
#define OP_BARS8_PX_H 64
#define OP_BARS8_GFX_BYTES (OP_BARS8_PX_W * OP_BARS8_PX_H)

//=== BIGNUM Dynamic Allocation ===

// Allocate graphics buffer for BIGNUM operator
extern u8* op_bignum_alloc_graphics_buffer(void);

// Free graphics buffer for BIGNUM operator
extern void op_bignum_free_graphics_buffer(u8* buffer);

//=== BARS8 Dynamic Allocation ===

// Allocate graphics buffer for BARS8 operator
extern u8* op_bars8_alloc_graphics_buffer(void);

// Free graphics buffer for BARS8 operator  
extern void op_bars8_free_graphics_buffer(u8* buffer);

//=== Configuration ===

// Check if graphics memory optimization is enabled
extern u8 graphics_memory_optimization_enabled(void);

#endif // _GRAPHICS_MEMORY_OPTIMIZATION_H_