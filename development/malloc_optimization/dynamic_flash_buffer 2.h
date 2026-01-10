/* dynamic_flash_buffer.h
 * Dynamic Flash Buffer Management for BEES
 * 
 * Replaces static scalerBuf with dynamic allocation to:
 * - Only allocate memory when flash operations needed
 * - Free memory immediately after use
 * - Reduce baseline memory usage
 */

#ifndef _DYNAMIC_FLASH_BUFFER_H_
#define _DYNAMIC_FLASH_BUFFER_H_

#include "types.h"

//=====================================
//===== Configuration

// Enable dynamic flash buffer allocation
#define DYNAMIC_FLASH_BUFFER_ENABLED 1

// Buffer configuration
#define FLASH_BUFFER_DEFAULT_SIZE    1024     // Default size (same as original)
#define FLASH_BUFFER_MAX_SIZE        4096     // Maximum allowed size
#define FLASH_BUFFER_ALIGNMENT       4       // 32-bit alignment for flash operations

//=====================================
//===== Dynamic Flash Buffer Management

typedef struct _dynamic_flash_buffer {
    s32* data;           // Buffer data pointer
    u32 size;            // Current size in elements (s32)
    u32 allocated_bytes; // Total allocated bytes
    u8 in_use;           // Buffer currently allocated flag
} dynamic_flash_buffer_t;

//=====================================
//===== Function Declarations

// Initialize dynamic flash buffer system
extern void dynamic_flash_buffer_init(void);

// Cleanup dynamic flash buffer system  
extern void dynamic_flash_buffer_deinit(void);

// Allocate flash buffer for temporary operations
extern s32* dynamic_flash_buffer_alloc(u32 num_elements);

// Free flash buffer after operations complete
extern void dynamic_flash_buffer_free(void);

// Get current buffer pointer (for compatibility)
extern s32* dynamic_flash_buffer_get(void);

// Check if buffer is currently allocated
extern u8 dynamic_flash_buffer_is_allocated(void);

// Get buffer statistics
extern u32 dynamic_flash_buffer_get_size(void);
extern u32 dynamic_flash_buffer_get_allocated_bytes(void);
extern u32 dynamic_flash_buffer_memory_saved(void);

//=====================================
//===== Convenience Macros for Flash Operations

// Allocate buffer, execute code block, then free buffer
#define WITH_FLASH_BUFFER(size, code_block) do { \
    s32* scalerBuf = dynamic_flash_buffer_alloc(size); \
    if (scalerBuf) { \
        code_block \
        dynamic_flash_buffer_free(); \
    } else { \
        print_dbg("\r\n ERROR: Failed to allocate flash buffer"); \
    } \
} while(0)

// For backward compatibility with existing code
#define FLASH_BUFFER_ALLOC(size) dynamic_flash_buffer_alloc(size)
#define FLASH_BUFFER_FREE() dynamic_flash_buffer_free()
#define FLASH_BUFFER_GET() dynamic_flash_buffer_get()

#endif // _DYNAMIC_FLASH_BUFFER_H_