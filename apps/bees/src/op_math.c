#include "op_math.h"


// standard (overflow)
io_t op_add(io_t a, io_t b) { return ((a) + (b)); }
io_t op_sub(io_t a, io_t b) { return ((a) - (b)); }
io_t op_mul(io_t a, io_t b) { return ((a) * (b)); }
io_t op_div(io_t a, io_t b) { return ((a) / (b)); }

/// saturating add
io_t op_sadd(io_t a, io_t b) {
  // Optimized 16-bit saturating addition
  // Use fast saturation with compiler-friendly code
  s32 res32 = (s32)a + (s32)b;
  
  // Clamp to 16-bit signed range [-32768, 32767]
  if (res32 > 32767) {
    return 32767;
  }
  if (res32 < -32768) {
    return -32768;
  }
  return (s16)res32;
}

/// saturating subtract
io_t op_ssub(io_t a, io_t b) {
  // Optimized 16-bit saturating subtraction
  // Use fast saturation with compiler-friendly code
  s32 res32 = (s32)a - (s32)b;
  
  // Clamp to 16-bit signed range [-32768, 32767]
  if (res32 > 32767) {
    return 32767;
  }
  if (res32 < -32768) {
    return -32768;
  }
  return (s16)res32;
}

// print
void op_print(char* buf, io_t x) { itoa_whole((s32)(x), (buf), 6); }
