#ifndef _op_random_H_
#define _op_random_H_

#include "types.h"
#include "op.h"
#include "op_math.h"

//--- op_add_t : addition
typedef struct op_random_struct {
  op_t super;
  volatile io_t val; 
  volatile io_t min;
  volatile io_t max;
  volatile io_t trig;
  volatile io_t seed;
  volatile io_t * in_val[4]; // min, max, trig, seed
  op_out_t outs[1];

  u32 a,c,x;
  u16 range;
  u8 bitShift;
} op_random_t;
void op_random_init(void* mem);

// Legacy unpickle for pre-Nov 2017 scenes (before SEED input was added)
const u8* op_random_unpickle_legacy(op_random_t* op, const u8* src);

#endif // header guard
