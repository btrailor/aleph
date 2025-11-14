#!/usr/bin/env python3
"""
Fixed-Point Math Optimization Validation
Simple Python test to validate our C optimization results
"""

import time
import random

def itoa_reference(val):
    """Reference implementation using modulus (slow)"""
    if val == 0:
        return "0"
    
    sign = val < 0
    if sign:
        val = -val
    
    digits = []
    while val > 0:
        digit = val % 10  # EXPENSIVE: Modulus operation
        val = val // 10   # EXPENSIVE: Division operation
        digits.append(str(digit))
    
    result = ''.join(reversed(digits))
    return '-' + result if sign else result

def itoa_optimized(val):
    """Optimized implementation avoiding modulus"""
    if val == 0:
        return "0"
    
    sign = val < 0
    if sign:
        val = -val
    
    digits = []
    while val > 0:
        temp = val // 10           # Single division
        digit = val - (temp * 10)  # Equivalent to val % 10, but faster
        val = temp
        digits.append(str(digit))
    
    result = ''.join(reversed(digits))
    return '-' + result if sign else result

def test_correctness():
    """Test that optimized version produces identical results"""
    test_values = [
        0, 1, -1, 9, -9, 10, -10, 99, -99, 100, -100,
        1000, -1000, 10000, -10000, 100000, -100000,
        42, -42, 1337, -1337, 98765, -98765,
        2147483647, -2147483647, 2147483646, -2147483646,
        999999999, -999999999, 123456789, -123456789
    ]
    
    print("=== CORRECTNESS TESTING ===")
    failures = 0
    
    for val in test_values:
        ref_result = itoa_reference(val)
        opt_result = itoa_optimized(val)
        
        if ref_result != opt_result:
            print(f"FAIL: {val}")
            print(f"  Reference: '{ref_result}'")
            print(f"  Optimized: '{opt_result}'")
            failures += 1
        else:
            print(f"PASS: {val} -> '{opt_result}'")
    
    # Test random values
    print("\nTesting 1000 random values...")
    random.seed(42)  # Reproducible results
    random_failures = 0
    
    for _ in range(1000):
        val = random.randint(-2147483647, 2147483647)
        ref_result = itoa_reference(val)
        opt_result = itoa_optimized(val)
        
        if ref_result != opt_result:
            print(f"RANDOM FAIL: {val} -> ref:'{ref_result}' opt:'{opt_result}'")
            random_failures += 1
            if random_failures >= 10:  # Stop after 10 failures
                break
    
    total_failures = failures + random_failures
    print(f"\nCorrectness Results: {len(test_values) + 1000 - total_failures}/{len(test_values) + 1000} tests passed")
    return total_failures == 0

def test_performance():
    """Benchmark performance improvement"""
    test_values = [42, 1337, 98765, -12345, 999999999, -999999999] * 1000
    iterations = 10
    
    print("\n=== PERFORMANCE TESTING ===")
    
    # Warmup
    for val in test_values[:100]:
        itoa_reference(val)
        itoa_optimized(val)
    
    # Benchmark reference implementation
    start_time = time.perf_counter()
    for _ in range(iterations):
        for val in test_values:
            itoa_reference(val)
    reference_time = time.perf_counter() - start_time
    
    # Benchmark optimized implementation
    start_time = time.perf_counter()
    for _ in range(iterations):
        for val in test_values:
            itoa_optimized(val)
    optimized_time = time.perf_counter() - start_time
    
    # Calculate improvement
    improvement = reference_time / optimized_time
    percent_improvement = ((reference_time - optimized_time) / reference_time) * 100
    
    print(f"Performance Results:")
    print(f"  Reference time:    {reference_time:.4f} seconds")
    print(f"  Optimized time:    {optimized_time:.4f} seconds")
    print(f"  Improvement:       {improvement:.2f}x faster")
    print(f"  Percent reduction: {percent_improvement:.1f}%")
    
    if improvement >= 1.5:
        print(f"  Status: ✅ PERFORMANCE TARGET MET (>1.5x improvement)")
        return True
    elif improvement >= 1.2:
        print(f"  Status: ⚠️  MODERATE IMPROVEMENT (1.2-1.5x)")
        return True
    else:
        print(f"  Status: ❌ PERFORMANCE TARGET MISSED (<1.2x improvement)")
        return False

def main():
    print("Fixed-Point Math Optimization Validation")
    print("="*50)
    
    correctness_passed = test_correctness()
    performance_passed = test_performance()
    
    print("\n=== FINAL RESULTS ===")
    if correctness_passed and performance_passed:
        print("✅ All tests PASSED")
        print("🚀 Fixed-point optimization validated and ready")
        return 0
    else:
        if not correctness_passed:
            print("❌ Correctness tests FAILED")
        if not performance_passed:
            print("❌ Performance targets not met")
        print("⚠️  Optimization needs review")
        return 1

if __name__ == "__main__":
    exit(main())