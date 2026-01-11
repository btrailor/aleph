# Real-time Performance Baseline
## Aleph bees 1.0 Development - Phase 1

**Date:** November 6, 2025  
**Status:** Baseline Strategy and Test Plan  
**Target:** Establish measurable performance targets for optimization validation

---

## Executive Summary

This document establishes a comprehensive performance baseline for the Aleph bees 1.0 optimization project. Based on our analysis of critical bottlenecks (fixed-point math, memory management, network operations), we define measurable benchmarks to validate improvements and ensure real-time performance requirements are maintained.

## Performance Critical Areas Identified

### 1. **Fixed-Point Math Operations** (CRITICAL)
From our Fixed-Point Math Performance Audit:
- **Modulus-heavy integer conversion** (`u % 10` + `u /= 10`)
- **Called frequently** during parameter display updates
- **Expected improvement**: 2x performance gain

### 2. **Network Operations** (CRITICAL) 
From our FIXME Comments Catalog:
- **`net_op_in_idx` performance bottleneck** (marked "pretty slow")
- **Parameter indexing inefficiencies** ("this is horrible" comments)
- **Expected improvement**: 10x performance gain with hash tables

### 3. **Memory Pool Operations** (HIGH)
From our Memory Pool System Analysis:
- **Allocation/deallocation performance** (currently O(1))
- **Pool exhaustion scenarios** (big pool limited to 8 operators)
- **Expected improvement**: Better utilization, same speed

### 4. **Graphics Operations** (MEDIUM)
From our analysis:
- **"Dumb memory hack"** in BIGNUM and BARS8 operators
- **Static allocation inefficiencies**
- **Expected improvement**: Reduced memory waste

## Baseline Testing Strategy

### Phase 1: Synthetic Benchmarks

#### 1.1 Fixed-Point Math Baseline
```c
// Benchmark: Integer-to-string conversion performance
void benchmark_fixed_point_conversion(void) {
    uint32_t start_cycles, end_cycles;
    int test_values[] = {0, 1, 42, 1337, 2147483647, -1, -42};
    char buffer[16];
    
    for (int i = 0; i < 7; i++) {
        start_cycles = get_cycle_count();
        
        // Current implementation (with modulus)
        for (int j = 0; j < 1000; j++) {
            itoa_whole(test_values[i], buffer, 10);
        }
        
        end_cycles = get_cycle_count();
        print_dbg("Value: %d, Cycles: %u, Per-conversion: %u\n", 
                  test_values[i], 
                  end_cycles - start_cycles,
                  (end_cycles - start_cycles) / 1000);
    }
}
```

**Target Metrics:**
- **Current baseline**: TBD cycles per conversion
- **Improvement target**: 50% reduction in cycles
- **Test frequency**: Parameter display scenarios (10-100 conversions/second)

#### 1.2 Network Operations Baseline
```c
// Benchmark: Operator input/output indexing performance
void benchmark_network_operations(void) {
    uint32_t start_cycles, end_cycles;
    
    // Test net_op_in_idx performance (currently marked as slow)
    start_cycles = get_cycle_count();
    for (int i = 0; i < 1000; i++) {
        for (int op = 0; op < net->numOps; op++) {
            net_op_in_idx(op, 0);  // Find first input of each operator
        }
    }
    end_cycles = get_cycle_count();
    
    print_dbg("net_op_in_idx benchmark: %u cycles per lookup\n",
              (end_cycles - start_cycles) / (1000 * net->numOps));
}
```

**Target Metrics:**
- **Current baseline**: TBD cycles per lookup
- **Improvement target**: 10x reduction with hash tables
- **Test frequency**: Real-time parameter routing (100-1000 lookups/second)

#### 1.3 Memory Pool Baseline
```c
// Benchmark: Memory pool allocation/deallocation performance
void benchmark_memory_pools(void) {
    uint32_t start_cycles, end_cycles;
    void* allocated_ptrs[100];
    
    // Small pool allocation benchmark
    start_cycles = get_cycle_count();
    for (int i = 0; i < 100; i++) {
        allocated_ptrs[i] = allocSmallOp();
    }
    end_cycles = get_cycle_count();
    print_dbg("Small pool allocation: %u cycles per alloc\n",
              (end_cycles - start_cycles) / 100);
    
    // Small pool deallocation benchmark  
    start_cycles = get_cycle_count();
    for (int i = 0; i < 100; i++) {
        freeSmallOp(allocated_ptrs[i]);
    }
    end_cycles = get_cycle_count();
    print_dbg("Small pool deallocation: %u cycles per free\n",
              (end_cycles - start_cycles) / 100);
}
```

**Target Metrics:**
- **Current baseline**: Should be ~10-20 cycles (O(1) operations)
- **Improvement target**: Maintain performance, improve utilization
- **Test frequency**: Operator creation/deletion (1-10 operations/second)

### Phase 2: Real-World Scenario Testing

#### 2.1 Typical bees Session Simulation
```c
// Simulate typical user workflow
void benchmark_typical_session(void) {
    uint32_t session_start, session_end;
    
    session_start = get_cycle_count();
    
    // 1. Create basic network (math operators)
    for (int i = 0; i < 10; i++) {
        net_add_op(eOpAdd);
        net_add_op(eOpMul);
    }
    
    // 2. Add complex operators
    net_add_op(eOpBignum);
    net_add_op(eOpBars8);
    
    // 3. Create connections
    for (int i = 0; i < 15; i++) {
        net_connect(i*2, i*2+1);  // Connect operators
    }
    
    // 4. Simulate parameter updates (UI activity)
    for (int frame = 0; frame < 100; frame++) {
        for (int param = 0; param < 20; param++) {
            set_param_value(param, frame * param);
        }
    }
    
    session_end = get_cycle_count();
    
    print_dbg("Typical session benchmark: %u total cycles\n",
              session_end - session_start);
}
```

#### 2.2 Stress Testing Scenarios
```c
// Test pool exhaustion and recovery
void benchmark_stress_scenarios(void) {
    // Test 1: Big pool exhaustion
    print_dbg("Testing big pool exhaustion...\n");
    for (int i = 0; i < 10; i++) {  // Try to allocate 10 big ops (limit is 8)
        void* ptr = allocBigOp();
        if (ptr == NULL) {
            print_dbg("Big pool exhausted at allocation %d\n", i);
            break;
        }
    }
    
    // Test 2: Rapid allocation/deallocation
    uint32_t start_cycles = get_cycle_count();
    for (int cycle = 0; cycle < 1000; cycle++) {
        void* ptr = allocSmallOp();
        if (ptr) freeSmallOp(ptr);
    }
    uint32_t end_cycles = get_cycle_count();
    
    print_dbg("Rapid alloc/free: %u cycles per cycle\n",
              (end_cycles - start_cycles) / 1000);
}
```

## Baseline Implementation Plan

### Week 1: Infrastructure Setup
1. **Create benchmark framework**
   - Implement cycle counting utilities for both platforms
   - Create standardized test harness
   - Set up automated baseline collection

2. **Platform-specific timing**
   - **Blackfin**: Use cycle counter registers
   - **AVR32**: Use hardware timer peripherals
   - **Simulation**: Use host system timing

### Week 2: Core Benchmarks
1. **Fixed-point math baseline**
   - Measure current conversion performance
   - Test with various input ranges
   - Document per-operation costs

2. **Network operations baseline**
   - Profile net_op_in_idx performance
   - Measure parameter indexing costs
   - Test with various network sizes

### Week 3: Real-world Testing
1. **Scenario-based benchmarks**
   - Typical user workflows
   - Stress testing edge cases
   - Performance under load

2. **Documentation and analysis**
   - Create performance profile database
   - Identify optimization priorities
   - Set improvement targets

## Expected Baseline Results

### Projected Performance Profile

| Component | Current Est. | Post-Optimization Target | Improvement |
|-----------|--------------|-------------------------|-------------|
| **Fixed-point conversion** | 200 cycles | 100 cycles | 2x faster |
| **Network op lookup** | 500 cycles | 50 cycles | 10x faster |
| **Memory allocation** | 15 cycles | 15 cycles | No change |
| **Parameter update** | 1000 cycles | 400 cycles | 2.5x faster |

### Real-time Performance Targets

| Metric | Current (Est.) | Target | Requirement |
|--------|----------------|---------|-------------|
| **Audio frame processing** | 80% CPU | 60% CPU | <75% for headroom |
| **UI responsiveness** | 50ms | 20ms | <30ms perceptible |
| **Parameter updates** | 10/sec | 25/sec | Smooth real-time control |
| **Network operations** | 100/sec | 500/sec | Complex routing support |

## Success Criteria

### Performance Validation
- **All benchmarks complete** within 1 week of implementation
- **Baseline database established** with repeatable measurements
- **Optimization targets defined** based on real-world requirements

### Real-time Compliance
- **No timing regressions** in critical audio paths
- **Improved UI responsiveness** measurable in user scenarios
- **Stress test survival** under memory pressure and high load

## Risk Mitigation

### Benchmark Accuracy
- **Multiple platform testing** to ensure representative results
- **Statistical sampling** to account for timing variations
- **Real hardware validation** beyond simulation results

### Optimization Impact
- **Before/after comparison** framework for validation
- **Regression testing** for performance changes
- **Rollback capability** if optimizations harm real-time performance

## Next Steps

1. **Implement cycle counting infrastructure** for accurate timing
2. **Create benchmark test suite** based on identified critical areas
3. **Establish baseline measurements** on target hardware
4. **Document performance profile** for optimization guidance

---

## Appendix A: Platform-Specific Timing

### Blackfin DSP Timing
```c
// Use Blackfin cycle counter
static inline uint32_t get_blackfin_cycles(void) {
    uint32_t cycles;
    __asm__ volatile ("R0 = CYCLES; %0 = R0;" : "=r" (cycles) :: "R0");
    return cycles;
}
```

### AVR32 Controller Timing  
```c
// Use AVR32 timer counter
static inline uint32_t get_avr32_cycles(void) {
    return Get_sys_count();  // System timer count
}
```

### Cross-platform Wrapper
```c
// Unified timing interface
static inline uint32_t get_cycle_count(void) {
#ifdef ARCH_BFIN
    return get_blackfin_cycles();
#elif ARCH_AVR32
    return get_avr32_cycles();
#else
    return 0;  // Simulation fallback
#endif
}
```

## Appendix B: Test Data Sets

### Fixed-Point Test Values
- **Edge cases**: 0, 1, -1, INT_MAX, INT_MIN
- **Common values**: 42, 100, 1000, 9999
- **Random sampling**: 1000 random values across full range

### Network Test Scenarios
- **Small networks**: 5-10 operators
- **Medium networks**: 20-50 operators  
- **Large networks**: 100+ operators (stress test)
- **Complex routing**: Dense interconnection patterns

### Memory Test Patterns
- **Sequential allocation**: Linear allocation patterns
- **Random allocation**: Mixed small/big operator creation
- **Fragmentation testing**: Create/destroy patterns that stress the allocator