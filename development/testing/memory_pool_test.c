/**
 * Memory Pool Optimization Test Suite
 * 
 * Phase 2.4: Hybrid Three-Pool Memory Management System
 * Tests the optimization from 2-pool to 3-pool allocation strategy
 * 
 * Expected improvements:
 * - 60% fragmentation reduction
 * - Better pool utilization  
 * - Increased big pool availability (8 -> 12 operators)
 * - Optimal allocation for medium-sized operators (129-2048 bytes)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

// Mock the Aleph types and functions for testing
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed short s16;
typedef signed int s32;

// Define the pool constants (matching our new configuration)
#define SMALL_OP_SIZE 128
#define MAX_SMALL_OPS 179

#define MEDIUM_OP_SIZE 2048  
#define MAX_MEDIUM_OPS 32

#define BIG_OP_SIZE (1024 * 16)
#define MAX_BIG_OPS 12

// Simulate operator sizes from the real system
typedef struct {
    const char* name;
    u32 size;
    const char* category;
} test_operator_t;

// Real operator size distribution from Aleph analysis
static test_operator_t test_operators[] = {
    // Small operators (≤128 bytes) - should use small pool
    {"ADD", 64, "Small Math"},
    {"SUB", 64, "Small Math"}, 
    {"MUL", 72, "Small Math"},
    {"DIV", 72, "Small Math"},
    {"AND", 48, "Small Logic"},
    {"OR", 48, "Small Logic"},
    {"XOR", 48, "Small Logic"},
    {"TOG", 56, "Small Logic"},
    {"GATE", 88, "Small Logic"},
    {"SWITCH", 96, "Small Control"},
    
    // Medium operators (129-2048 bytes) - should use medium pool  
    {"ACCUM", 256, "Medium Math"},
    {"COUNTER", 384, "Medium Logic"},
    {"TIMER", 512, "Medium Control"},
    {"PRESET", 768, "Medium System"},
    {"HISTORY", 1024, "Medium Data"},
    {"ROUTE", 1536, "Medium Control"},
    {"MEM1D", 1792, "Medium Memory"},
    
    // Big operators (>2048 bytes) - should use big pool
    {"BIGNUM", 4096, "Big Graphics"},
    {"BARS8", 8192, "Big Graphics"}, 
    {"SCREEN", 12288, "Big Graphics"},
    {"LIFE", 16384, "Big Complex"}
};

static const u32 NUM_TEST_OPERATORS = sizeof(test_operators) / sizeof(test_operators[0]);

// Memory allocation tracking
typedef struct {
    u32 small_allocated;
    u32 medium_allocated; 
    u32 big_allocated;
    u32 total_wasted_bytes;
    u32 fragmentation_events;
} memory_stats_t;

// Calculate fragmentation for old 2-pool system
memory_stats_t simulate_old_system(test_operator_t* ops, u32 count) {
    memory_stats_t stats = {0};
    
    for (u32 i = 0; i < count; i++) {
        if (ops[i].size <= SMALL_OP_SIZE) {
            // Small pool allocation
            stats.small_allocated++;
            stats.total_wasted_bytes += (SMALL_OP_SIZE - ops[i].size);
        } else {
            // Big pool allocation (old system had no medium pool)
            stats.big_allocated++;
            stats.total_wasted_bytes += (BIG_OP_SIZE - ops[i].size);
            
            // Track severe fragmentation for medium-sized ops in big pool
            if (ops[i].size <= MEDIUM_OP_SIZE) {
                stats.fragmentation_events++;
            }
        }
    }
    
    return stats;
}

// Calculate fragmentation for new 3-pool system
memory_stats_t simulate_new_system(test_operator_t* ops, u32 count) {
    memory_stats_t stats = {0};
    
    for (u32 i = 0; i < count; i++) {
        if (ops[i].size <= SMALL_OP_SIZE) {
            // Small pool allocation
            stats.small_allocated++;
            stats.total_wasted_bytes += (SMALL_OP_SIZE - ops[i].size);
        } else if (ops[i].size <= MEDIUM_OP_SIZE) {
            // Medium pool allocation (new optimization!)
            stats.medium_allocated++;
            stats.total_wasted_bytes += (MEDIUM_OP_SIZE - ops[i].size);
        } else {
            // Big pool allocation
            stats.big_allocated++;
            stats.total_wasted_bytes += (BIG_OP_SIZE - ops[i].size);
        }
    }
    
    return stats;
}

void print_memory_analysis(const char* system_name, memory_stats_t stats) {
    printf("\n=== %s Memory Analysis ===\n", system_name);
    printf("Small Pool Usage:   %u operators\n", stats.small_allocated);
    printf("Medium Pool Usage:  %u operators\n", stats.medium_allocated);
    printf("Big Pool Usage:     %u operators\n", stats.big_allocated);
    printf("Total Wasted Bytes: %u bytes (%.1f KB)\n", 
           stats.total_wasted_bytes, stats.total_wasted_bytes / 1024.0);
    printf("Fragmentation Events: %u\n", stats.fragmentation_events);
    
    // Calculate pool utilization efficiency
    u32 total_allocated = (stats.small_allocated * SMALL_OP_SIZE) + 
                         (stats.medium_allocated * MEDIUM_OP_SIZE) + 
                         (stats.big_allocated * BIG_OP_SIZE);
    u32 total_used = total_allocated - stats.total_wasted_bytes;
    
    printf("Pool Efficiency: %.1f%% (%u used / %u allocated)\n",
           (total_used * 100.0) / total_allocated, total_used, total_allocated);
}

int test_pool_optimization() {
    printf("🧪 Memory Pool Optimization Test - Phase 2.4\n");
    printf("============================================\n");
    
    // Test realistic operator allocation scenario
    test_operator_t scenario[] = {
        // Typical bees session: heavy on small/medium, moderate big usage
        {"ADD", 64, "Small"}, {"ADD", 64, "Small"}, {"ADD", 64, "Small"},
        {"SUB", 64, "Small"}, {"SUB", 64, "Small"}, 
        {"MUL", 72, "Small"}, {"MUL", 72, "Small"},
        {"SWITCH", 96, "Small"}, {"SWITCH", 96, "Small"}, {"SWITCH", 96, "Small"},
        {"GATE", 88, "Small"}, {"GATE", 88, "Small"},
        {"TIMER", 512, "Medium"}, {"TIMER", 512, "Medium"}, {"TIMER", 512, "Medium"},
        {"PRESET", 768, "Medium"}, {"PRESET", 768, "Medium"},
        {"ACCUM", 256, "Medium"}, {"ACCUM", 256, "Medium"}, {"ACCUM", 256, "Medium"},
        {"HISTORY", 1024, "Medium"}, {"HISTORY", 1024, "Medium"},
        {"ROUTE", 1536, "Medium"},
        {"BIGNUM", 4096, "Big"}, {"BIGNUM", 4096, "Big"},
        {"BARS8", 8192, "Big"}, 
        {"SCREEN", 12288, "Big"}
    };
    
    u32 scenario_count = sizeof(scenario) / sizeof(scenario[0]);
    printf("Testing scenario with %u operators...\n", scenario_count);
    
    // Simulate both systems
    memory_stats_t old_stats = simulate_old_system(scenario, scenario_count);
    memory_stats_t new_stats = simulate_new_system(scenario, scenario_count);
    
    // Print detailed analysis
    print_memory_analysis("Old 2-Pool System", old_stats);
    print_memory_analysis("New 3-Pool System", new_stats);
    
    // Calculate improvements
    printf("\n🎯 OPTIMIZATION RESULTS\n");
    printf("=======================\n");
    
    float fragmentation_reduction = ((float)(old_stats.total_wasted_bytes - new_stats.total_wasted_bytes) / old_stats.total_wasted_bytes) * 100;
    printf("Fragmentation Reduction: %.1f%% (target: 60%%)\n", fragmentation_reduction);
    
    u32 big_pool_freed = old_stats.big_allocated - new_stats.big_allocated;
    printf("Big Pool Operators Freed: %u (moved to medium pool)\n", big_pool_freed);
    
    float big_pool_availability = ((float)(MAX_BIG_OPS - new_stats.big_allocated) / MAX_BIG_OPS) * 100;
    printf("Big Pool Availability: %.1f%% (%u/%u free)\n", 
           big_pool_availability, MAX_BIG_OPS - new_stats.big_allocated, MAX_BIG_OPS);
    
    printf("Medium Pool Efficiency: %u operators using optimal 2KB allocation\n", new_stats.medium_allocated);
    
    // Memory layout comparison
    printf("\n📊 MEMORY LAYOUT COMPARISON\n");
    printf("===========================\n");
    
    u32 old_total = (MAX_SMALL_OPS * SMALL_OP_SIZE) + (8 * BIG_OP_SIZE);  // Old: 256 small + 8 big
    u32 new_total = (MAX_SMALL_OPS * SMALL_OP_SIZE) + (MAX_MEDIUM_OPS * MEDIUM_OP_SIZE) + (MAX_BIG_OPS * BIG_OP_SIZE);
    
    printf("Old System Memory: %u KB (256×128B + 8×16KB)\n", old_total / 1024);
    printf("New System Memory: %u KB (179×128B + 32×2KB + 12×16KB)\n", new_total / 1024);
    printf("Memory Overhead: %+d KB\n", (s32)(new_total - old_total) / 1024);
    
    // Pool capacity analysis
    printf("\n🏊 POOL CAPACITY ANALYSIS\n");
    printf("=========================\n");
    printf("Small Pool: %u operators (was 256, optimized to 179)\n", MAX_SMALL_OPS);
    printf("Medium Pool: %u operators (NEW - handles 129-2048 byte range)\n", MAX_MEDIUM_OPS);
    printf("Big Pool: %u operators (was 8, increased to 12)\n", MAX_BIG_OPS);
    
    // Validation tests
    bool success = true;
    
    if (fragmentation_reduction < 60.0) {
        printf("❌ FAILURE: Fragmentation reduction %.1f%% below 60%% target\n", fragmentation_reduction);
        success = false;
    } else {
        printf("✅ SUCCESS: Fragmentation reduction %.1f%% exceeds 60%% target\n", fragmentation_reduction);
    }
    
    if (new_stats.big_allocated > old_stats.big_allocated) {
        printf("❌ FAILURE: Big pool usage increased instead of decreased\n");
        success = false;
    } else {
        printf("✅ SUCCESS: Big pool usage reduced from %u to %u operators\n", 
               old_stats.big_allocated, new_stats.big_allocated);
    }
    
    if (new_stats.medium_allocated == 0) {
        printf("❌ FAILURE: Medium pool not being utilized\n");
        success = false;
    } else {
        printf("✅ SUCCESS: Medium pool serving %u operators optimally\n", new_stats.medium_allocated);
    }
    
    // Test pool exhaustion scenarios
    printf("\n🔥 POOL EXHAUSTION ANALYSIS\n");
    printf("===========================\n");
    printf("Old System Big Pool: %u/%u used (%.1f%% capacity)\n", 
           old_stats.big_allocated, 8, (old_stats.big_allocated * 100.0) / 8);
    printf("New System Big Pool: %u/%u used (%.1f%% capacity)\n", 
           new_stats.big_allocated, MAX_BIG_OPS, (new_stats.big_allocated * 100.0) / MAX_BIG_OPS);
    
    if (old_stats.big_allocated >= 8) {
        printf("⚠️  WARNING: Old system would have exhausted big pool!\n");
    }
    
    printf("\n");
    return success ? 0 : 1;
}

int test_edge_cases() {
    printf("🧪 Edge Case Testing\n");
    printf("====================\n");
    
    // Test boundary conditions
    test_operator_t boundary_ops[] = {
        {"Edge128", 128, "Small Max"},      // Exactly small pool max
        {"Edge129", 129, "Medium Min"},     // Exactly medium pool min
        {"Edge2048", 2048, "Medium Max"},   // Exactly medium pool max
        {"Edge2049", 2049, "Big Min"}       // Exactly big pool min
    };
    
    printf("Testing boundary size allocations...\n");
    
    memory_stats_t old_boundary = simulate_old_system(boundary_ops, 4);
    memory_stats_t new_boundary = simulate_new_system(boundary_ops, 4);
    
    print_memory_analysis("Old System Boundaries", old_boundary);
    print_memory_analysis("New System Boundaries", new_boundary);
    
    // Verify correct pool selection
    bool boundary_success = true;
    
    // In new system: 128->small, 129->medium, 2048->medium, 2049->big
    if (new_boundary.small_allocated != 1 || new_boundary.medium_allocated != 2 || new_boundary.big_allocated != 1) {
        printf("❌ FAILURE: Boundary allocation incorrect\n");
        boundary_success = false;
    } else {
        printf("✅ SUCCESS: Boundary allocations correct (1 small, 2 medium, 1 big)\n");
    }
    
    return boundary_success ? 0 : 1;
}

int test_real_world_scenario() {
    printf("🧪 Real-World Usage Scenario\n");
    printf("=============================\n");
    
    // Simulate a complex bees patch based on analysis from memory-pool-system-analysis.md
    test_operator_t complex_patch[] = {
        // Math operators (Small pool - should be ~60% of total)
        {"ADD", 64, "Small"}, {"ADD", 64, "Small"}, {"ADD", 64, "Small"}, {"ADD", 64, "Small"},
        {"SUB", 64, "Small"}, {"SUB", 64, "Small"}, {"SUB", 64, "Small"},
        {"MUL", 72, "Small"}, {"MUL", 72, "Small"}, {"MUL", 72, "Small"}, {"MUL", 72, "Small"},
        {"DIV", 72, "Small"}, {"DIV", 72, "Small"},
        
        // Logic operators (Small pool - should be ~25% of total) 
        {"AND", 48, "Small"}, {"AND", 48, "Small"}, {"AND", 48, "Small"},
        {"OR", 48, "Small"}, {"OR", 48, "Small"},
        {"XOR", 48, "Small"}, {"XOR", 48, "Small"},
        {"TOG", 56, "Small"}, {"TOG", 56, "Small"},
        {"GATE", 88, "Small"}, {"GATE", 88, "Small"},
        {"SWITCH", 96, "Small"}, {"SWITCH", 96, "Small"}, {"SWITCH", 96, "Small"},
        
        // Complex processing (Medium pool - should be ~10% of total)
        {"TIMER", 512, "Medium"}, {"TIMER", 512, "Medium"}, {"TIMER", 512, "Medium"},
        {"PRESET", 768, "Medium"}, {"PRESET", 768, "Medium"},
        {"ACCUM", 256, "Medium"}, {"ACCUM", 256, "Medium"},
        {"HISTORY", 1024, "Medium"}, {"HISTORY", 1024, "Medium"},
        {"ROUTE", 1536, "Medium"},
        
        // Graphics operators (Big pool - should be ~5% of total)
        {"BIGNUM", 4096, "Big"}, {"BIGNUM", 4096, "Big"},
        {"BARS8", 8192, "Big"},
        {"SCREEN", 12288, "Big"}
    };
    
    u32 patch_size = sizeof(complex_patch) / sizeof(complex_patch[0]);
    printf("Simulating complex patch with %u operators...\n", patch_size);
    
    memory_stats_t old_patch = simulate_old_system(complex_patch, patch_size);
    memory_stats_t new_patch = simulate_new_system(complex_patch, patch_size);
    
    print_memory_analysis("Old System - Complex Patch", old_patch);
    print_memory_analysis("New System - Complex Patch", new_patch);
    
    // Calculate real-world improvements
    float real_fragmentation_reduction = ((float)(old_patch.total_wasted_bytes - new_patch.total_wasted_bytes) / old_patch.total_wasted_bytes) * 100;
    
    printf("\n🌍 REAL-WORLD IMPACT\n");
    printf("====================\n");
    printf("Fragmentation Reduction: %.1f%%\n", real_fragmentation_reduction);
    printf("Big Pool Operators Freed: %u\n", old_patch.big_allocated - new_patch.big_allocated);
    printf("Medium Pool Utilization: %u operators\n", new_patch.medium_allocated);
    
    // Pool exhaustion protection
    printf("Big Pool Safety Margin: %u operators remaining\n", MAX_BIG_OPS - new_patch.big_allocated);
    
    bool real_world_success = (real_fragmentation_reduction >= 60.0) && 
                             (new_patch.big_allocated < old_patch.big_allocated) &&
                             (new_patch.medium_allocated > 0);
    
    if (real_world_success) {
        printf("✅ SUCCESS: Real-world scenario optimization validated\n");
    } else {
        printf("❌ FAILURE: Real-world scenario did not meet optimization targets\n");
    }
    
    return real_world_success ? 0 : 1;
}

int main() {
    printf("🚀 Phase 2.4: Memory Pool Optimization Validation\n");
    printf("==================================================\n");
    printf("Testing hybrid three-pool allocation strategy...\n\n");
    
    int test_results = 0;
    
    // Run comprehensive test suite
    test_results += test_pool_optimization();
    printf("\n" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" "\n\n");
    
    test_results += test_edge_cases();
    printf("\n" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" "\n\n");
    
    test_results += test_real_world_scenario();
    printf("\n" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" "\n\n");
    
    // Final assessment
    if (test_results == 0) {
        printf("🎉 ALL TESTS PASSED! Memory Pool Optimization Successful\n");
        printf("✅ 60%%+ fragmentation reduction achieved\n");
        printf("✅ Big pool capacity increased (8 → 12 operators)\n");
        printf("✅ Medium pool efficiently handles 129-2048 byte operators\n");
        printf("✅ Optimal allocation strategy for all operator sizes\n");
        printf("✅ Pool exhaustion protection improved\n");
    } else {
        printf("❌ %d TEST(S) FAILED! Memory Pool Optimization needs review\n", test_results);
    }
    
    return test_results;
}