//
// Created by tony on 02/12/22.
//

#include <Common/StateOfKey.h>
#include <iostream>

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

void StateOfKeyBucket::insert(AbstractStateOfKeyPtr ask) {
  StateOfKeyBucketPtr nxt;
  nxt = this->next;
  if (this->count == bucketSize) {

    if (nxt == nullptr || nxt->count == bucketSize) {
      // std::cout<<"spawning new bucket for key"+ to_string(ask->key)+", count="+ to_string(this->count)+"\n";
      StateOfKeyBucketPtr b = make_shared<StateOfKeyBucket>();
      b->next = this->next;
      b->count = 1;
      b->setCellCount(bucketSize);
      b->cells[0] = ask;
      this->next = b;
    } else {
      nxt->cells[nxt->count] = ask;
      nxt->count++;
    }
  } else {
    this->cells[this->count] = ask;
    this->count++;

  }

}

AbstractStateOfKeyPtr StateOfKeyBucket::getByKey(keyType probeKey) {
  StateOfKeyBucket *b = this;
  while (1) {
    for (size_t index_ht = 0; index_ht < b->count; index_ht++) {
      if (probeKey == b->cells[index_ht]->key) {
        // matches++;
        return b->cells[index_ht];
      }
    }
    if (b->next == nullptr) {
      return nullptr;
    }
    b = b->next.get();
  }
  return nullptr;
}

//init with buckets
StateOfKeyHashTable::StateOfKeyHashTable(size_t bksr, size_t cells) {
  size_t bks = bksr;
  //NEXT_POW_2(bks);
  //creat the buckets
  buckets = vector<StateOfKeyBucket>(bks);
  for (size_t i = 0; i < bks; i++) {
    buckets[i].setCellCount(cells);
  }
  this->skip_bits = 0; /* the default for modulo hash */
  this->hash_mask = (bks - 1) << this->skip_bits;
}

void StateOfKeyHashTable::insertSafe(AbstractStateOfKeyPtr ask) {
  // size_t i;
  const uint32_t hashmask = this->hash_mask;
  const uint32_t skipbits = this->skip_bits;
  keyType idx = HASH(ask->key, hashmask, skipbits);
  StateOfKeyBucket *curr = &buckets[idx];
  curr->lock();
  curr->insert(ask);
  curr->unlock();
  // size_t allLen = len;
}

void StateOfKeyHashTable::insert(AbstractStateOfKeyPtr ask) {
  // size_t i;
  const uint32_t hashmask = this->hash_mask;
  const uint32_t skipbits = this->skip_bits;
  keyType idx = HASH(ask->key, hashmask, skipbits);
  StateOfKeyBucket *curr = &buckets[idx];
  curr->insert(ask);
}

AbstractStateOfKeyPtr StateOfKeyHashTable::getByKey(keyType probeKey) {
  const uint32_t hashmask = this->hash_mask;
  const uint32_t skipbits = this->skip_bits;
  int32_t idx = HASH(probeKey, hashmask, skipbits);
  return buckets[idx].getByKey(probeKey);
}