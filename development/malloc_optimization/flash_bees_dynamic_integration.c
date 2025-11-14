/* flash_bees_dynamic_integration.c
 * Example integration of dynamic flash buffer into flash_bees.c
 * 
 * Shows how to replace static scalerBuf with dynamic allocation
 */

#include "dynamic_flash_buffer.h" 
#include "../apps/bees/src/flash_bees.h"
#include "param_scaler.h"
#include "files.h"
#include "flashc.h"
#include "print_funcs.h"

//=====================================
//===== Modified Flash Initialization

// BEFORE: Static buffer allocation
// static s32* scalerBuf;  // <-- This line would be removed

// AFTER: Dynamic buffer system
// No static allocation needed!

void flash_init_scaler_data_dynamic(void) {
    // Initialize dynamic buffer system
    dynamic_flash_buffer_init();
    
    ///// for each param type, get number of offline data bytes,
    ///// load file and write to flash if necessary
    u32 b;
    const char* path;
    u8 p;
    u8* dst;
    void* scalerBytes;

    print_dbg("\r\n initializing NV data for param scaling (DYNAMIC), total bytes: ");
    print_dbg_ulong(PARAM_SCALER_DATA_SIZE);

    print_dbg("\r\n application data address in flash: 0x");
    print_dbg_hex((u32)flash_app_data());
    print_dbg("\r\n scaler data address: 0x");
    print_dbg_hex((u32)&(((beesFlashData*)(flash_app_data()))->scalerBytes));

    scalerBytes = (void*)&(((beesFlashData*)(flash_app_data()))->scalerBytes);

    for(p = 0; p < eParamTypesCount; p++) {
        b = scaler_get_data_bytes(p);
        if(b > 0) {
            print_dbg("\r\n processing param type: ");
            print_dbg_ulong(p);
            print_dbg(", data bytes: ");
            print_dbg_ulong(b);
            
            path = scaler_get_data_path(p);
            print_dbg("\r\n loading scaler data from file: ");
            print_dbg(path);
            
            // ===== DYNAMIC ALLOCATION APPROACH =====
            // Allocate buffer only when needed
            u32 elements_needed = (b >> 2); // Convert bytes to s32 elements
            
            WITH_FLASH_BUFFER(elements_needed, {
                s32* scalerBuf = dynamic_flash_buffer_get();
                
                // Load file into temporary buffer
                files_load_scaler_data(path, scalerBuf, elements_needed);
                
                // Write to flash
                dst = (void*)scaler_get_nv_data(p);
                print_dbg("\r\n writing scaler val data to flash at address: 0x");
                print_dbg_hex((u32)dst);
                flashc_memcpy(dst, (void*)scalerBuf, b, true);
                
                // Buffer automatically freed at end of WITH_FLASH_BUFFER block
            });
            // ===== END DYNAMIC ALLOCATION =====
        }
        
        b = scaler_get_rep_bytes(p);
        if(b > 0) {
            path = scaler_get_rep_path(p);
            print_dbg("\r\n writing scaler representation data to flash: ");
            print_dbg(path);
            
            // ===== DYNAMIC ALLOCATION APPROACH =====
            u32 elements_needed = (b >> 2);
            
            WITH_FLASH_BUFFER(elements_needed, {
                s32* scalerBuf = dynamic_flash_buffer_get();
                
                // Load representation file  
                files_load_scaler_name(path, scalerBuf, elements_needed);
                
                // Write to flash
                dst = (void*)scaler_get_nv_rep(p);
                print_dbg("\r\n writing scaler rep data to flash at address: 0x");
                print_dbg_hex((u32)dst);
                flashc_memcpy(dst, (void*)scalerBuf, b, true);
                
                // Buffer automatically freed
            });
            // ===== END DYNAMIC ALLOCATION =====
        }
    }
    
    // Clean up dynamic buffer system
    dynamic_flash_buffer_deinit();
    
    print_dbg("\r\n scaler data initialization complete (DYNAMIC)");
}

//=====================================
//===== Alternative Manual Approach

void flash_init_scaler_data_manual_dynamic(void) {
    // Initialize dynamic buffer system
    dynamic_flash_buffer_init();
    
    u32 b;
    const char* path;
    u8 p;
    u8* dst;
    void* scalerBytes;
    s32* scalerBuf = NULL;

    scalerBytes = (void*)&(((beesFlashData*)(flash_app_data()))->scalerBytes);

    for(p = 0; p < eParamTypesCount; p++) {
        b = scaler_get_data_bytes(p);
        if(b > 0) {
            // Allocate buffer for this operation
            u32 elements_needed = (b >> 2);
            scalerBuf = dynamic_flash_buffer_alloc(elements_needed);
            
            if (scalerBuf) {
                path = scaler_get_data_path(p);
                files_load_scaler_data(path, scalerBuf, elements_needed);
                
                dst = (void*)scaler_get_nv_data(p);
                flashc_memcpy(dst, (void*)scalerBuf, b, true);
                
                // Free buffer after each operation
                dynamic_flash_buffer_free();
            } else {
                print_dbg("\r\n ERROR: Failed to allocate flash buffer for param ");
                print_dbg_ulong(p);
            }
        }
        
        // Repeat for representation data...
        b = scaler_get_rep_bytes(p);
        if(b > 0) {
            u32 elements_needed = (b >> 2);
            scalerBuf = dynamic_flash_buffer_alloc(elements_needed);
            
            if (scalerBuf) {
                path = scaler_get_rep_path(p);
                files_load_scaler_name(path, scalerBuf, elements_needed);
                
                dst = (void*)scaler_get_nv_rep(p);
                flashc_memcpy(dst, (void*)scalerBuf, b, true);
                
                dynamic_flash_buffer_free();
            }
        }
    }
    
    dynamic_flash_buffer_deinit();
}

//=====================================
//===== Memory Usage Comparison

void flash_print_memory_comparison(void) {
    print_dbg("\r\n === Flash Buffer Memory Comparison ===");
    
    // Original static approach
    u32 static_memory = 1024 * sizeof(s32); // Always allocated
    print_dbg("\r\n Static approach: ");
    print_dbg_ulong(static_memory);
    print_dbg(" bytes always allocated");
    
    // Dynamic approach
    dynamic_flash_buffer_init();
    u32 dynamic_saved = dynamic_flash_buffer_memory_saved();
    print_dbg("\r\n Dynamic approach: 0 bytes baseline, ");
    print_dbg_ulong(dynamic_saved);
    print_dbg(" bytes saved");
    
    // Show efficiency during operation
    s32* buffer = dynamic_flash_buffer_alloc(512); // Smaller operation
    if (buffer) {
        u32 allocated = dynamic_flash_buffer_get_allocated_bytes();
        print_dbg("\r\n Example operation: ");
        print_dbg_ulong(allocated);
        print_dbg(" bytes allocated (");
        print_dbg_ulong(static_memory - allocated);
        print_dbg(" bytes saved)");
        dynamic_flash_buffer_free();
    }
    
    dynamic_flash_buffer_deinit();
    print_dbg("\r\n ======================================");
}

//=====================================
//===== Error Handling Example

typedef enum {
    FLASH_INIT_SUCCESS = 0,
    FLASH_INIT_BUFFER_ALLOC_ERROR = -1,
    FLASH_INIT_FILE_LOAD_ERROR = -2,
    FLASH_INIT_FLASH_WRITE_ERROR = -3
} flash_init_result_t;

flash_init_result_t flash_init_scaler_data_with_error_handling(void) {
    dynamic_flash_buffer_init();
    
    u32 b;
    const char* path;
    u8 p;
    u8* dst;
    flash_init_result_t result = FLASH_INIT_SUCCESS;

    for(p = 0; p < eParamTypesCount && result == FLASH_INIT_SUCCESS; p++) {
        b = scaler_get_data_bytes(p);
        if(b > 0) {
            u32 elements_needed = (b >> 2);
            s32* scalerBuf = dynamic_flash_buffer_alloc(elements_needed);
            
            if (!scalerBuf) {
                print_dbg("\r\n ERROR: Buffer allocation failed for param ");
                print_dbg_ulong(p);
                result = FLASH_INIT_BUFFER_ALLOC_ERROR;
                break;
            }
            
            path = scaler_get_data_path(p);
            if (files_load_scaler_data(path, scalerBuf, elements_needed) != 0) {
                print_dbg("\r\n ERROR: File load failed: ");
                print_dbg(path);
                dynamic_flash_buffer_free();
                result = FLASH_INIT_FILE_LOAD_ERROR;
                break;
            }
            
            dst = (void*)scaler_get_nv_data(p);
            if (flashc_memcpy(dst, (void*)scalerBuf, b, true) != FLASHC_RC_OK) {
                print_dbg("\r\n ERROR: Flash write failed");
                dynamic_flash_buffer_free();
                result = FLASH_INIT_FLASH_WRITE_ERROR;
                break;
            }
            
            dynamic_flash_buffer_free();
        }
    }
    
    dynamic_flash_buffer_deinit();
    
    if (result == FLASH_INIT_SUCCESS) {
        print_dbg("\r\n Flash initialization completed successfully (DYNAMIC)");
    } else {
        print_dbg("\r\n Flash initialization failed with error: ");
        print_dbg_ulong(result);
    }
    
    return result;
}