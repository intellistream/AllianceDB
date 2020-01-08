//
// Created by Shuhao Zhang on 1/11/19.
//


#include <vector>
#include <assert.h>
#include <zconf.h>
#include "avxsort.h"
#include "sort_common.h"
#include "localjoiner.h"
#include "pmj_helper.h"
#include "../joins/shj_struct.h"

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
SHJJoiner
shj(int32_t tid, relation_t *rel_R, relation_t *rel_S, void *pVoid) {

    SHJJoiner joiner(rel_R->num_tuples, rel_S->num_tuples);
    uint32_t index_R = 0;//index of rel_R
    uint32_t index_S = 0;//index of rel_S


#ifndef NO_TIMING
    START_MEASURE(joiner.timer)
#endif
    do {
        if (index_R < rel_R->num_tuples) {
            joiner.join(tid, &rel_R->tuples[index_R], true, &joiner.matches, NULL, pVoid);
            index_R++;
        }
        if (index_S < rel_S->num_tuples) {
            joiner.join(tid, &rel_S->tuples[index_S], false, &joiner.matches, NULL, pVoid);
            index_S++;
        }
    } while (index_R < rel_R->num_tuples || index_S < rel_S->num_tuples);

#ifndef NO_TIMING
    END_MEASURE(joiner.timer)
#endif

    return joiner;
}

/**
 * SHJ algorithm to be used in each thread.
 * @param tid
 * @param tuple
 * @param IStuple_R
 * @param htR
 * @param htS
 * @param matches
 * @param pVoid
 * @param timer
 * @return
 */
void SHJJoiner::join(int32_t tid, tuple_t *tuple, bool IStuple_R, int64_t *matches,
                     void *(*thread_fun)(const tuple_t *, const tuple_t *, int64_t *), void *pVoid) {

    const uint32_t hashmask_R = htR->hash_mask;
    const uint32_t skipbits_R = htR->skip_bits;
    const uint32_t hashmask_S = htS->hash_mask;
    const uint32_t skipbits_S = htS->skip_bits;

//    DEBUGMSG(1, "JOINING: tid: %d, tuple: %d, R?%d\n", tid, tuple->key, tuple_R)
    if (IStuple_R) {
        BEGIN_MEASURE_BUILD_ACC(timer)
        build_hashtable_single(htR, tuple, hashmask_R, skipbits_R);//(1)
        END_MEASURE_BUILD_ACC(timer)//accumulate hash table build time.

#ifdef DEBUG
        if (tid == 0) {
            window0.R_Window.push_back(tuple->key);
            print_window(window0.R_Window);
        } else {
            window1.R_Window.push_back(tuple->key);
            print_window(window1.R_Window);
        }
#endif
        proble_hashtable_single_measure(htS, tuple, hashmask_S, skipbits_S, matches, thread_fun, &timer);//(2)
    } else {
        BEGIN_MEASURE_BUILD_ACC(timer)
        build_hashtable_single(htS, tuple, hashmask_S, skipbits_S);//(3)
        END_MEASURE_BUILD_ACC(timer)//accumulate hash table build time.
#ifdef DEBUG
        if (tid == 0) {
            window0.S_Window.push_back(tuple->key);
            print_window(window0.S_Window);
        } else {
            window1.S_Window.push_back(tuple->key);
            print_window(window1.S_Window);
        }
#endif
        proble_hashtable_single_measure(htR, tuple, hashmask_R, skipbits_R, matches, thread_fun, &timer);//(4)
    }
}

/**
 * Clean state stored in local thread, basically used in HS mode
 * @param tid
 * @param tuple
 * @param htR
 * @param htS
 * @param cleanR
 */
void SHJJoiner::clean(int32_t tid, tuple_t *tuple, bool cleanR) {
    if (cleanR) {
        //if SHJ is used, we need to clean up hashtable of R.
        BEGIN_MEASURE_DEBUILD_ACC(timer)
        debuild_hashtable_single(htR, tuple, htR->hash_mask, htR->skip_bits);
        END_MEASURE_DEBUILD_ACC(timer)
#ifdef DEBUG
        if (tid == 0) {
            window0.R_Window.remove(tuple->key);
            print_window(window0.R_Window);
        } else {
            window1.R_Window.remove(tuple->key);
            print_window(window1.R_Window);
        }
#endif

    } else {
        BEGIN_MEASURE_DEBUILD_ACC(timer)
        debuild_hashtable_single(htS, tuple, htS->hash_mask, htS->skip_bits);
        END_MEASURE_DEBUILD_ACC(timer)
#ifdef DEBUG
        if (tid == 0) {
            window0.S_Window.remove(tuple->key);
            print_window(window0.S_Window);
        } else {
            window1.S_Window.remove(tuple->key);
            print_window(window1.S_Window);
        }
#endif
    }
}

SHJJoiner::SHJJoiner(int sizeR, int sizeS) {
    //allocate two hashtables.

    uint32_t nbucketsR = (sizeR / BUCKET_SIZE);
    allocate_hashtable(&htR, nbucketsR);

    uint32_t nbucketsS = (sizeS / BUCKET_SIZE);
    allocate_hashtable(&htS, nbucketsS);
}

SHJJoiner::~SHJJoiner() {
    destroy_hashtable(htR);
    destroy_hashtable(htS);
}

/**
 *  The main idea of PMJ is to read as much data as can fit in memory.
 *  Then, in-memory data is sorted and is joined together, and then is flushed into disk.
 *  When all data is received, PMJ joins the disk-resident data using a refinement version
 *  of the sort-merge join that allows producing join results while merging.
 *
 *  We change it to read up to progressive_step of data. Then sort and join, and then push aside at rest.
 *  When all data is received, join the rest data using refinement version of SMJ.
 *
 * @param tid
 * @param rel_R
 * @param rel_S
 * @param pVoid
 * @param timer
 * @return
 */
PMJJoiner
pmj(int32_t tid, relation_t *rel_R, relation_t *rel_S, void *pVoid) {

    PMJJoiner joiner(rel_R->num_tuples, rel_S->num_tuples, 1);
#ifndef NO_TIMING
    START_MEASURE(joiner.timer)
#endif
    //Phase 1 ('Join during run creation')
    int sizeR = rel_R->num_tuples;
    int sizeS = rel_S->num_tuples;
    int i = 0;
    int j = 0;

    int progressive_stepR = ALIGN_NUMTUPLES((int) (progressive_step * sizeR));//cacheline aligned.
    int progressive_stepS = ALIGN_NUMTUPLES((int) (progressive_step * sizeS));

    assert(progressive_stepR > 0 && progressive_stepS > 0);

    std::vector<run> Q;//let Q be an empty set;

    /***Initialize***/
    /**** allocate temporary space for sorting ****/
    size_t relRsz;
    tuple_t *outptrR;
    tuple_t *outptrS;

    relRsz = sizeR * sizeof(tuple_t)
             + RELATION_PADDING(1, CACHELINEPADDING(1));//TODO: think why we need to patch this.

    outptrR = (tuple_t *) malloc_aligned(relRsz);

    relRsz = sizeS * sizeof(tuple_t)
             + RELATION_PADDING(1, CACHELINEPADDING(1));//TODO: think why we need to patch this.

    outptrS = (tuple_t *) malloc_aligned(relRsz);

    /***Sorting***/
    do {
        sorting_phase(tid, rel_R, rel_S, sizeR, sizeS, progressive_stepR, progressive_stepS, &i, &j, &joiner.matches,
                      &Q, outptrR + i, outptrS + j, &joiner.timer);

    } while (i < sizeR - progressive_stepR && j < sizeS - progressive_stepS);//while R!=null, S!=null.

    /***Handling Left-Over***/
    progressive_stepR = sizeR - i;
    progressive_stepS = sizeS - j;
    sorting_phase(tid, rel_R, rel_S, sizeR, sizeS, progressive_stepR, progressive_stepS, &i, &j, &joiner.matches, &Q,
                  outptrR + i, outptrS + j, &joiner.timer);

    DEBUGMSG("Join during run creation:%d", joiner.matches)

    merging_phase(&joiner.matches, &Q, &joiner.timer);

    DEBUGMSG("Join during run merge matches:%d", joiner.matches)
#ifndef NO_TIMING
    END_MEASURE(joiner.timer)
#endif
    return joiner;
}

void PMJJoiner::keep_tuple_single(tuple_t *tmp_rel, const int outerPtr,
                                  tuple_t *tuple, int fat_tuple_size) {
    for (auto i = 0; i < fat_tuple_size; i++) {
        tmp_rel[outerPtr + i] = tuple[i];//deep copy tuples.
//        tmp_rel[outerPtr + i].key = -1;//to check if it's deep copy.
//        DEBUGMSG("%d, %d", std::addressof( tmp_rel[outerPtr + i]), std::addressof(tuple[i]))
    }
}

void
PMJJoiner::join_tuple_single(int32_t tid, tuple_t *tmp_rel, int *outerPtr, tuple_t *tuple, int fat_tuple_size,
                             int64_t *matches, T_TIMER *timer, t_pmj *pPmj, bool IStuple_R) {

    if (*outerPtr > 0 && fat_tuple_size > 0) {
        tuple_t *out_relR;
        tuple_t *out_relS;
        if (IStuple_R) {
            auto relRsz = fat_tuple_size * sizeof(tuple_t)
                          + RELATION_PADDING(1, CACHELINEPADDING(1));//TODO: think why we need to patch this.
            out_relR = (tuple_t *) malloc_aligned(relRsz);

            relRsz = *outerPtr * sizeof(tuple_t)
                     + RELATION_PADDING(1, CACHELINEPADDING(1));//TODO: think why we need to patch this.
            out_relS = (tuple_t *) malloc_aligned(relRsz);

            sorting_phase(tid,
                          tuple,
                          fat_tuple_size,
                          tmp_rel,
                          *outerPtr,
                          matches,
                          &pPmj->Q,
                          out_relR,
                          out_relS,
                          timer);

            pPmj->extraPtrR += fat_tuple_size;
            pPmj->extraPtrS += *outerPtr;

        } else {
            auto relRsz = fat_tuple_size * sizeof(tuple_t)
                          + RELATION_PADDING(1, CACHELINEPADDING(1));//TODO: think why we need to patch this.
            out_relS = (tuple_t *) malloc_aligned(relRsz);

            relRsz = *outerPtr * sizeof(tuple_t)
                     + RELATION_PADDING(1, CACHELINEPADDING(1));//TODO: think why we need to patch this.
            out_relR = (tuple_t *) malloc_aligned(relRsz);

            sorting_phase(tid,
                          tmp_rel,
                          *outerPtr,
                          tuple,
                          fat_tuple_size,
                          matches,
                          &pPmj->Q,
                          out_relR,
                          out_relS,
                          timer);
            pPmj->extraPtrR += *outerPtr;
            pPmj->extraPtrS += fat_tuple_size;
        }
    }
}

/**
 * PMJ algorithm to be used in each thread.
 * First store enough tuples from R and S, then call PMJ algorithm.
 * @param tid
 * @param tuple
 * @param IStuple_R
 * @param htR
 * @param htS
 * @param matches
 * @param pVoid
 * @param timer
 * @return
 */
void PMJJoiner:: //0x7fff08000c70
join(int32_t tid, tuple_t *tuple, int fat_tuple_size, bool IStuple_R, int64_t *matches,
     void *(*thread_fun)(const tuple_t *, const tuple_t *, int64_t *), void *pVoid) {
    auto *arg = (t_pmj *) t_arg;

    //store tuples.
    if (IStuple_R) {
        DEBUGMSG("TID %d: before store R is %s.", tid,
                 print_tuples(arg->tmp_relR, arg->outerPtrR).c_str())
        BEGIN_MEASURE_BUILD_ACC(timer)
        keep_tuple_single(arg->tmp_relR, arg->outerPtrR, tuple, fat_tuple_size);
        arg->outerPtrR += fat_tuple_size;
        END_MEASURE_BUILD_ACC(timer)//accumulate hash table build time.
        DEBUGMSG("TID %d: after store R is %s.", tid,
                 print_tuples(arg->tmp_relR, arg->outerPtrR).c_str())
#ifdef DEBUG
        for (auto i = 0; i < fat_tuple_size; i++) {
            if (tid == 0) {
                window0.R_Window.push_back(tuple[i].key);
            } else if (tid == 1) {
                window1.R_Window.push_back(tuple[i].key);
            }

        }
#endif
        DEBUGMSG("TID: %d Sorting in normal stage start", tid)

        auto inputS = copy_tuples(arg->tmp_relS, arg->outerPtrS);

        join_tuple_single(tid, inputS, &arg->outerPtrS,
                          tuple, fat_tuple_size, matches, &timer, arg,
                          IStuple_R);
        DEBUGMSG("TID: %d Join during run creation:%d", tid, *matches)

    } else {
        DEBUGMSG("TID %d: before store S is %s.", tid,
                 print_tuples(arg->tmp_relS, arg->outerPtrS).c_str())
        BEGIN_MEASURE_BUILD_ACC(timer)
        keep_tuple_single(arg->tmp_relS, arg->outerPtrS, tuple, fat_tuple_size);
        arg->outerPtrS += fat_tuple_size;
        END_MEASURE_BUILD_ACC(timer)//accumulate hash table build time.
        DEBUGMSG("TID %d: after store S is %s.", tid,
                 print_tuples(arg->tmp_relS, arg->outerPtrS).c_str())
#ifdef DEBUG
        for (auto i = 0; i < fat_tuple_size; i++) {
            if (tid == 0) {
                window0.S_Window.push_back(tuple[i].key);
            } else if (tid == 1) {
                window1.S_Window.push_back(tuple[i].key);
            }

        }
#endif
        DEBUGMSG("TID: %d Sorting in normal stage start", tid)
        auto inputR = copy_tuples(arg->tmp_relR, arg->outerPtrR);
        join_tuple_single(tid, inputR, &arg->outerPtrR, tuple, fat_tuple_size,
                          matches, &timer, arg, IStuple_R);
        DEBUGMSG("TID: %d Join during run creation:%d", tid, *matches)

    }

}

void PMJJoiner::clean(int32_t tid, tuple_t *tuple, bool cleanR) {
    std::cout << "this method should not be called" << std::endl;
//    if (cleanR) {
//        auto idx = find_index(this->t_arg->tmp_relR, this->t_arg->outerPtrR + this->t_arg->innerPtrR, tuple);
//        this->t_arg->tmp_relR[idx] = this->t_arg->tmp_relR[this->t_arg->outerPtrR + this->t_arg->innerPtrR - 1];
//        this->t_arg->innerPtrR--;
//        if (this->t_arg->innerPtrR < 0) {
//            this->t_arg->outerPtrR -= progressive_step_tupleR;
//            this->t_arg->innerPtrR = progressive_step_tupleR - 1;
//        }
//
//#ifdef DEBUG
//        if (tid == 0) {
//            window0.R_Window.remove(tuple->key);
//            DEBUGMSG("T0 after remove R (expected): %s, actual: %s", print_window(window0.R_Window).c_str(),
//                     print_tuples(this->t_arg->tmp_relR, this->t_arg->outerPtrR + this->t_arg->innerPtrR).c_str())
//        } else {
//            window1.R_Window.remove(tuple->key);
//            DEBUGMSG("T1 after remove R (expected): %s, actual: %s", print_window(window1.R_Window).c_str(),
//                     print_tuples(this->t_arg->tmp_relR, this->t_arg->outerPtrR + this->t_arg->innerPtrR).c_str())
//        }
//#endif
//
//    } else {
//        auto idx = find_index(this->t_arg->tmp_relS, this->t_arg->outerPtrS + this->t_arg->innerPtrS, tuple);
//        this->t_arg->tmp_relS[idx] = this->t_arg->tmp_relS[this->t_arg->outerPtrS + this->t_arg->innerPtrS - 1];
//        this->t_arg->innerPtrS--;
//        if (this->t_arg->innerPtrS < 0) {
//            this->t_arg->outerPtrS -= progressive_step_tupleS;
//            this->t_arg->innerPtrS = progressive_step_tupleS - 1;
//        }
//
//#ifdef DEBUG
//        if (tid == 0) {
//            window0.S_Window.remove(tuple->key);
//            DEBUGMSG("T0 after remove S (expected): %s,(actual): %s", print_window(window0.S_Window).c_str(),
//                     print_tuples(this->t_arg->tmp_relS, this->t_arg->outerPtrS + this->t_arg->innerPtrS).c_str())
//        } else {
//            window1.S_Window.remove(tuple->key);
//            DEBUGMSG("T1 after remove S (expected): %s,(actual): %s", print_window(window1.S_Window).c_str(),
//                     print_tuples(this->t_arg->tmp_relS, this->t_arg->outerPtrS + this->t_arg->innerPtrS).c_str())
//        }
//#endif
//    }
}

/**
 * HS cleaner
 * @param tid
 * @param tuple
 * @param htR
 * @param htS
 * @param cleanR
 */
void PMJJoiner::
clean(int32_t tid, tuple_t *fat_tuple, int fat_tuple_size, bool cleanR) {

    if (cleanR) {

        DEBUGMSG("Start clean %d; TID:%d, Initial R: %s", fat_tuple[0].key, tid,
                 print_relation(this->t_arg->tmp_relR, this->t_arg->outerPtrR).c_str())

        for (auto i = 0; i < fat_tuple_size; i++) {
/*
                int idx = find_index(this->t_arg->tmp_relR,
                                     this->t_arg->outerPtrR,
                                     tuple[i]);
                if (idx == -1) {
                     DEBUGMSG(
                            "Clean %d not find correct position. something is wrong. TID:%d, Initial R: %s",
                            tuple[i]->key, tid, print_relation(this->t_arg->tmp_relR, this->t_arg->outerPtrR).c_str())
                    break;//something is wrong.
                }

                this->t_arg->tmp_relR[idx] = this->t_arg->tmp_relR[
                        this->t_arg->outerPtrR - 1];
//                remove++;
                this->t_arg->outerPtrR--;
*/
#ifdef DEBUG
            if (tid == 0) {
                window0.R_Window.remove(fat_tuple[i].key);
                DEBUGMSG("T0 removes R %d, left with:%s", fat_tuple[i].key,
                         print_window(window0.R_Window).c_str())
            } else if (tid == 1) {
                window1.R_Window.remove(fat_tuple[i].key);
                DEBUGMSG("T1 removes R %d, left with:%s", fat_tuple[i].key,
                         print_window(window1.R_Window).c_str())
            }
#endif

        }
        this->t_arg->tmp_relR = &this->t_arg->tmp_relR[fat_tuple_size];
        this->t_arg->outerPtrR -= fat_tuple_size;
        DEBUGMSG("Stop clean %d; T%d left with:%s", fat_tuple[0].key, tid,
                 print_relation(this->t_arg->tmp_relR, this->t_arg->outerPtrR).c_str())
//        this->t_arg->outerPtrR -= remove;
//        if (this->t_arg->innerPtrR < 0) {
//            this->t_arg->outerPtrR += this->t_arg->innerPtrR;
//            this->t_arg->innerPtrR = 0;
//        }


#ifdef DEBUG
        if (tid == 0) {
            DEBUGMSG("T0 after remove R (expected): %s, actual: %s", print_window(window0.R_Window).c_str(),
                     print_tuples(this->t_arg->tmp_relR,
                                  this->t_arg->outerPtrR + this->t_arg->innerPtrR).c_str())
        } else if (tid == 1) {
            DEBUGMSG("T1 after remove R (expected): %s, actual: %s, size: %d", print_window(window1.R_Window).c_str(),
                     print_tuples(this->t_arg->tmp_relR,
                                  this->t_arg->outerPtrR + this->t_arg->innerPtrR).c_str(), this->t_arg->outerPtrR)
        }
#endif


    } else {

        for (auto i = 0; i < fat_tuple_size; i++) {

            int idx = find_index(this->t_arg->tmp_relS, this->t_arg->outerPtrS, &fat_tuple[i]);
            if (idx == -1) {
                break;//it must be an ack signal, and does not stored in this thread.
            }


            this->t_arg->tmp_relS[idx] = this->t_arg->tmp_relS[this->t_arg->outerPtrS - 1];
//                remove++;
            this->t_arg->outerPtrS--;

#ifdef DEBUG
            if (tid == 0) {
                window0.S_Window.remove(fat_tuple[i].key);
                DEBUGMSG("T0 removes S %d, left with:%s", fat_tuple[i].key,
                         print_window(window0.S_Window).c_str());
            } else if (tid == 1) {
                window1.S_Window.remove(fat_tuple[i].key);
                DEBUGMSG("T1 removes S %d, left with:%s", fat_tuple[i].key,
                         print_window(window1.S_Window).c_str())
            }
#endif


        }

//        this->t_arg->tmp_relS = &this->t_arg->tmp_relS[remove];
//        this->t_arg->outerPtrS -= remove;
        DEBUGMSG("Stop clean %d; T%d left with:%s", fat_tuple[0].key, tid,
                 print_relation(this->t_arg->tmp_relS, this->t_arg->outerPtrS).c_str())

#ifdef DEBUG
        if (tid == 0) {
            DEBUGMSG("T0 after remove S (expected): %s,(actual): %s", print_window(window0.S_Window).c_str(),
                     print_tuples(this->t_arg->tmp_relS,
                                  this->t_arg->outerPtrS).c_str())
        } else if (tid == 1) {
            DEBUGMSG("T1 after remove S (expected): %s,(actual): %s, size: %d", print_window(window1.S_Window).c_str(),
                     print_tuples(this->t_arg->tmp_relS,
                                  this->t_arg->outerPtrS).c_str(), this->t_arg->outerPtrS)
        }
#endif
    }
}


long PMJJoiner::
merge(int32_t tid, int64_t *matches,
      void *(*thread_fun)(const tuple_t *, const tuple_t *, int64_t *), void *pVoid) {
    auto *arg = (t_pmj *) t_arg;
    int stepR;
    int stepS;
    /***Handling Left-Over***/
    DEBUGMSG("TID:%d in Clean up stage, current matches:", tid, *matches)
    stepR = arg->innerPtrR;
    stepS = arg->innerPtrS;


    tuple_t *out_relR;
    tuple_t *out_relS;

    auto relRsz = stepR * sizeof(tuple_t)
                  + RELATION_PADDING(1, CACHELINEPADDING(1));//TODO: think why we need to patch this.
    out_relR = (tuple_t *) malloc_aligned(relRsz);

    relRsz = stepS * sizeof(tuple_t)
             + RELATION_PADDING(1, CACHELINEPADDING(1));//TODO: think why we need to patch this.
    out_relS = (tuple_t *) malloc_aligned(relRsz);


    sorting_phase(tid, arg->tmp_relR + arg->outerPtrR, stepR,
                  arg->tmp_relS + arg->outerPtrS, stepS,
                  matches, &arg->Q,
                  out_relR,
                  out_relS,
//                  arg->out_relR + arg->outerPtrR,
//                  arg->out_relS + arg->outerPtrS,
                  &timer);
    DEBUGMSG("TID:%d Clean up stage: Join during run creation:%d, arg->Q %d", tid, *matches, arg->Q.size())

    merging_phase(matches, &arg->Q, &timer);
    DEBUGMSG("TID:%d Clean up stage: Join during run merge matches:%d", tid, *matches)
    return *matches;
}


PMJJoiner::PMJJoiner(int
                     sizeR, int
                     sizeS, int
                     nthreads) {
    t_arg = new t_pmj(sizeR, sizeS);
}

void PMJJoiner::join(int32_t tid, tuple_t *tuple, bool IStuple_R, int64_t *matches,
                     void *(*thread_fun)(const tuple_t *, const tuple_t *, int64_t *), void *pVoid) {
    auto *arg = (t_pmj *) t_arg;

    //store tuples.
    if (IStuple_R) {
        arg->tmp_relR[arg->outerPtrR + arg->innerPtrR] = *tuple;
        arg->innerPtrR++;
    } else {
        arg->tmp_relS[arg->outerPtrS + arg->innerPtrS] = *tuple;
        arg->innerPtrS++;
    }
    int stepR = progressive_step_tupleR;
    int stepS = progressive_step_tupleS;

    tuple_t *out_relR;
    tuple_t *out_relS;

    if (arg->outerPtrR < arg->sizeR - stepR && arg->outerPtrS < arg->sizeS - stepS) {//normal process
        //check if it is ready to start process.
        if (arg->innerPtrR >= stepR
            && arg->innerPtrS >= stepS) {//start process and reset inner pointer.
            auto relRsz = stepR * sizeof(tuple_t)
                          + RELATION_PADDING(1, CACHELINEPADDING(1));//TODO: think why we need to patch this.
            out_relR = (tuple_t *) malloc_aligned(relRsz);

            relRsz = stepS * sizeof(tuple_t)
                     + RELATION_PADDING(1, CACHELINEPADDING(1));//TODO: think why we need to patch this.
            out_relS = (tuple_t *) malloc_aligned(relRsz);


            /***Sorting***/
            sorting_phase(tid, arg->tmp_relR + arg->outerPtrR, stepR, arg->tmp_relS + arg->outerPtrS, stepS,
                          matches, &arg->Q,
                          out_relR,
                          out_relS,
//                          arg->out_relR + arg->outerPtrR,
//                          arg->out_relS + arg->outerPtrS,
                          &timer);
            arg->outerPtrR += stepR;
            arg->outerPtrS += stepS;
            DEBUGMSG("Join during run creation:%d", *matches)

            /***Reset Inner Pointer***/
            arg->innerPtrR -= stepR;
            arg->innerPtrS -= stepS;

            delete out_relR;
            delete out_relS;
        }
    } else if (arg->outerPtrR + arg->innerPtrR == arg->sizeR &&
               arg->outerPtrS + arg->innerPtrS == arg->sizeS) {//received everything

        /***Handling Left-Over***/
        stepR = arg->sizeR - arg->outerPtrR;
        stepS = arg->sizeS - arg->outerPtrS;

        auto relRsz = stepR * sizeof(tuple_t)
                      + RELATION_PADDING(1, CACHELINEPADDING(1));//TODO: think why we need to patch this.
        out_relR = (tuple_t *) malloc_aligned(relRsz);

        relRsz = stepS * sizeof(tuple_t)
                 + RELATION_PADDING(1, CACHELINEPADDING(1));//TODO: think why we need to patch this.
        out_relS = (tuple_t *) malloc_aligned(relRsz);


        sorting_phase(tid, arg->tmp_relR + arg->outerPtrR, stepR, arg->tmp_relS + arg->outerPtrS, stepS,
                      matches, &arg->Q,
//                      arg->out_relR + arg->outerPtrR,
//                      arg->out_relS + arg->outerPtrS,
                      out_relR,
                      out_relS,
                      &timer);
        DEBUGMSG("Join during run creation:%d", *matches)
        merging_phase(matches, &arg->Q, &timer);
        DEBUGMSG("Join during run merge matches:%d", *matches)

        delete out_relR;
        delete out_relS;
    }




}


/**
 *
 *
 * @param relR
 * @param relS
 * @param nthreads
 * @return
 */
RippleJoiner
rpj(int32_t tid, relation_t *rel_R, relation_t *rel_S, void *pVoid) {

    RippleJoiner joiner(rel_R, rel_S, 1);

    uint32_t index_R = 0;//index of rel_R
    uint32_t index_S = 0;//index of rel_S

    uint32_t cur_step = 0;


#ifndef NO_TIMING
    START_MEASURE(joiner.timer)
#endif

    // just a simple nested loop with progressive response, R and S have the same input rate
    do {
        if (index_R < rel_R->num_tuples) {
            joiner.join(tid, &rel_R->tuples[index_R], true, &joiner.matches, NULL, pVoid);
            index_R++;
        }
        if (index_S < rel_S->num_tuples) {
            joiner.join(tid, &rel_S->tuples[index_S], false, &joiner.matches, NULL, pVoid);
            index_S++;
        }
    } while (index_R < rel_R->num_tuples || index_S < rel_S->num_tuples);
#ifndef NO_TIMING
    END_MEASURE(joiner.timer)
#endif
    return joiner;
}

/**
 *
 * TODO:一个是升级成wanderjoin（就是加索引）一个是要用estimation结果来指导fetch.
 *
 * RIPPLE JOIN algorithm to be used in each thread.
 * @param tid
 * @param tuple
 * @param tuple_R
 * @param htR
 * @param htS
 * @param matches
 * @param pVoid
 * @param timer
 * @return
 */

void RippleJoiner::join(int32_t tid, tuple_t *tuple, bool tuple_R, int64_t *matches,
                        void *(*thread_fun)(const tuple_t *, const tuple_t *, int64_t *), void *pVoid) {
    DEBUGMSG("tid: %d, tuple: %d, R?%d\n", tid, tuple->key, tuple_R);
    if (tuple_R) {
        BEGIN_MEASURE_BUILD_ACC(timer)
        samList.t_windows->R_Window.push_back(tuple);
        END_MEASURE_BUILD_ACC(timer)//accumulate hash table build time.

        match_single_tuple(samList.t_windows->S_Window, tuple, matches, thread_fun, &timer);
    } else {
//        samList.t_windows->S_Window.push_back(tuple->key);
        BEGIN_MEASURE_BUILD_ACC(timer)
        samList.t_windows->S_Window.push_back(tuple);
        END_MEASURE_BUILD_ACC(timer)//accumulate hash table build time.

        match_single_tuple(samList.t_windows->R_Window, tuple, matches, thread_fun, &timer);
    }
    // Compute estimation result
    long estimation_result = 0;
    if (samList.t_windows->R_Window.size() > 0 && samList.t_windows->S_Window.size() > 0) {
        estimation_result =
                ((int) relR->num_tuples * (int) relS->num_tuples)
                /
                ((int) samList.t_windows->R_Window.size() * (int) samList.t_windows->S_Window.size())
                *
                (int) (*matches);
    } else {
        estimation_result = *matches;
    }
//    fprintf(stdout, "estimation result: %d \n", estimation_result);
}

/**
 *
 * @param relR
 * @param relS
 * @param nthreads
 */
RippleJoiner::RippleJoiner(relation_t *relR, relation_t *relS, int
nthreads) : relR(
        relR), relS(relS) {
    samList.num_threads = nthreads;
    samList.t_windows = new t_window();
}


/**
 *
 * @param tid
 * @param tuple
 * @param htR
 * @param htS
 * @param cleanR
 */
void RippleJoiner::clean(int32_t tid, tuple_t *tuple, bool cleanR) {
    if (cleanR) {
        samList.t_windows->R_Window.remove(tuple);
    } else {
        samList.t_windows->S_Window.remove(tuple);
    }
}


