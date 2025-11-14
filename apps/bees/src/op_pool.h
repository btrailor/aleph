#ifndef _ALEPH_BEES_OP_POOL_H_
#define _ALEPH_BEES_OP_POOL_H_

#include <stdlib.h>
#include <string.h>
#include "types.h"


#define SMALL_OP_SIZE 128
#define MAX_SMALL_OPS 500  // Increased for beekeep

#define MEDIUM_OP_SIZE 2048
#define MAX_MEDIUM_OPS 200  // Increased for beekeep

#define BIG_OP_SIZE (1024 * 16)
#define MAX_BIG_OPS 50     // Increased for beekeep

void initBigMemPool (void);
u8* allocBigOp(void);

void initMediumMemPool (void);
u8* allocMediumOp(void);

void initSmallMemPool (void);
u8* allocSmallOp(void);

int freeBigOp(u8* region);
int freeMediumOp(u8* region);
int freeSmallOp(u8* region);
int freeOp(u8* region);

// Pool statistics and monitoring
void getPoolStats(u32* smallUsed, u32* mediumUsed, u32* bigUsed);
void printPoolUsage(void);

#endif
