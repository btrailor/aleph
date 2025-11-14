/* malloc_optimization_test.c
 * Test Suite for Dynamic Memory Management Optimizations
 * 
 * Validates both network and flash buffer dynamic allocation systems
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "dynamic_network.h"
#include "dynamic_flash_buffer.h"

//=====================================
//===== Test Framework

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
    do { \
        printf("\n=== TEST: %s ===\n", name); \
        tests_run++; \
    } while(0)

#define ASSERT(condition, message) \
    do { \
        if (condition) { \
            printf("✅ PASS: %s\n", message); \
        } else { \
            printf("❌ FAIL: %s\n", message); \
            return; \
        } \
    } while(0)

#define TEST_COMPLETE() \
    do { \
        tests_passed++; \
        printf("✅ Test completed successfully\n"); \
    } while(0)

//=====================================
//===== Dynamic Network Tests

void test_dynamic_network_initialization(void) {
    TEST("Dynamic Network Initialization");
    
    // Test initialization
    dynamic_net_init();
    
    u16 ops_used, ops_max, ins_used, ins_max, outs_used, outs_max;
    dynamic_net_get_stats(&ops_used, &ops_max, &ins_used, &ins_max, &outs_used, &outs_max);
    
    ASSERT(ops_used == 0, "Initial ops count is zero");
    ASSERT(ops_max == INITIAL_OPS_SIZE, "Ops capacity matches initial size");
    ASSERT(ins_max == INITIAL_INS_SIZE, "Ins capacity matches initial size");
    ASSERT(outs_max == INITIAL_OUTS_SIZE, "Outs capacity matches initial size");
    
    u32 memory_used = dynamic_net_memory_usage();
    u32 memory_saved = dynamic_net_memory_saved();
    
    ASSERT(memory_used > 0, "Memory usage is tracked");
    ASSERT(memory_saved > 0, "Memory savings vs fixed arrays");
    
    printf("Memory used: %u bytes, saved: %u bytes\n", memory_used, memory_saved);
    
    dynamic_net_deinit();
    
    TEST_COMPLETE();
}

void test_dynamic_network_expansion(void) {
    TEST("Dynamic Network Expansion");
    
    dynamic_net_init();
    
    // Test ops expansion
    int result = dynamic_net_expand_ops(INITIAL_OPS_SIZE + 10);
    ASSERT(result == 0, "Ops expansion successful");
    
    u16 ops_used, ops_max, ins_used, ins_max, outs_used, outs_max;
    dynamic_net_get_stats(&ops_used, &ops_max, &ins_used, &ins_max, &outs_used, &outs_max);
    
    ASSERT(ops_max >= INITIAL_OPS_SIZE + 10, "Ops capacity expanded");
    
    // Test limit enforcement
    result = dynamic_net_expand_ops(MAX_OPS_LIMIT + 100);
    ASSERT(result == -1, "Ops limit enforcement works");
    
    // Test ins/outs expansion
    result = dynamic_net_expand_ins(INITIAL_INS_SIZE + 20);
    ASSERT(result == 0, "Ins expansion successful");
    
    result = dynamic_net_expand_outs(INITIAL_OUTS_SIZE + 30);
    ASSERT(result == 0, "Outs expansion successful");
    
    dynamic_net_get_stats(&ops_used, &ops_max, &ins_used, &ins_max, &outs_used, &outs_max);
    
    ASSERT(ins_max >= INITIAL_INS_SIZE + 20, "Ins capacity expanded");
    ASSERT(outs_max >= INITIAL_OUTS_SIZE + 30, "Outs capacity expanded");
    
    printf("Final capacities: ops=%u, ins=%u, outs=%u\n", ops_max, ins_max, outs_max);
    
    dynamic_net_deinit();
    
    TEST_COMPLETE();
}

void test_dynamic_network_limits(void) {
    TEST("Dynamic Network Limits");
    
    dynamic_net_init();
    
    // Test capacity checking
    int can_add = dynamic_net_can_add_op(10, 10);
    ASSERT(can_add == 1, "Can add operator within limits");
    
    can_add = dynamic_net_can_add_op(MAX_INS_LIMIT + 1, 10);
    ASSERT(can_add == 0, "Cannot exceed ins limit");
    
    can_add = dynamic_net_can_add_op(10, MAX_OUTS_LIMIT + 1);
    ASSERT(can_add == 0, "Cannot exceed outs limit");
    
    dynamic_net_deinit();
    
    TEST_COMPLETE();
}

//=====================================
//===== Dynamic Flash Buffer Tests

void test_dynamic_flash_buffer_initialization(void) {
    TEST("Dynamic Flash Buffer Initialization");
    
    dynamic_flash_buffer_init();
    
    ASSERT(!dynamic_flash_buffer_is_allocated(), "Initially no buffer allocated");
    ASSERT(dynamic_flash_buffer_get() == NULL, "Get returns NULL initially");
    ASSERT(dynamic_flash_buffer_get_size() == 0, "Size is zero initially");
    
    u32 memory_saved = dynamic_flash_buffer_memory_saved();
    ASSERT(memory_saved > 0, "Memory savings vs static buffer");
    
    printf("Memory saved vs static: %u bytes\n", memory_saved);
    
    dynamic_flash_buffer_deinit();
    
    TEST_COMPLETE();
}

void test_dynamic_flash_buffer_allocation(void) {
    TEST("Dynamic Flash Buffer Allocation");
    
    dynamic_flash_buffer_init();
    
    // Test default allocation
    s32* buffer = dynamic_flash_buffer_alloc(0); // Should use default size
    ASSERT(buffer != NULL, "Default allocation successful");
    ASSERT(dynamic_flash_buffer_is_allocated(), "Buffer marked as allocated");
    ASSERT(dynamic_flash_buffer_get_size() == FLASH_BUFFER_DEFAULT_SIZE, "Default size used");
    
    // Test buffer usage
    buffer[0] = 0x12345678;
    buffer[100] = 0xABCDEF00;
    ASSERT(buffer[0] == 0x12345678, "Buffer write/read works");
    ASSERT(buffer[100] == 0xABCDEF00, "Buffer array access works");
    
    dynamic_flash_buffer_free();
    ASSERT(!dynamic_flash_buffer_is_allocated(), "Buffer marked as freed");
    ASSERT(dynamic_flash_buffer_get() == NULL, "Get returns NULL after free");
    
    // Test custom size allocation
    buffer = dynamic_flash_buffer_alloc(2048);
    ASSERT(buffer != NULL, "Custom size allocation successful");
    ASSERT(dynamic_flash_buffer_get_size() == 2048, "Custom size used");
    
    dynamic_flash_buffer_free();
    
    // Test oversized allocation
    buffer = dynamic_flash_buffer_alloc(FLASH_BUFFER_MAX_SIZE + 1);
    ASSERT(buffer == NULL, "Oversized allocation rejected");
    
    dynamic_flash_buffer_deinit();
    
    TEST_COMPLETE();
}

void test_dynamic_flash_buffer_convenience_macro(void) {
    TEST("Dynamic Flash Buffer Convenience Macro");
    
    dynamic_flash_buffer_init();
    
    int test_executed = 0;
    
    WITH_FLASH_BUFFER(512, {
        s32* scalerBuf = dynamic_flash_buffer_get();
        ASSERT(scalerBuf != NULL, "Macro provides buffer");
        ASSERT(dynamic_flash_buffer_get_size() == 512, "Macro uses correct size");
        
        // Simulate flash operation
        for (int i = 0; i < 100; i++) {
            scalerBuf[i] = i * 2;
        }
        
        ASSERT(scalerBuf[50] == 100, "Buffer operations work in macro");
        test_executed = 1;
    });
    
    ASSERT(test_executed == 1, "Macro code block executed");
    ASSERT(!dynamic_flash_buffer_is_allocated(), "Buffer auto-freed after macro");
    
    dynamic_flash_buffer_deinit();
    
    TEST_COMPLETE();
}

//=====================================
//===== Performance Tests

void test_memory_efficiency(void) {
    TEST("Memory Efficiency Comparison");
    
    // Test network efficiency
    dynamic_net_init();
    u32 net_saved = dynamic_net_memory_saved();
    u32 net_used = dynamic_net_memory_usage();
    dynamic_net_deinit();
    
    // Test flash buffer efficiency
    dynamic_flash_buffer_init();
    u32 flash_saved = dynamic_flash_buffer_memory_saved();
    dynamic_flash_buffer_deinit();
    
    u32 total_saved = net_saved + flash_saved;
    
    printf("=== Memory Efficiency Results ===\n");
    printf("Network memory saved: %u bytes\n", net_saved);
    printf("Network initial usage: %u bytes\n", net_used);
    printf("Flash buffer saved: %u bytes\n", flash_saved);
    printf("Total memory saved: %u bytes\n", total_saved);
    printf("Efficiency gain: %.1f%%\n", 
           total_saved > 0 ? (float)total_saved / (total_saved + net_used) * 100.0f : 0.0f);
    
    ASSERT(total_saved > 0, "Combined memory savings achieved");
    ASSERT(net_used < net_saved, "Dynamic allocation more efficient than fixed");
    
    TEST_COMPLETE();
}

//=====================================
//===== Integration Tests

void test_integration_flash_operations(void) {
    TEST("Integration: Flash Operations");
    
    dynamic_flash_buffer_init();
    
    // Simulate typical flash_bees.c operations
    WITH_FLASH_BUFFER(1024, {
        s32* scalerBuf = dynamic_flash_buffer_get();
        
        // Simulate file loading
        for (u32 i = 0; i < 100; i++) {
            scalerBuf[i] = (s32)(i * 0x10000 + 0x1234);
        }
        
        // Simulate flash writing preparation
        u32 checksum = 0;
        for (u32 i = 0; i < 100; i++) {
            checksum += (u32)scalerBuf[i];
        }
        
        ASSERT(checksum > 0, "Flash operation simulation completed");
        
        // Buffer automatically freed at end of block
    });
    
    ASSERT(!dynamic_flash_buffer_is_allocated(), "Buffer cleaned up after operation");
    
    dynamic_flash_buffer_deinit();
    
    TEST_COMPLETE();
}

//=====================================
//===== Main Test Runner

int main(void) {
    printf("=== Malloc Optimization Test Suite ===\n");
    printf("Testing dynamic network and flash buffer management\n");
    
    // Dynamic Network Tests
    test_dynamic_network_initialization();
    test_dynamic_network_expansion();
    test_dynamic_network_limits();
    
    // Dynamic Flash Buffer Tests
    test_dynamic_flash_buffer_initialization();
    test_dynamic_flash_buffer_allocation();
    test_dynamic_flash_buffer_convenience_macro();
    
    // Performance Tests
    test_memory_efficiency();
    
    // Integration Tests
    test_integration_flash_operations();
    
    printf("\n=== Test Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Success rate: %.1f%%\n", 
           tests_run > 0 ? (float)tests_passed / tests_run * 100.0f : 0.0f);
    
    if (tests_passed == tests_run) {
        printf("🎉 All tests passed!\n");
        return 0;
    } else {
        printf("❌ Some tests failed\n");
        return 1;
    }
}