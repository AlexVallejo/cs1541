#ifndef __SKELETON_H__
#define __SKELETON_H__

///////////////////////////////////////////////////////////////////////////////
//
// CS 1541 Introduction to Computer Architecture
// Use this skeleton code to create a cache instance and implement cache operations.
// Feel free to add new variables in the structure if needed.
//
///////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>

struct cache_blk_t {
  unsigned long tag;
  char valid;
  char dirty;
  /* Add either a pointer (to construct FIFO or LRU lists)
   or a time stamp to implement the replacement polity */
};

enum cache_policy {
  LRU,
  FIFO
};

struct cache_t {
  int nsets;    // # sets
  int bsize;    // block size
  int assoc;    // associativity

  enum cache_policy policy;       // cache replacement policy
  struct cache_blk_t **blocks;    // cache blocks in the cache
};

struct cache_t *
cache_create(int size, int blocksize, int assoc, enum cache_policy policy){
  // The cache is represented by a 2-D array of blocks.
  // The first dimension of the 2D array is "nsets" which is the number of sets (entries)
  // The second dimension is "assoc", which is the number of blocks in each set.

  int i;
  int nblocks = assoc;                      // number of blocks in the cache
  int nsets = size / (blocksize * assoc);   // number of sets (entries) in the cache

  //
  // YOUR JOB: calculate the number of sets and blocks in the cache
  //
  // nblocks = X;
  // nsets = Y;

  struct cache_t *C = (struct cache_t *)calloc(1, sizeof(struct cache_t));

  C->nsets = nsets;
  C->bsize = blocksize;
  C->assoc = assoc;
  C->policy = policy;

  C->blocks= (struct cache_blk_t **)calloc(nsets, sizeof(struct cache_blk_t));

  for(i = 0; i < nsets; i++) {
    C->blocks[i] = (struct cache_blk_t *)calloc(assoc, sizeof(struct cache_blk_t));
  }

  return C;
}

int
cache_access(struct cache_t *cp, unsigned long address,
             char access_type, unsigned long long now) {
  //////////////////////////////////////////////////////////////////////
  //
  // Your job:
  // based on address determine the set to access in cp
  // examine blocks in the set to check hit/miss
  // if miss, determine the victim in the set to replace
  // if update the block list based on the replacement policy
  // return 0 if a hit, 1 if a miss or 2 if a miss_with_write_back
  //
  //////////////////////////////////////////////////////////////////////
}

#endif

