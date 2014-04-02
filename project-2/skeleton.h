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
  int time;
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

struct cache_t * cache_create(int size, int blocksize, int assoc, enum cache_policy policy){
  // The cache is represented by a 2-D array of blocks.
  // The first dimension of the 2D array is "nsets" which is the number of sets (entries)
  // The second dimension is "assoc", which is the number of blocks in each set.

  int i;
  int nblocks; // number of blocks in the cache
  int nsets;   // number of sets (entries) in the cache

  //TODO verify that this is correct
  nsets = assoc;
  nblocks = size / (blocksize * assoc);

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

//TODO fix these calculations to be correct
long calc_index(long address, int blocksize){
  return address / blocksize;
}

//TODO fix these calculations to be correct
unsigned long calc_tag(unsigned long address, int blocksize){
  return address % blocksize;
}

int detect_hit(struct cache_t *cp, unsigned long req_tag, long req_index){
  int i;
  struct cache_blk_t temp;

  for(i = 0; i < cp->assoc; i++){
    if(cp->blocks[i][req_index]->tag == req_tag)
      return 1;
  }
  return 0;
}

// return 0 if a hit, 1 if a miss or 2 if a miss_with_write_back
int cache_access(struct cache_t *cp, unsigned long address,
             char access_type, unsigned long long now) {
  int hit;
  int i;
  long requested_index;
  unsigned long requested_tag;
  struct cache_blk_t temp;

  requested_index = calc_index(address, cp->bsize);
  requested_tag = calc_tag(address, cp->bsize);
  hit = 0;

  if(detect_hit(cp, requested_tag, requested_index) == 1){
    return 0;
  }
  else if(){
  }
  if(){
  }
}

#endif
