//
// Created by Shuhao Zhang on 1/11/19.
//

#ifndef ALLIANCEDB_LOCALJOINER_H
#define ALLIANCEDB_LOCALJOINER_H

#include "../utils/xxhash64.h"
#include "../utils/t_timer.h"  /* startTimer, stopTimer */
#include "../utils/generator.h"          /* numa_localize() */
#include "npj_types.h"          /* bucket_t, hashtable_t, bucket_buffer_t */
#include "npj_params.h"         /* constant parameters */
#include "common_functions.h"
#include <list>


/**
 * As an example of join execution, consider a join with join predicate T1.attr1 = T2.attr2.
 * The join operator will incrementally load a hash table H1 for T1 by hashing attr1 using hash function f1,
 * and another hash table H2 for T2 by hashing attr2 using hash function f2.
 * The symmetric hash join operator starts by
 * (1): getting a tuple from T1, hashing its attr1 field using f1, and inserting it into H1.
 * (2): it probes H2 by applying f2 to attr1 of the current T1 tuple, returning any matched tuple pairs.
 *
 * (3): it gets a tuple from T2, hashes it by applying f2 to attr2, and inserts it into H2.
 * (4): it probes H1 by applying f1 to attr2 of the current T2 tuple, and returns any matches.
 * (5): This continues until all tuples from T1 and T2 have been consumed.
 *
 * @param relR
 * @param relS
 * @param nthreads
 * @return
 */
long
_shj(int32_t tid, tuple_t *tuple,
     bool tuple_R, hashtable_t *htR, hashtable_t *htS, int64_t *matches, void *pVoid, T_TIMER *timer);

long
shj(int32_t tid, relation_t *rel_R, relation_t *rel_S, void *pVoid, T_TIMER *timer);

long
shj_JB(int32_t tid, hashtable_t *ht_R, hashtable_t *ht_S, relation_t *rel_R, relation_t *rel_S, std::list<int> list_r,
       std::list<int> list_s, void *pVoid, T_TIMER *timer);

#endif //ALLIANCEDB_LOCALJOINER_H
