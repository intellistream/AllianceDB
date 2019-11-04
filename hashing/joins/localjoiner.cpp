//
// Created by Shuhao Zhang on 1/11/19.
//

#include "localjoiner.h"


long _shj(int32_t tid, tuple_t *tuple, bool tuple_R, hashtable_t *htR, hashtable_t *htS, int64_t *matches, void *pVoid,
          T_TIMER *timer) {

    const uint32_t hashmask_R = htR->hash_mask;
    const uint32_t skipbits_R = htR->skip_bits;

    const uint32_t hashmask_S = htS->hash_mask;
    const uint32_t skipbits_S = htS->skip_bits;

    DEBUGMSG("TID: %d, tuple: %d, R?%d\n", tid, tuple->key, tuple_R);
    if (tuple_R) {
        if (tid == 0) {
            BEGIN_MEASURE_BUILD_ACC((*timer))
        }
        build_hashtable_single(htR, tuple, hashmask_R, skipbits_R);//(1)
        if (tid == 0) {
            END_MEASURE_BUILD_ACC((*timer))
        }
        if (tid == 0) {
            proble_hashtable_single_measure(htS, tuple, hashmask_S, skipbits_S, matches, timer->progressivetimer);//(2)
        } else {
            proble_hashtable_single(htS, tuple, hashmask_S, skipbits_S, matches);//(4)
        }
    } else {
        if (tid == 0) {
            BEGIN_MEASURE_BUILD_ACC((*timer))
        }
        build_hashtable_single(htS, tuple, hashmask_S, skipbits_S);//(3)
        if (tid == 0) {
            END_MEASURE_BUILD_ACC((*timer))
        }
        if (tid == 0) {
            proble_hashtable_single_measure(htR, tuple, hashmask_R, skipbits_R, matches, timer->progressivetimer);//(4)
        } else {
            proble_hashtable_single(htR, tuple, hashmask_R, skipbits_R, matches);//(4)
        }
    }
    return *matches;
}

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
shj_JB(int32_t tid, hashtable_t *ht_R, hashtable_t *ht_S, relation_t *rel_R, relation_t *rel_S,
       std::list<int> list_r, std::list<int> list_s, void *pVoid,
       T_TIMER *timer) {

    const uint32_t hashmask_R = ht_R->hash_mask;
    const uint32_t skipbits_R = ht_R->skip_bits;

    const uint32_t hashmask_S = ht_S->hash_mask;
    const uint32_t skipbits_S = ht_S->skip_bits;

    int64_t matches = 0;//number of matches.

    std::list<int>::const_iterator iterator;
    for (iterator = list_r.begin(); iterator != list_r.end(); ++iterator) {
//        std::cout << *iterator;

        if (tid == 0) {
            BEGIN_MEASURE_BUILD_ACC((*timer))
        }
        build_hashtable_single(ht_R, rel_R, *iterator, hashmask_R, skipbits_R);//(1)
        if (tid == 0) {
            END_MEASURE_BUILD_ACC((*timer))
        }
        if (tid == 0) {
            proble_hashtable_single_measure(ht_S, rel_R, *iterator, hashmask_S, skipbits_S, &matches,
                                            timer->progressivetimer);//(2)
        } else {
            proble_hashtable_single(ht_S, rel_R, *iterator, hashmask_S, skipbits_S, &matches);//(4)
        }

    }

    for (iterator = list_s.begin(); iterator != list_s.end(); ++iterator) {
//        std::cout << *iterator;

        if (tid == 0) {
            BEGIN_MEASURE_BUILD_ACC((*timer))
        }
        build_hashtable_single(ht_S, rel_S, *iterator, hashmask_S, skipbits_S);//(3)
        if (tid == 0) {
            END_MEASURE_BUILD_ACC((*timer))
        }
        if (tid == 0) {
            proble_hashtable_single_measure(ht_R, rel_S, *iterator, hashmask_R, skipbits_R, &matches,
                                            timer->progressivetimer);//(4)
        } else {
            proble_hashtable_single(ht_R, rel_S, *iterator, hashmask_R, skipbits_R, &matches);//(4)
        }
    }

    return matches;
}

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
shj(int32_t tid, relation_t *rel_R,
    relation_t *rel_S, void *pVoid,
    T_TIMER *timer) {

    //allocate two hashtables.
    hashtable_t *htR;
    hashtable_t *htS;

    uint32_t nbucketsR = (rel_R->num_tuples / BUCKET_SIZE);
    allocate_hashtable(&htR, nbucketsR);

    uint32_t nbucketsS = (rel_S->num_tuples / BUCKET_SIZE);
    allocate_hashtable(&htS, nbucketsS);

    uint32_t index_R = 0;//index of rel_R
    uint32_t index_S = 0;//index of rel_S

    int64_t matches = 0;//number of matches.

    do {
        if (index_R < rel_R->num_tuples) {
            _shj(tid, &rel_R->tuples[index_R], true, htR, htS, &matches, pVoid, timer);
            index_R++;
        }
        if (index_S < rel_S->num_tuples) {
            _shj(tid, &rel_S->tuples[index_S], false, htR, htS, &matches, pVoid, timer);
            index_S++;
        }
    } while (index_R < rel_R->num_tuples || index_S < rel_S->num_tuples);

    destroy_hashtable(htR);
    destroy_hashtable(htS);
    return matches;
}
