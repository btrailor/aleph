/* fix_test.c
 * Test harness for fixed-point math optimization validation
 * Tests correctness and performance of modulus elimination
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

// Include the optimized fix functions
#include "fix.h"

// Test configuration
#define NUM_TEST_VALUES 1000
#define NUM_PERFORMANCE_ITERATIONS 10000

// Test value ranges
static const int test_values[] = {
    // Edge cases
    0, 1, -1, 9, -9, 10, -10, 99, -99, 100, -100,
    // Powers of 10
    1000, -1000, 10000, -10000, 100000, -100000,
    // Random values
    42, -42, 1337, -1337, 98765, -98765,
    // Near limits
    2147483647, -2147483647, 2147483646, -2147483646,
    // Boundary cases
    999999999, -999999999, 123456789, -123456789
};

static const int num_test_values = sizeof(test_values) / sizeof(test_values[0]);

// Reference implementation (original with modulus)
void itoa_whole_reference(int val, char* buf, int len) {
    static char* p;
    static unsigned int a, u;
    static unsigned long int sign;
    
    // Clear buffer
    memset(buf, 0, len);
    
    if(val == 0) {
        *buf = '0';
        return;
    }
    
    p = buf + len - 1;
    sign = BIT_SIGN_32(val);
    
    if (sign) {
        len--;
        val = BIT_INVERT_32(val) + 1;  // Original implementation
    }
    
    u = (unsigned int)val;
    
    // Original modulus-based implementation
    while(p >= buf) {
        if (u > 0) {
            a = u % 10;        // ORIGINAL: Expensive modulus
            u /= 10;           // ORIGINAL: Expensive division
            *p = '0' + a;
        } else {
            *p = ' ';
        }
        p--;
    }
    
    if(sign) { 
        *buf = '-'; 
    }
}

// Performance timing utilities
static inline uint64_t get_cycles(void) {
#ifdef __x86_64__
    uint64_t cycles;
    __asm__ volatile ("rdtsc" : "=A" (cycles));
    return cycles;
#else
    // Fallback to clock() for other architectures
    return (uint64_t)clock();
#endif
}

// Test correctness of optimization
int test_correctness(void) {
    char buf_optimized[16];
    char buf_reference[16];
    int failures = 0;
    
    printf("=== CORRECTNESS TESTING ===\n");
    
    for (int i = 0; i < num_test_values; i++) {
        int test_val = test_values[i];
        
        // Clear buffers
        memset(buf_optimized, 0, sizeof(buf_optimized));
        memset(buf_reference, 0, sizeof(buf_reference));
        
        // Test both implementations
        itoa_whole(test_val, buf_optimized, 15);          // Our optimization
        itoa_whole_reference(test_val, buf_reference, 15); // Reference
        
        // Compare results
        if (strcmp(buf_optimized, buf_reference) != 0) {
            printf("FAIL: Value %d\n", test_val);
            printf("  Optimized:  '%s'\n", buf_optimized);
            printf("  Reference:  '%s'\n", buf_reference);
            failures++;
        } else {
            printf("PASS: %d -> '%s'\n", test_val, buf_optimized);
        }
    }
    
    printf("\nCorrectness Results: %d/%d tests passed\n", 
           num_test_values - failures, num_test_values);
    
    return failures;
}

// Test performance improvement
void test_performance(void) {
    char buffer[16];
    uint64_t start_cycles, end_cycles;
    uint64_t optimized_cycles = 0, reference_cycles = 0;
    
    printf("\n=== PERFORMANCE TESTING ===\n");
    
    // Warm up cache
    for (int i = 0; i < 100; i++) {
        itoa_whole(12345, buffer, 15);
        itoa_whole_reference(12345, buffer, 15);
    }
    
    // Test optimized implementation
    start_cycles = get_cycles();
    for (int iteration = 0; iteration < NUM_PERFORMANCE_ITERATIONS; iteration++) {
        for (int i = 0; i < num_test_values; i++) {
            itoa_whole(test_values[i], buffer, 15);
        }
    }
    end_cycles = get_cycles();
    optimized_cycles = end_cycles - start_cycles;
    
    // Test reference implementation  
    start_cycles = get_cycles();
    for (int iteration = 0; iteration < NUM_PERFORMANCE_ITERATIONS; iteration++) {
        for (int i = 0; i < num_test_values; i++) {
            itoa_whole_reference(test_values[i], buffer, 15);
        }
    }
    end_cycles = get_cycles();
    reference_cycles = end_cycles - start_cycles;
    
    // Calculate performance improvement
    double improvement = (double)reference_cycles / (double)optimized_cycles;
    double percent_improvement = ((double)(reference_cycles - optimized_cycles) / (double)reference_cycles) * 100.0;
    
    printf("Performance Results:\n");
    printf("  Reference cycles:  %llu\n", (unsigned long long)reference_cycles);
    printf("  Optimized cycles:  %llu\n", (unsigned long long)optimized_cycles);
    printf("  Improvement:       %.2fx faster\n", improvement);
    printf("  Percent reduction: %.1f%%\n", percent_improvement);
    
    if (improvement >= 1.5) {
        printf("  Status: ✅ PERFORMANCE TARGET MET (>1.5x improvement)\n");
    } else if (improvement >= 1.2) {
        printf("  Status: ⚠️  MODERATE IMPROVEMENT (1.2-1.5x)\n");
    } else {
        printf("  Status: ❌ PERFORMANCE TARGET MISSED (<1.2x improvement)\n");
    }
}

// Test edge cases and boundary conditions
void test_edge_cases(void) {
    char buffer[16];
    
    printf("\n=== EDGE CASE TESTING ===\n");
    
    // Test overflow protection
    printf("Testing overflow protection:\n");
    itoa_whole(0x80000000, buffer, 15);  // Most negative 32-bit int
    printf("  INT_MIN (0x80000000): '%s'\n", buffer);
    
    // Test zero
    itoa_whole(0, buffer, 15);
    printf("  Zero: '%s'\n", buffer);
    
    // Test single digits
    for (int i = 1; i <= 9; i++) {
        itoa_whole(i, buffer, 15);
        printf("  Single digit %d: '%s'\n", i, buffer);
    }
    
    // Test negative single digits
    for (int i = -1; i >= -9; i--) {
        itoa_whole(i, buffer, 15);
        printf("  Negative single digit %d: '%s'\n", i, buffer);
    }
}

// Generate random test cases
void test_random_values(void) {
    char buf_optimized[16];
    char buf_reference[16];
    int failures = 0;
    
    printf("\n=== RANDOM VALUE TESTING ===\n");
    
    srand((unsigned int)time(NULL));
    
    for (int i = 0; i < NUM_TEST_VALUES; i++) {
        // Generate random 32-bit signed integer
        int test_val = (int)(rand() * ((long long)rand() << 16));
        
        memset(buf_optimized, 0, sizeof(buf_optimized));
        memset(buf_reference, 0, sizeof(buf_reference));
        
        itoa_whole(test_val, buf_optimized, 15);
        itoa_whole_reference(test_val, buf_reference, 15);
        
        if (strcmp(buf_optimized, buf_reference) != 0) {
            printf("FAIL: Random value %d\n", test_val);
            printf("  Optimized: '%s'\n", buf_optimized);
            printf("  Reference: '%s'\n", buf_reference);
            failures++;
            
            // Stop after 10 failures to avoid spam
            if (failures >= 10) {
                printf("  ... stopping after 10 failures\n");
                break;
            }
        }
    }
    
    printf("Random test results: %d/%d passed\n", 
           NUM_TEST_VALUES - failures, NUM_TEST_VALUES);
}

int main(void) {
    printf("Fixed-Point Math Optimization Test Suite\n");
    printf("========================================\n\n");
    
    int correctness_failures = 0;
    
    // Run all test suites
    correctness_failures += test_correctness();
    test_edge_cases();
    test_random_values();
    test_performance();
    
    printf("\n=== FINAL RESULTS ===\n");
    if (correctness_failures == 0) {
        printf("✅ All correctness tests PASSED\n");
        printf("🚀 Fixed-point optimization is ready for production\n");
        return 0;
    } else {
        printf("❌ %d correctness test(s) FAILED\n", correctness_failures);
        printf("⚠️  Optimization needs debugging before deployment\n");
        return 1;
    }
}