/* dynamic_flash_buffer.c
 * Dynamic Flash Buffer Management Implementation
 * 
 * Provides on-demand allocation for flash operations, eliminating
 * static buffer waste in flash_bees.c
 */

#include "dynamic_flash_buffer.h"
#include "memory.h"
#include "print_funcs.h"
#include <string.h>

//=====================================
//===== Static Variables

static dynamic_flash_buffer_t flash_buffer = {0};
static u8 flash_buffer_system_initialized = 0;

// Statistics tracking
static u32 total_allocations = 0;
static u32 total_deallocations = 0;
static u32 peak_allocated_bytes = 0;
static u32 memory_saved_vs_static = 0;

//=====================================
//===== Implementation

void dynamic_flash_buffer_init(void) {
    if (flash_buffer_system_initialized) {
        print_dbg("\r\n WARNING: Dynamic flash buffer already initialized");
        return;
    }
    
    print_dbg("\r\n Initializing dynamic flash buffer system...");
    
    // Initialize structure
    memset(&flash_buffer, 0, sizeof(dynamic_flash_buffer_t));
    
    // Calculate memory saved vs. static allocation
    // Original: static s32* scalerBuf for 1024 values = 4096 bytes always allocated
    memory_saved_vs_static = FLASH_BUFFER_DEFAULT_SIZE * sizeof(s32);
    
    flash_buffer_system_initialized = 1;
    
    print_dbg("\r\n Dynamic flash buffer initialized:");
    print_dbg("\r\n   Default size: ");
    print_dbg_ulong(FLASH_BUFFER_DEFAULT_SIZE);
    print_dbg(" elements");
    print_dbg("\r\n   Memory saved vs static: ");
    print_dbg_ulong(memory_saved_vs_static);
    print_dbg(" bytes");
}

void dynamic_flash_buffer_deinit(void) {
    if (!flash_buffer_system_initialized) {
        return;
    }
    
    print_dbg("\r\n Deinitializing dynamic flash buffer...");
    
    // Free any allocated buffer
    if (flash_buffer.in_use && flash_buffer.data) {
        dynamic_flash_buffer_free();
    }
    
    flash_buffer_system_initialized = 0;
    
    print_dbg("\r\n Dynamic flash buffer deinitialized");
    print_dbg("\r\n   Total allocations: ");
    print_dbg_ulong(total_allocations);
    print_dbg("\r\n   Total deallocations: ");
    print_dbg_ulong(total_deallocations);
    print_dbg("\r\n   Peak allocated: ");
    print_dbg_ulong(peak_allocated_bytes);
    print_dbg(" bytes");
}

s32* dynamic_flash_buffer_alloc(u32 num_elements) {
    if (!flash_buffer_system_initialized) {
        print_dbg("\r\n ERROR: Flash buffer system not initialized");
        return NULL;
    }
    
    if (flash_buffer.in_use) {
        print_dbg("\r\n WARNING: Flash buffer already allocated, freeing previous");
        dynamic_flash_buffer_free();
    }
    
    if (num_elements == 0) {
        num_elements = FLASH_BUFFER_DEFAULT_SIZE;
    }
    
    if (num_elements > FLASH_BUFFER_MAX_SIZE) {
        print_dbg("\r\n ERROR: Flash buffer size too large: ");
        print_dbg_ulong(num_elements);
        print_dbg(" (max: ");
        print_dbg_ulong(FLASH_BUFFER_MAX_SIZE);
        print_dbg(")");
        return NULL;
    }
    
    u32 bytes_needed = num_elements * sizeof(s32);
    
    print_dbg("\r\n Allocating flash buffer: ");
    print_dbg_ulong(num_elements);
    print_dbg(" elements (");
    print_dbg_ulong(bytes_needed);
    print_dbg(" bytes)");
    
    // Allocate aligned memory
    flash_buffer.data = (s32*)alloc_mem(bytes_needed);
    if (!flash_buffer.data) {
        print_dbg("\r\n ERROR: Failed to allocate flash buffer");
        return NULL;
    }
    
    // Initialize buffer
    memset(flash_buffer.data, 0, bytes_needed);
    
    // Update tracking
    flash_buffer.size = num_elements;
    flash_buffer.allocated_bytes = bytes_needed;
    flash_buffer.in_use = 1;
    
    total_allocations++;
    if (bytes_needed > peak_allocated_bytes) {
        peak_allocated_bytes = bytes_needed;
    }
    
    print_dbg("\r\n Flash buffer allocated at: 0x");
    print_dbg_hex((u32)flash_buffer.data);
    
    return flash_buffer.data;
}

void dynamic_flash_buffer_free(void) {
    if (!flash_buffer_system_initialized) {
        return;
    }
    
    if (!flash_buffer.in_use || !flash_buffer.data) {
        print_dbg("\r\n WARNING: Attempting to free unallocated flash buffer");
        return;
    }
    
    print_dbg("\r\n Freeing flash buffer: ");
    print_dbg_ulong(flash_buffer.allocated_bytes);
    print_dbg(" bytes");
    
    // Free memory
    free_mem((heap_t)flash_buffer.data);
    
    // Reset structure
    flash_buffer.data = NULL;
    flash_buffer.size = 0;
    flash_buffer.allocated_bytes = 0;
    flash_buffer.in_use = 0;
    
    total_deallocations++;
    
    print_dbg("\r\n Flash buffer freed");
}

s32* dynamic_flash_buffer_get(void) {
    if (!flash_buffer_system_initialized || !flash_buffer.in_use) {
        return NULL;
    }
    
    return flash_buffer.data;
}

u8 dynamic_flash_buffer_is_allocated(void) {
    return flash_buffer_system_initialized && flash_buffer.in_use;
}

u32 dynamic_flash_buffer_get_size(void) {
    if (!flash_buffer_system_initialized || !flash_buffer.in_use) {
        return 0;
    }
    
    return flash_buffer.size;
}

u32 dynamic_flash_buffer_get_allocated_bytes(void) {
    if (!flash_buffer_system_initialized || !flash_buffer.in_use) {
        return 0;
    }
    
    return flash_buffer.allocated_bytes;
}

u32 dynamic_flash_buffer_memory_saved(void) {
    return memory_saved_vs_static;
}

//=====================================
//===== Advanced Features

// Resize buffer if needed (preserving data)
s32* dynamic_flash_buffer_resize(u32 new_num_elements) {
    if (!flash_buffer_system_initialized || !flash_buffer.in_use) {
        print_dbg("\r\n ERROR: Cannot resize unallocated buffer");
        return NULL;
    }
    
    if (new_num_elements <= flash_buffer.size) {
        return flash_buffer.data; // Already sufficient
    }
    
    if (new_num_elements > FLASH_BUFFER_MAX_SIZE) {
        print_dbg("\r\n ERROR: Resize too large");
        return NULL;
    }
    
    print_dbg("\r\n Resizing flash buffer from ");
    print_dbg_ulong(flash_buffer.size);
    print_dbg(" to ");
    print_dbg_ulong(new_num_elements);
    print_dbg(" elements");
    
    u32 new_bytes = new_num_elements * sizeof(s32);
    s32* new_data = (s32*)alloc_mem(new_bytes);
    if (!new_data) {
        print_dbg("\r\n ERROR: Failed to resize flash buffer");
        return NULL;
    }
    
    // Copy existing data
    memcpy(new_data, flash_buffer.data, flash_buffer.allocated_bytes);
    
    // Clear new portion
    memset((u8*)new_data + flash_buffer.allocated_bytes, 0, 
           new_bytes - flash_buffer.allocated_bytes);
    
    // Free old buffer
    free_mem((heap_t)flash_buffer.data);
    
    // Update tracking
    flash_buffer.data = new_data;
    flash_buffer.size = new_num_elements;
    flash_buffer.allocated_bytes = new_bytes;
    
    if (new_bytes > peak_allocated_bytes) {
        peak_allocated_bytes = new_bytes;
    }
    
    return flash_buffer.data;
}

// Get buffer statistics for debugging
void dynamic_flash_buffer_print_stats(void) {
    print_dbg("\r\n === Flash Buffer Statistics ===");
    print_dbg("\r\n Initialized: ");
    print_dbg_ulong(flash_buffer_system_initialized);
    print_dbg("\r\n Currently allocated: ");
    print_dbg_ulong(flash_buffer.in_use);
    
    if (flash_buffer.in_use) {
        print_dbg("\r\n Current size: ");
        print_dbg_ulong(flash_buffer.size);
        print_dbg(" elements");
        print_dbg("\r\n Current bytes: ");
        print_dbg_ulong(flash_buffer.allocated_bytes);
        print_dbg("\r\n Buffer address: 0x");
        print_dbg_hex((u32)flash_buffer.data);
    }
    
    print_dbg("\r\n Total allocations: ");
    print_dbg_ulong(total_allocations);
    print_dbg("\r\n Total deallocations: ");
    print_dbg_ulong(total_deallocations);
    print_dbg("\r\n Peak allocated: ");
    print_dbg_ulong(peak_allocated_bytes);
    print_dbg(" bytes");
    print_dbg("\r\n Memory saved vs static: ");
    print_dbg_ulong(memory_saved_vs_static);
    print_dbg(" bytes");
    print_dbg("\r\n ==============================");
}