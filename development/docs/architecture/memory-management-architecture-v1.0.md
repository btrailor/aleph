# Memory Management Architecture Design v1.0

## Aleph bees 1.0 Development - Phase 1

**Date:** November 6, 2025  
**Status:** Architecture Design Phase  
**Target:** Optimize boqs v0.8 memory management system

---

## Executive Summary

While boqs made significant improvements to memory management in v0.8 (replacing the dangerous linear allocation system with proper dual-pool management), there are still opportunities for optimization and addressing remaining FIXME comments. This document outlines the architecture for bees 1.0 memory management improvements.

## Current System Analysis (boqs v0.8)

### Existing Architecture

boqs v0.8 implemented a **dual-pool system** that replaced monome's dangerous linear allocation:

```c
// Current pool definitions
#define SMALL_OP_SIZE 128      // 128 bytes
#define MAX_SMALL_OPS 256      // 256 * 128 = 32KB total
#define BIG_OP_SIZE (1024 * 16) // 16KB
#define MAX_BIG_OPS 8          // 8 * 16KB = 128KB total
                               // Total: 160KB allocated
```

### Memory Pool Implementation

- **Free List Management**: Proper linked list recycling
- **Size-Based Allocation**: Automatic small vs big pool selection
- **Pointer Validation**: Safety checks on deallocation
- **Pool Exhaustion Handling**: Graceful failure with debug output

### Identified Optimization Opportunities

#### 1. **Magic Number Issues** (FIXME Comments Found)

```c
// net.h line 16: "FIXME: need to fix malloc()"
#define NET_INS_MAX 256    // Magic number
#define NET_OUTS_MAX 256   // Magic number
#define NET_OPS_MAX 128    // Magic number
#define NET_PARAMS_MAX 256 // Magic number
```

#### 2. **Performance Bottlenecks** (FIXME Comments)

```c
// net.c line 1462: "FIXME: net_op_in_idx is pretty slow"
// net.c line 1477: "FIXME: net_op_in_idx is pretty slow"
```

#### 3. **Pool Size Optimization**

Current pool allocation may not be optimal for real-world usage patterns:

- Small pool: 256 × 128 bytes = 32KB
- Big pool: 8 × 16KB = 128KB
- **Question**: Are these ratios optimal for typical bees usage?

## Proposed Architecture v1.0

### Phase 1A: Pool Size Analysis and Optimization

#### Memory Usage Profiling System

```c
// New profiling structures
typedef struct {
    u32 small_allocs;
    u32 big_allocs;
    u32 small_frees;
    u32 big_frees;
    u32 small_peak_usage;
    u32 big_peak_usage;
    u32 allocation_failures;
    u32 fragmentation_events;
} memory_profile_t;
```

#### Dynamic Pool Sizing

Replace magic numbers with configurable limits based on available SDRAM:

```c
// Calculate optimal pool sizes based on available memory
void calculate_optimal_pools(u32 available_sdram) {
    // Reserve memory for other systems
    u32 available_for_ops = available_sdram * 0.6; // 60% for operators

    // Analyze historical usage to determine small:big ratio
    // Default: 20% small (many operators), 80% big (few large operators)
    u32 small_pool_memory = available_for_ops * 0.2;
    u32 big_pool_memory = available_for_ops * 0.8;

    // Calculate pool counts
    u32 max_small_ops = small_pool_memory / SMALL_OP_SIZE;
    u32 max_big_ops = big_pool_memory / BIG_OP_SIZE;
}
```

### Phase 1B: Performance Optimization

#### Fast Lookup Tables

Address the "net_op_in_idx is pretty slow" FIXME:

```c
// Replace linear search with hash table or direct indexing
typedef struct {
    u16 op_to_in_idx[NET_OPS_MAX];    // Direct mapping
    u16 op_to_out_idx[NET_OPS_MAX];   // Direct mapping
    bool indices_valid;                // Cache validity flag
} fast_lookup_t;
```

#### Memory Pool Coalescing

For advanced optimization, implement adjacent block coalescing:

```c
// Detect and merge adjacent free blocks in big pool
int coalesce_big_pool_blocks(void) {
    // Scan for adjacent free blocks and merge them
    // This could allow for larger allocations than BIG_OP_SIZE
}
```

### Phase 1C: Enhanced Safety and Debugging

#### Memory Leak Detection

```c
// Track all allocations for leak detection
typedef struct allocation_record {
    void* ptr;
    size_t size;
    const char* file;
    int line;
    u32 timestamp;
    struct allocation_record* next;
} allocation_record_t;

// Debug macros
#ifdef DEBUG_MEMORY
#define ALLOC_SMALL_OP() debug_alloc_small_op(__FILE__, __LINE__)
#define ALLOC_BIG_OP() debug_alloc_big_op(__FILE__, __LINE__)
#else
#define ALLOC_SMALL_OP() allocSmallOp()
#define ALLOC_BIG_OP() allocBigOp()
#endif
```

#### Pool Corruption Detection

```c
// Add canary values to detect buffer overruns
typedef struct {
    u32 canary_start;     // 0xDEADBEEF
    u8 data[SMALL_OP_SIZE - 8];
    u32 canary_end;       // 0xBEEFDEAD
} safe_small_op_t;
```

## Implementation Plan

### Phase 1A: Analysis (Week 1)

1. **Memory Usage Profiling**

   - Add profiling hooks to existing allocation functions
   - Collect usage data from typical bees sessions
   - Analyze small vs big pool utilization patterns

2. **Pool Size Optimization**
   - Calculate optimal pool ratios based on profiling data
   - Implement dynamic pool sizing based on available SDRAM
   - Test performance impact of different pool configurations

### Phase 1B: Performance (Week 2)

1. **Fast Lookup Implementation**

   - Replace linear searches with direct indexing where possible
   - Implement hash tables for complex lookups
   - Benchmark performance improvements

2. **Address Critical FIXMEs**
   - Fix `net_op_in_idx` performance bottleneck
   - Replace magic numbers with calculated values
   - Optimize parameter indexing ("this is horrible" comments)

### Phase 1C: Safety (Week 3)

1. **Debug Infrastructure**

   - Implement memory leak detection (debug builds only)
   - Add canary-based corruption detection
   - Create memory usage reporting tools

2. **Advanced Features**
   - Implement big pool coalescing for large allocations
   - Add memory pressure handling (graceful degradation)
   - Create memory pool statistics API

## Success Metrics

### Performance Targets

- **Allocation Speed**: 50% improvement in operator creation time
- **Memory Efficiency**: 90%+ pool utilization under typical loads
- **Search Performance**: O(1) lookup for op_in_idx and op_out_idx operations

### Reliability Targets

- **Zero Memory Leaks**: Perfect cleanup in all test scenarios
- **Corruption Detection**: 100% detection of buffer overruns
- **Graceful Degradation**: No crashes under memory pressure

### Memory Utilization Targets

- **Optimal Pool Ratios**: Data-driven small:big pool sizing
- **Reduced Waste**: <5% internal fragmentation
- **Dynamic Scaling**: Automatic adjustment to available SDRAM

## Risk Assessment

### Low Risk

- Pool size optimization (data-driven approach)
- Debug infrastructure additions (ifdef protected)

### Medium Risk

- Fast lookup table implementation (requires careful indexing)
- Memory profiling hooks (potential performance impact)

### High Risk

- Big pool coalescing (complex pointer management)
- Dynamic pool sizing (changes initialization order)

## Next Steps

1. **Implement memory profiling hooks** in existing v0.8 system
2. **Collect baseline performance data** from current implementation
3. **Design and test fast lookup tables** for performance-critical operations
4. **Create comprehensive test suite** for memory management validation

---

## Appendix A: Current FIXME Comments Analysis

| File  | Line | Issue                                   | Priority |
| ----- | ---- | --------------------------------------- | -------- |
| net.h | 16   | "need to fill malloc()"                 | High     |
| net.c | 1462 | "net_op_in_idx is pretty slow"          | High     |
| net.c | 1477 | "net_op_in_idx is pretty slow"          | High     |
| net.c | 352  | "this is horrible" (parameter indexing) | Medium   |
| net.c | 413  | "this is horrible" (parameter indexing) | Medium   |
| net.c | 75   | "magic # bs" (switch indexing)          | Low      |

## Appendix B: Memory Layout Analysis

```
Current v0.8 Memory Layout:
┌─────────────────────────────────────────────────────────────┐
│ Small Pool: 32KB (256 × 128 bytes)                         │
├─────────────────────────────────────────────────────────────┤
│ Big Pool: 128KB (8 × 16KB)                                 │
├─────────────────────────────────────────────────────────────┤
│ Free List Headers: ~2KB                                    │
├─────────────────────────────────────────────────────────────┤
│ Network Structures: ~8KB                                   │
└─────────────────────────────────────────────────────────────┘
Total: ~170KB for operator memory management
```

Proposed v1.0 optimizations will maintain this layout while improving utilization and performance.
