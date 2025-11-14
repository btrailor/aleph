#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

// Reference implementation (with modulus - slower)
void itoa_whole_reference(int32_t val, char* dst, uint8_t places) {
    int32_t u, a;
    uint8_t pos = 0;
    
    if (val < 0) {
        dst[pos++] = '-';
        u = -val;
    } else {
        u = val;
    }
    
    if (u == 0) {
        dst[pos++] = '0';
        dst[pos] = '\0';
        return;
    }
    
    // Count digits first
    int32_t temp = u;
    uint8_t digit_count = 0;
    while (temp > 0) {
        temp /= 10;
        digit_count++;
    }
    
    // Generate digits in reverse order
    char digits[12];  // Max 10 digits + sign + null
    uint8_t digit_pos = 0;
    
    while (u > 0) {
        a = u % 10;      // EXPENSIVE: Modulus operation
        u /= 10;         // EXPENSIVE: Division operation
        digits[digit_pos++] = '0' + a;
    }
    
    // Reverse and copy
    for (int i = digit_pos - 1; i >= 0; i--) {
        dst[pos++] = digits[i];
    }
    dst[pos] = '\0';
}

// Optimized implementation (without modulus - faster)
void itoa_whole_optimized(int32_t val, char* dst, uint8_t places) {
    int32_t u, a, temp;
    uint8_t pos = 0;
    
    if (val < 0) {
        dst[pos++] = '-';
        u = -val;
    } else {
        u = val;
    }
    
    if (u == 0) {
        dst[pos++] = '0';
        dst[pos] = '\0';
        return;
    }
    
    // Count digits first
    int32_t count_temp = u;
    uint8_t digit_count = 0;
    while (count_temp > 0) {
        count_temp /= 10;
        digit_count++;
    }
    
    // Generate digits in reverse order
    char digits[12];  // Max 10 digits + sign + null
    uint8_t digit_pos = 0;
    
    while (u > 0) {
        temp = u / 10;           // Single division
        a = u - (temp * 10);     // Equivalent to u % 10, but faster
        u = temp;                // No second division needed
        digits[digit_pos++] = '0' + a;
    }
    
    // Reverse and copy
    for (int i = digit_pos - 1; i >= 0; i--) {
        dst[pos++] = digits[i];
    }
    dst[pos] = '\0';
}

int test_correctness() {
    printf("=== CORRECTNESS TESTING ===\n");
    
    int32_t test_values[] = {
        0, 1, -1, 9, -9, 10, -10, 42, -42, 99, -99, 100, -100,
        1000, -1000, 10000, -10000, 12345, -12345, 98765, -98765,
        999999, -999999, 1000000, -1000000, 2147483647, -2147483647
    };
    
    int num_tests = sizeof(test_values) / sizeof(test_values[0]);
    int failures = 0;
    
    char buf_ref[16], buf_opt[16];
    
    for (int i = 0; i < num_tests; i++) {
        int32_t val = test_values[i];
        
        itoa_whole_reference(val, buf_ref, 15);
        itoa_whole_optimized(val, buf_opt, 15);
        
        if (strcmp(buf_ref, buf_opt) != 0) {
            printf("FAIL: %d\n", val);
            printf("  Reference: '%s'\n", buf_ref);
            printf("  Optimized: '%s'\n", buf_opt);
            failures++;
        } else {
            printf("PASS: %d -> '%s'\n", val, buf_opt);
        }
    }
    
    printf("\nCorrectness Results: %d/%d tests passed\n", num_tests - failures, num_tests);
    return failures == 0;
}

int test_performance() {
    printf("\n=== PERFORMANCE TESTING ===\n");
    
    // Test values that will stress the conversion
    int32_t test_values[] = {
        42, 1337, 98765, -12345, 999999, -999999, 
        123456789, -123456789, 2147483647, -2147483647
    };
    int num_values = sizeof(test_values) / sizeof(test_values[0]);
    int iterations = 100000;  // Enough iterations to measure
    
    char buffer[16];
    clock_t start, end;
    double ref_time, opt_time;
    
    printf("Testing with %d iterations of %d values each...\n", iterations, num_values);
    
    // Test reference implementation
    start = clock();
    for (int iter = 0; iter < iterations; iter++) {
        for (int i = 0; i < num_values; i++) {
            itoa_whole_reference(test_values[i], buffer, 15);
        }
    }
    end = clock();
    ref_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    // Test optimized implementation  
    start = clock();
    for (int iter = 0; iter < iterations; iter++) {
        for (int i = 0; i < num_values; i++) {
            itoa_whole_optimized(test_values[i], buffer, 15);
        }
    }
    end = clock();
    opt_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    // Calculate improvement
    double improvement = ref_time / opt_time;
    double percent_improvement = ((ref_time - opt_time) / ref_time) * 100;
    
    printf("Performance Results:\n");
    printf("  Reference time:    %.4f seconds\n", ref_time);
    printf("  Optimized time:    %.4f seconds\n", opt_time);
    printf("  Improvement:       %.2fx faster\n", improvement);
    printf("  Percent reduction: %.1f%%\n", percent_improvement);
    
    if (improvement >= 1.5) {
        printf("  Status: ✅ PERFORMANCE TARGET MET (>1.5x improvement)\n");
        return 1;
    } else if (improvement >= 1.2) {
        printf("  Status: ⚠️  MODERATE IMPROVEMENT (1.2-1.5x)\n");
        return 1;
    } else {
        printf("  Status: ❌ PERFORMANCE TARGET MISSED (<1.2x improvement)\n");
        return 0;
    }
}

int main(int argc, char* argv[]) {
    printf("Fixed-Point Math Optimization Validation\n");
    printf("==========================================\n\n");
    
    int correctness_passed = test_correctness();
    int performance_passed = test_performance();
    
    printf("\n=== FINAL RESULTS ===\n");
    if (correctness_passed && performance_passed) {
        printf("✅ All tests PASSED\n");
        printf("🚀 Fixed-point optimization validated and ready\n");
        return 0;
    } else {
        if (!correctness_passed) {
            printf("❌ Correctness tests FAILED\n");
        }
        if (!performance_passed) {
            printf("❌ Performance targets not met\n");
        }
        printf("⚠️  Optimization needs review\n");
        return 1;
    }
}