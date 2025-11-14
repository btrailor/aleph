#include <stdlib.h>
#include <string.h>
#include "op_pool.h"
#include "print_funcs.h"
#include "memory.h"

static u8* smallOpData;

typedef struct _smallOpCons {
  u8* head;
  struct _smallOpCons *tail;
} smallOpCons;

static smallOpCons smallOpPool[MAX_SMALL_OPS];

static smallOpCons *smallOpHead;

static u8* mediumOpData;

typedef struct _mediumOpCons {
  u8* head;
  struct _mediumOpCons *tail;
} mediumOpCons;

static mediumOpCons mediumOpPool[MAX_MEDIUM_OPS];

static mediumOpCons *mediumOpHead;

static u8* bigOpData;

typedef struct _bigOpCons {
  u8* head;
  struct _bigOpCons *tail;
} bigOpCons;

static bigOpCons bigOpPool[MAX_BIG_OPS];

static bigOpCons *bigOpHead;


void initBigMemPool (void) {
  bigOpData = (u8*)alloc_mem(BIG_OP_SIZE * MAX_BIG_OPS);
  int i;
  for(i=0; i < MAX_BIG_OPS-1; i++) {
    bigOpPool[i].head = bigOpData + i * BIG_OP_SIZE;
    bigOpPool[i].tail = &(bigOpPool[i+1]);
  }
  bigOpPool[MAX_BIG_OPS-1].head = bigOpData + (MAX_BIG_OPS-1) * BIG_OP_SIZE;
  bigOpPool[MAX_BIG_OPS-1].tail = NULL;
  bigOpHead = bigOpPool;
}

u8* allocBigOp(void) {
  if (bigOpHead == NULL) {
    print_dbg("\r\nbigOpPool exhausted");
    return NULL;
  }
  u8* region = bigOpHead->head;
  bigOpHead = bigOpHead->tail;
  return region;
}

int freeBigOp(u8* region) {
  int idx = region - bigOpData;
  if(idx % BIG_OP_SIZE !=0) {
    print_dbg("\r\nWarning non-snapping chunk pointer (idx = ");
    print_dbg_ulong(idx);
    print_dbg(") passed to freeBigOp");
    return -1;
  }
  idx = idx / BIG_OP_SIZE;
  if (idx >= MAX_BIG_OPS || idx < 0) {
    print_dbg("\r\nWarning out-of-range chunk pointer passed to freeBigOp");
    return -1;
  }
  
  bigOpPool[idx].tail = bigOpHead;
  bigOpHead = &(bigOpPool[idx]);
  return 0;
}

void initMediumMemPool (void) {
  mediumOpData = (u8*)alloc_mem(MEDIUM_OP_SIZE * MAX_MEDIUM_OPS);
  
  if (mediumOpData == NULL) {
    print_dbg("\r\nERROR: alloc_mem failed for medium op pool!");
    return;
  }
  
  int i;
  for(i=0; i < MAX_MEDIUM_OPS-1; i++) {
    mediumOpPool[i].head = mediumOpData + i * MEDIUM_OP_SIZE;
    mediumOpPool[i].tail = &(mediumOpPool[i+1]);
  }
  mediumOpPool[MAX_MEDIUM_OPS-1].head = mediumOpData + (MAX_MEDIUM_OPS-1) * MEDIUM_OP_SIZE;
  mediumOpPool[MAX_MEDIUM_OPS-1].tail = NULL;
  mediumOpHead = mediumOpPool;
}

u8* allocMediumOp(void) {
  if (mediumOpHead == NULL) {
    print_dbg("\r\nmediumOpPool exhausted");
    return NULL;
  }
  u8* region = mediumOpHead->head;
  mediumOpHead = mediumOpHead->tail;
  return region;
}

int freeMediumOp(u8* region) {
  int idx = region - mediumOpData;
  if(idx % MEDIUM_OP_SIZE !=0) {
    print_dbg("\r\nWarning non-snapping chunk pointer (idx = ");
    print_dbg_ulong(idx);
    print_dbg(") passed to freeMediumOp");
    return -1;
  }
  idx = idx / MEDIUM_OP_SIZE;
  if (idx >= MAX_MEDIUM_OPS || idx < 0) {
    print_dbg("\r\nWarning out-of-range chunk pointer passed to freeMediumOp");
    return -1;
  }
  
  mediumOpPool[idx].tail = mediumOpHead;
  mediumOpHead = &(mediumOpPool[idx]);
  return 0;
}

void initSmallMemPool (void) {
  smallOpData = (u8*)alloc_mem(SMALL_OP_SIZE * MAX_SMALL_OPS);
  int i;
  for(i=0; i < MAX_SMALL_OPS-1; i++) {
    smallOpPool[i].head = smallOpData + i * SMALL_OP_SIZE;
    smallOpPool[i].tail = &(smallOpPool[i+1]);
  }
  smallOpPool[MAX_SMALL_OPS-1].head = smallOpData + (MAX_SMALL_OPS-1) * SMALL_OP_SIZE;
  smallOpPool[MAX_SMALL_OPS-1].tail = NULL;
  smallOpHead = smallOpPool;
}

u8* allocSmallOp(void) {
  if (smallOpHead == NULL) {
    print_dbg("\r\nsmallOpPool exhausted\n");
    return NULL;
  }
  u8* region = smallOpHead->head;
  smallOpHead = smallOpHead->tail;
  return region;
}

int freeSmallOp(u8* region) {
  int idx = region - smallOpData;
  if(idx % SMALL_OP_SIZE !=0) {
    print_dbg("\r\nWarning non-snapping chunk pointer (idx = ");
    print_dbg_ulong(idx);
    print_dbg(") passed to freeSmallOp");
    return -1;
  }
  idx = idx / SMALL_OP_SIZE;
  if (idx >= MAX_SMALL_OPS || idx < 0) {
    print_dbg("\r\nWarning out-of-range chunk pointer passed to freeSmallOp");
    return -1;
  }
  
  smallOpPool[idx].tail = smallOpHead;
  smallOpHead = &(smallOpPool[idx]);
  return 0;
}

int freeOp (u8* region) {
  int ret = freeSmallOp(region);
  if (ret != -1) {
    return ret;
  }
  ret = freeMediumOp(region);
  if (ret != -1) {
    return ret;
  }
  ret = freeBigOp(region);
  if (ret != -1) {
    return ret;
  }
  return 0;
}

// Pool statistics and monitoring functions
void getPoolStats(u32* smallUsed, u32* mediumUsed, u32* bigUsed) {
  // Count free blocks in each pool by walking the free lists
  u32 smallFree = 0, mediumFree = 0, bigFree = 0;
  
  // Count small pool free blocks
  smallOpCons* current = smallOpHead;
  while (current != NULL) {
    smallFree++;
    current = current->tail;
  }
  
  // Count medium pool free blocks
  mediumOpCons* mediumCurrent = mediumOpHead;
  while (mediumCurrent != NULL) {
    mediumFree++;
    mediumCurrent = mediumCurrent->tail;
  }
  
  // Count big pool free blocks
  bigOpCons* bigCurrent = bigOpHead;
  while (bigCurrent != NULL) {
    bigFree++;
    bigCurrent = bigCurrent->tail;
  }
  
  // Calculate used blocks
  *smallUsed = MAX_SMALL_OPS - smallFree;
  *mediumUsed = MAX_MEDIUM_OPS - mediumFree;
  *bigUsed = MAX_BIG_OPS - bigFree;
}

void printPoolUsage(void) {
  u32 smallUsed, mediumUsed, bigUsed;
  getPoolStats(&smallUsed, &mediumUsed, &bigUsed);
  
  print_dbg("\r\n=== Memory Pool Usage Statistics ===");
  
  print_dbg("\r\nSmall Pool (128 bytes):   ");
  print_dbg_ulong(smallUsed);
  print_dbg("/");
  print_dbg_ulong(MAX_SMALL_OPS);
  print_dbg(" used (");
  print_dbg_ulong((smallUsed * 100) / MAX_SMALL_OPS);
  print_dbg("%)");
  
  print_dbg("\r\nMedium Pool (2KB):        ");
  print_dbg_ulong(mediumUsed);
  print_dbg("/");
  print_dbg_ulong(MAX_MEDIUM_OPS);
  print_dbg(" used (");
  print_dbg_ulong((mediumUsed * 100) / MAX_MEDIUM_OPS);
  print_dbg("%)");
  
  print_dbg("\r\nBig Pool (16KB):          ");
  print_dbg_ulong(bigUsed);
  print_dbg("/");
  print_dbg_ulong(MAX_BIG_OPS);
  print_dbg(" used (");
  print_dbg_ulong((bigUsed * 100) / MAX_BIG_OPS);
  print_dbg("%)");
  
  // Calculate total memory usage
  u32 totalUsed = (smallUsed * SMALL_OP_SIZE) + (mediumUsed * MEDIUM_OP_SIZE) + (bigUsed * BIG_OP_SIZE);
  u32 totalAvailable = (MAX_SMALL_OPS * SMALL_OP_SIZE) + (MAX_MEDIUM_OPS * MEDIUM_OP_SIZE) + (MAX_BIG_OPS * BIG_OP_SIZE);
  
  print_dbg("\r\nTotal Memory:             ");
  print_dbg_ulong(totalUsed / 1024);
  print_dbg("KB/");
  print_dbg_ulong(totalAvailable / 1024);
  print_dbg("KB used (");
  print_dbg_ulong((totalUsed * 100) / totalAvailable);
  print_dbg("%)");
  print_dbg("\r\n====================================");
}
