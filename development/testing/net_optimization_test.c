#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

// Mock the network structures for testing
#define NET_OPS_MAX 128
#define NET_INS_MAX 256
#define NET_OUTS_MAX 256

typedef struct {
    int32_t opIdx;
    uint8_t opInIdx;
    uint8_t play;
} inode_t;

typedef struct {
    uint8_t opOutIdx;
    int16_t target;
    int32_t opIdx;
} onode_t;

typedef struct {
    void* ops[NET_OPS_MAX];
    uint16_t numOps;
    uint16_t numIns;
    uint16_t numOuts;
    uint16_t numParams;
    inode_t ins[NET_INS_MAX];
    onode_t outs[NET_OUTS_MAX];
} ctlnet_t;

// Global network pointer (like in the real code)
ctlnet_t test_net;
ctlnet_t* net = &test_net;

// Mock print functions
void print_dbg(const char* str) { /* printf("%s", str); */ }
void print_dbg_ulong(unsigned long val) { /* printf("%lu", val); */ }

// Inline optimization code (self-contained test)
// Lookup table: operator index -> first input index
// -1 indicates operator has no inputs
static int16_t op_first_input_idx[NET_OPS_MAX];
static uint8_t lookup_table_initialized = 0;

// Initialize the lookup table by scanning existing network
void net_optimize_init_lookup_table(void) {
    uint16_t op_idx, in_idx;
    
    // Initialize all entries to -1 (no inputs)
    for (op_idx = 0; op_idx < NET_OPS_MAX; op_idx++) {
        op_first_input_idx[op_idx] = -1;
    }
    
    // Scan existing inputs to build lookup table
    for (in_idx = 0; in_idx < net->numIns; in_idx++) {
        op_idx = net->ins[in_idx].opIdx;
        
        // If this is the first input we've seen for this operator
        if (op_first_input_idx[op_idx] == -1) {
            op_first_input_idx[op_idx] = in_idx;
        }
    }
    
    lookup_table_initialized = 1;
}

// Optimized version of net_op_in_idx using lookup table
uint16_t net_op_in_idx_optimized(const uint16_t op_idx, const uint16_t in_idx) {
    if (!lookup_table_initialized) {
        net_optimize_init_lookup_table();
    }
    
    if (op_idx >= NET_OPS_MAX) {
        return 0;  // Invalid operator index
    }
    
    int16_t first_input = op_first_input_idx[op_idx];
    if (first_input == -1) {
        return 0;  // Operator has no inputs
    }
    
    return first_input + in_idx;
}

// Reference implementation (original linear search)
uint16_t net_op_in_idx_reference(const uint16_t op_idx, const uint16_t in_idx) {
    uint16_t which;
    for (which = 0; which < net->numIns; which++) {
        if (net->ins[which].opIdx == op_idx) {
            return (which + in_idx);
        }
    }
    return 0; // get here if op has no inputs
}

// Verify lookup table correctness (for testing)
uint8_t net_optimize_verify_lookup_table(void) {
    uint16_t op_idx;
    uint8_t errors = 0;
    
    for (op_idx = 0; op_idx < net->numOps; op_idx++) {
        // Test first input (in_idx = 0)
        uint16_t reference_result = net_op_in_idx_reference(op_idx, 0);
        uint16_t optimized_result = net_op_in_idx_optimized(op_idx, 0);
        
        if (reference_result != optimized_result) {
            printf("VERIFY FAILED: op=%d ref=%d opt=%d\n", 
                   op_idx, reference_result, optimized_result);
            errors++;
        }
    }
    
    return errors == 0;
}

// Create a test network with some operators
void setup_test_network() {
    memset(&test_net, 0, sizeof(test_net));
    
    // Simulate adding several operators with different input counts
    // Op 0: 2 inputs (ins[0], ins[1])
    test_net.ins[0].opIdx = 0; test_net.ins[0].opInIdx = 0;
    test_net.ins[1].opIdx = 0; test_net.ins[1].opInIdx = 1;
    
    // Op 1: 0 inputs (no inputs)
    
    // Op 2: 3 inputs (ins[2], ins[3], ins[4])
    test_net.ins[2].opIdx = 2; test_net.ins[2].opInIdx = 0;
    test_net.ins[3].opIdx = 2; test_net.ins[3].opInIdx = 1;
    test_net.ins[4].opIdx = 2; test_net.ins[4].opInIdx = 2;
    
    // Op 3: 1 input (ins[5])
    test_net.ins[5].opIdx = 3; test_net.ins[5].opInIdx = 0;
    
    // Op 4: 4 inputs (ins[6], ins[7], ins[8], ins[9])
    test_net.ins[6].opIdx = 4; test_net.ins[6].opInIdx = 0;
    test_net.ins[7].opIdx = 4; test_net.ins[7].opInIdx = 1;
    test_net.ins[8].opIdx = 4; test_net.ins[8].opInIdx = 2;
    test_net.ins[9].opIdx = 4; test_net.ins[9].opInIdx = 3;
    
    test_net.numOps = 5;
    test_net.numIns = 10;
    test_net.numOuts = 0;
    test_net.numParams = 0;
}

int test_correctness() {
    printf("=== CORRECTNESS TESTING ===\n");
    
    setup_test_network();
    
    // Test cases: [op_idx, in_idx, expected_result]
    struct {
        uint16_t op_idx;
        uint16_t in_idx;
        uint16_t expected;
        const char* description;
    } test_cases[] = {
        {0, 0, 0, "Op 0, first input"},
        {0, 1, 1, "Op 0, second input"},
        {1, 0, 0, "Op 1, no inputs (should return 0)"},
        {2, 0, 2, "Op 2, first input"},
        {2, 1, 3, "Op 2, second input"},
        {2, 2, 4, "Op 2, third input"},
        {3, 0, 5, "Op 3, only input"},
        {4, 0, 6, "Op 4, first input"},
        {4, 1, 7, "Op 4, second input"},
        {4, 2, 8, "Op 4, third input"},
        {4, 3, 9, "Op 4, fourth input"},
    };
    
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int failures = 0;
    
    for (int i = 0; i < num_tests; i++) {
        uint16_t ref_result = net_op_in_idx_reference(test_cases[i].op_idx, test_cases[i].in_idx);
        uint16_t opt_result = net_op_in_idx_optimized(test_cases[i].op_idx, test_cases[i].in_idx);
        
        if (ref_result != opt_result || ref_result != test_cases[i].expected) {
            printf("FAIL: %s\n", test_cases[i].description);
            printf("  Op %d, In %d: expected=%d, reference=%d, optimized=%d\n",
                   test_cases[i].op_idx, test_cases[i].in_idx, 
                   test_cases[i].expected, ref_result, opt_result);
            failures++;
        } else {
            printf("PASS: %s -> %d\n", test_cases[i].description, opt_result);
        }
    }
    
    // Test automatic verification function
    uint8_t verify_result = net_optimize_verify_lookup_table();
    if (!verify_result) {
        printf("FAIL: Automatic verification failed\n");
        failures++;
    } else {
        printf("PASS: Automatic verification passed\n");
    }
    
    printf("\nCorrectness Results: %d/%d tests passed\n", num_tests + 1 - failures, num_tests + 1);
    return failures == 0;
}

int test_performance() {
    printf("\n=== PERFORMANCE TESTING ===\n");
    
    setup_test_network();
    
    // Create larger test network for better performance measurement
    printf("Creating larger test network...\n");
    
    // Add many more operators and inputs
    uint16_t input_idx = 10;  // Start after existing inputs
    for (uint16_t op = 5; op < 50; op++) {
        // Each operator gets 2-5 inputs
        uint16_t num_inputs = 2 + (op % 4);
        for (uint16_t in = 0; in < num_inputs; in++) {
            if (input_idx < NET_INS_MAX) {
                test_net.ins[input_idx].opIdx = op;
                test_net.ins[input_idx].opInIdx = in;
                input_idx++;
            }
        }
    }
    test_net.numOps = 50;
    test_net.numIns = input_idx;
    
    printf("Test network: %d operators, %d inputs\n", test_net.numOps, test_net.numIns);
    
    int iterations = 100000;
    clock_t start, end;
    double ref_time, opt_time;
    
    // Test reference implementation
    printf("Testing reference implementation...\n");
    start = clock();
    for (int iter = 0; iter < iterations; iter++) {
        for (uint16_t op = 0; op < test_net.numOps; op++) {
            net_op_in_idx_reference(op, 0);  // Get first input of each op
        }
    }
    end = clock();
    ref_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    // Test optimized implementation
    printf("Testing optimized implementation...\n");
    start = clock();
    for (int iter = 0; iter < iterations; iter++) {
        for (uint16_t op = 0; op < test_net.numOps; op++) {
            net_op_in_idx_optimized(op, 0);  // Get first input of each op
        }
    }
    end = clock();
    opt_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    // Calculate improvement
    double improvement = ref_time / opt_time;
    double percent_improvement = ((ref_time - opt_time) / ref_time) * 100;
    
    printf("Performance Results:\n");
    printf("  Reference time:    %.6f seconds\n", ref_time);
    printf("  Optimized time:    %.6f seconds\n", opt_time);
    printf("  Improvement:       %.2fx faster\n", improvement);
    printf("  Percent reduction: %.1f%%\n", percent_improvement);
    
    if (improvement >= 5.0) {
        printf("  Status: ✅ EXCELLENT PERFORMANCE (>5x improvement)\n");
        return 1;
    } else if (improvement >= 2.0) {
        printf("  Status: ✅ GOOD PERFORMANCE (2-5x improvement)\n");
        return 1;
    } else if (improvement >= 1.2) {
        printf("  Status: ⚠️  MODERATE IMPROVEMENT (1.2-2x)\n");
        return 1;
    } else {
        printf("  Status: ❌ PERFORMANCE TARGET MISSED (<1.2x improvement)\n");
        return 0;
    }
}

int main() {
    printf("Network Operations Optimization Test\n");
    printf("====================================\n\n");
    
    int correctness_passed = test_correctness();
    int performance_passed = test_performance();
    
    printf("\n=== FINAL RESULTS ===\n");
    if (correctness_passed && performance_passed) {
        printf("✅ All tests PASSED\n");
        printf("🚀 Network operations optimization validated and ready\n");
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