//
// Created by tony on 04/03/22.
//
#include <JoinAlgos/NPJ/MultiThreadHashTable.h>

#ifndef HASH
#define HASH(X, MASK, SKIP) (((X) & MASK) >> SKIP)
#endif

#ifndef NEXT_POW_2
/**
 *  compute the next number, greater than or equal to 32-bit unsigned v.
 *  taken from "bit twiddling hacks":
 *  http://graphics.stanford.edu/~seander/bithacks.html
 */
#define NEXT_POW_2(V)                           \
    do {                                        \
        V--;                                    \
        V |= V >> 1;                            \
        V |= V >> 2;                            \
        V |= V >> 4;                            \
        V |= V >> 8;                            \
        V |= V >> 16;                           \
        V++;                                    \
    } while(0)
#endif
using namespace OoOJoin;
using namespace std;

void MtBucket::duplicatedInsert(NPJTuplePtr tp) {
  MtBucketPtr nxt;
  nxt = this->next;
  if (this->count == BUCKET_SIZE) {
    if (nxt == nullptr || nxt->count == BUCKET_SIZE) {
      MtBucketPtr b = make_shared<MtBucket>();
      b->next = this->next;
      b->count = 1;
      b->tuples[0] = tp;
      this->next = b;
    } else {
      nxt->tuples[nxt->count] = tp;
      nxt->count++;
    }
  } else {
    this->tuples[this->count] = tp;
    this->count++;
  }

}

size_t MtBucket::probeTuple(NPJTuplePtr tp) {
// size_t allLen=this->count;
  size_t matches = 0;
  MtBucket *b = this;
  while (1) {
    for (size_t index_ht = 0; index_ht < b->count; index_ht++) {
      if (tp->key == b->tuples[index_ht]->key) {
        matches++;
      }
    }
    if (b->next == nullptr) {
      return matches;
    }
    b = b->next.get();
  }
  return 0;
}

//init with buckets
MultiThreadHashTable::MultiThreadHashTable(size_t bksr) {
  size_t bks = bksr;
  NEXT_POW_2(bks);
  //creat the buckets
  buckets = vector<MtBucket>(bks);

  this->skip_bits = 0; /* the default for modulo hash */
  this->hash_mask = (bks - 1) << this->skip_bits;
}

void MultiThreadHashTable::buildTable(NPJTuplePtr *tps, size_t len) {
  size_t i;
  const uint32_t hashmask = this->hash_mask;
  const uint32_t skipbits = this->skip_bits;
  size_t allLen = len;
  for (i = 0; i < allLen; i++) {
    NPJTuplePtr nowTuple;
    MtBucket *curr;
    nowTuple = tps[i]; //read [i]
    int32_t idx = HASH(nowTuple->key, hashmask, skipbits); //get the hash value
    curr = &buckets[idx];
    curr->lock();
    curr->duplicatedInsert(nowTuple);
    curr->unlock();
  }
}

size_t MultiThreadHashTable::probeTuple(NPJTuplePtr tp) {
  const uint32_t hashmask = this->hash_mask;
  const uint32_t skipbits = this->skip_bits;
  int32_t idx = HASH(tp->key, hashmask, skipbits);
  return buckets[idx].probeTuple(tp);

}