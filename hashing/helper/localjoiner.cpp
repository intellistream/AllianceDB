//
// Created by Shuhao Zhang on 1/11/19.
//


#include <vector>
#include <assert.h>
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
 * @param tuple_R
 * @param htR
 * @param htS
 * @param matches
 * @param pVoid
 * @param timer
 * @return
 */
void SHJJoiner::join(int32_t tid, tuple_t *tuple, bool tuple_R, int64_t *matches,
                     void *(*thread_fun)(const tuple_t *, const tuple_t *, int64_t *), void *pVoid) {

    const uint32_t hashmask_R = htR->hash_mask;
    const uint32_t skipbits_R = htR->skip_bits;
    const uint32_t hashmask_S = htS->hash_mask;
    const uint32_t skipbits_S = htS->skip_bits;

//    DEBUGMSG(1, "JOINING: tid: %d, tuple: %d, R?%d\n", tid, tuple->key, tuple_R)
    if (tuple_R) {
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
//    timer.numS++;//one process.
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

    DEBUGMSG("Join during run creation:%d", matches)

    merging_phase(&joiner.matches, &Q, &joiner.timer);

    DEBUGMSG("Join during run merge matches:%d", matches)

    return joiner;
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
long PMJJoiner:: //0x7fff08000c70
join(int32_t tid, tuple_t **tuple, bool IStuple_R, int64_t *matches,
     void *(*thread_fun)(const tuple_t *, const tuple_t *, int64_t *), void *pVoid) {
    auto *arg = (t_pmj *) t_arg;

    int valid_member = 0;
    //store tuples.
    if (IStuple_R) {
        for (auto i = 0; i < progressive_step_tupleR; i++) {
            if (tuple[i] != nullptr) {
                arg->tmp_relR[arg->outerPtrR + arg->innerPtrR] = *tuple[i];
                arg->innerPtrR++;
#ifdef DEBUG
                if (tid == 0) {
                    window0.R_Window.push_back(tuple[i]->key);
                    DEBUGMSG("T0 after receive R (expected): %s, (actual): %s", print_window(window0.R_Window).c_str(),
                             print_tuples(arg->tmp_relR, arg->outerPtrR + arg->innerPtrR).c_str())
                } else {
                    window1.R_Window.push_back(tuple[i]->key);
                    DEBUGMSG("T1 after receive R (expected): %s, (actual): %s", print_window(window1.R_Window).c_str(),
                             print_tuples(arg->tmp_relR, arg->outerPtrR + arg->innerPtrR).c_str())
                }
#endif
                valid_member++;
            }
        }

    } else {
        for (auto i = 0; i < progressive_step_tupleS; i++) {
            if (tuple[i] != nullptr) {
                arg->tmp_relS[arg->outerPtrS + arg->innerPtrS] = *tuple[i];
                arg->innerPtrS++;
#ifdef DEBUG
                if (tid == 0) {
                    window0.S_Window.push_back(tuple[i]->key);
                    DEBUGMSG("T0 after receive S (expected): %s, actual: %s", print_window(window0.S_Window).c_str(),
                             print_tuples(arg->tmp_relS, arg->outerPtrS + arg->innerPtrS).c_str())
                } else {
                    window1.S_Window.push_back(tuple[i]->key);
                    DEBUGMSG("T1 after receive S (expected): %s, actual: %s", print_window(window1.S_Window).c_str(),
                             print_tuples(arg->tmp_relS, arg->outerPtrS + arg->innerPtrS).c_str())
                }
#endif
                valid_member++;
            }
        }
    }
    int stepR = progressive_step_tupleR;
    int stepS = progressive_step_tupleS;

    DEBUGMSG("[TID:%d, arg->outerPtrR:%d, arg->innerPtrR:%d"
             " arg->outerPtrS:%d, arg->innerPtrS:%d]", tid, arg->outerPtrR, arg->innerPtrR, arg->outerPtrS,
             arg->innerPtrS)
//    if (arg->outerPtrR < arg->sizeR - stepR && arg->outerPtrS < arg->sizeS - stepS) {//normal process
    //check if it is ready to start process.
    if (arg->innerPtrR >= stepR
        && arg->innerPtrS >= stepS) {//start process and reset inner pointer.

        DEBUGMSG("Sorting in normal stage")
        sorting_phase(tid, arg->tmp_relR + arg->outerPtrR, arg->tmp_relS + arg->outerPtrS, stepR, stepS,
                      matches, &arg->Q, arg->outptrR + arg->outerPtrR, arg->outptrS + arg->outerPtrS, arg->timer);
        arg->outerPtrR += stepR;
        arg->outerPtrS += stepS;
        DEBUGMSG("Join during run creation:%d", *matches)

        /***Reset Inner Pointer***/
        arg->innerPtrR = 0;
        arg->innerPtrS = 0;
    }
    return valid_member;
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
clean(int32_t tid, tuple_t **tuple, bool cleanR) {

    if (cleanR) {
        for (auto i = 0; i < progressive_step_tupleR; i++) {
            if (tuple[i] != nullptr) {
                auto idx = find_index(this->t_arg->tmp_relR, this->t_arg->outerPtrR + this->t_arg->innerPtrR,
                                      tuple[i]);
                this->t_arg->tmp_relR[idx] = this->t_arg->tmp_relR[this->t_arg->outerPtrR + this->t_arg->innerPtrR - 1];
                this->t_arg->innerPtrR--;
                if (this->t_arg->innerPtrR < 0) {
                    this->t_arg->outerPtrR -= progressive_step_tupleR;
                    this->t_arg->innerPtrR = progressive_step_tupleR - 1;
                }
#ifdef DEBUG
                if (tid == 0) {
                    window0.R_Window.remove(tuple[i]->key);
                    DEBUGMSG("T0 after remove R (expected): %s, actual: %s", print_window(window0.R_Window).c_str(),
                             print_tuples(this->t_arg->tmp_relR,
                                          this->t_arg->outerPtrR + this->t_arg->innerPtrR).c_str())
                } else {
                    window1.R_Window.remove(tuple[i]->key);
                    DEBUGMSG("T1 after remove R (expected): %s, actual: %s", print_window(window1.R_Window).c_str(),
                             print_tuples(this->t_arg->tmp_relR,
                                          this->t_arg->outerPtrR + this->t_arg->innerPtrR).c_str())
                }
#endif
            }
        }
    } else {
        for (auto i = 0; i < progressive_step_tupleS; i++) {
            if (tuple[i] != nullptr) {
                auto idx = find_index(this->t_arg->tmp_relS, this->t_arg->outerPtrS + this->t_arg->innerPtrS,
                                      tuple[i]);
                this->t_arg->tmp_relS[idx] = this->t_arg->tmp_relS[this->t_arg->outerPtrS + this->t_arg->innerPtrS - 1];
                this->t_arg->innerPtrS--;
                if (this->t_arg->innerPtrS < 0) {
                    this->t_arg->outerPtrS -= progressive_step_tupleS;
                    this->t_arg->innerPtrS = progressive_step_tupleS - 1;
                }

#ifdef DEBUG
                if (tid == 0) {
                    window0.S_Window.remove(tuple[i]->key);
                    DEBUGMSG("T0 after remove S (expected): %s,(actual): %s", print_window(window0.S_Window).c_str(),
                             print_tuples(this->t_arg->tmp_relS,
                                          this->t_arg->outerPtrS + this->t_arg->innerPtrS).c_str())
                } else {
                    window1.S_Window.remove(tuple[i]->key);
                    DEBUGMSG("T1 after remove S (expected): %s,(actual): %s", print_window(window1.S_Window).c_str(),
                             print_tuples(this->t_arg->tmp_relS,
                                          this->t_arg->outerPtrS + this->t_arg->innerPtrS).c_str())
                }
#endif
            }
        }
    }
}

long PMJJoiner::
cleanup(int32_t tid, int64_t *matches, void *(*thread_fun)(const tuple_t *, const tuple_t *, int64_t *), void *pVoid) {
    auto *arg = (t_pmj *) t_arg;
    int stepR;
    int stepS;
    /***Handling Left-Over***/
    DEBUGMSG("TID:%d in Clean up stage: sorting", tid)
    stepR = arg->innerPtrR;
    stepS = arg->innerPtrS;
    sorting_phase(tid, arg->tmp_relR + arg->outerPtrR, arg->tmp_relS + arg->outerPtrS, stepR, stepS,
                  matches, &arg->Q, arg->outptrR + arg->outerPtrR, arg->outptrS + arg->outerPtrS,arg->timer);
    DEBUGMSG("TID:%d Clean up stage: Join during run creation:%d, arg->Q %d", tid, *matches, arg->Q.size())
    merging_phase(matches, &arg->Q, nullptr);
    DEBUGMSG("TID:%d Clean up stage: Join during run merge matches:%d", tid, *matches)
    return *matches;
}


PMJJoiner::PMJJoiner(int sizeR, int sizeS, int nthreads) {
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

    if (arg->outerPtrR < arg->sizeR - stepR && arg->outerPtrS < arg->sizeS - stepS) {//normal process
        //check if it is ready to start process.
        if (arg->innerPtrR >= stepR
            && arg->innerPtrS >= stepS) {//start process and reset inner pointer.

            /***Sorting***/
            sorting_phase(tid, arg->tmp_relR + arg->outerPtrR, arg->tmp_relS + arg->outerPtrS, stepR, stepS,
                          matches, &arg->Q, arg->outptrR + arg->outerPtrR, arg->outptrS + arg->outerPtrS,arg->timer);
            arg->outerPtrR += stepR;
            arg->outerPtrS += stepS;
            DEBUGMSG("Join during run creation:%d", *matches)

            /***Reset Inner Pointer***/
            arg->innerPtrR -= stepR;
            arg->innerPtrS -= stepS;
        }
    } else if (arg->outerPtrR + arg->innerPtrR == arg->sizeR &&
               arg->outerPtrS + arg->innerPtrS == arg->sizeS) {//received everything

        /***Handling Left-Over***/
        stepR = arg->sizeR - arg->outerPtrR;
        stepS = arg->sizeS - arg->outerPtrS;
        sorting_phase(tid, arg->tmp_relR + arg->outerPtrR, arg->tmp_relS + arg->outerPtrS, stepR, stepS,
                      matches, &arg->Q, arg->outptrR + arg->outerPtrR, arg->outptrS + arg->outerPtrS,arg->timer);
        DEBUGMSG("Join during run creation:%d", *matches)
        merging_phase(matches, &arg->Q, nullptr);
        DEBUGMSG("Join during run merge matches:%d", *matches)
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
long
rpj(int32_t tid, relation_t *rel_R,
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

    uint32_t cur_step = 0;

    int64_t matches = 0;//number of matches.

    // just a simple nested loop with progressive response, R and S have the same input rate
    do {
        while (index_R < cur_step) {
            if (rel_R->tuples[index_R].key == rel_S->tuples[cur_step].key) {
                matches++;
            }
            index_R++;
        }
        while (index_S <= cur_step) {
            if (rel_R->tuples[cur_step].key == rel_S->tuples[index_S].key) {
                matches++;
            }
            index_S++;
        }
        index_R = 0;
        index_S = 0;
        cur_step++;
//        DEBUGMSG(1, "JOINING: tid: %d, cur step: %d, matches: %d\n", tid, cur_step, matches);
    } while (cur_step < rel_R->num_tuples || cur_step < rel_S->num_tuples);

    destroy_hashtable(htR);
    destroy_hashtable(htS);
    return matches;
}

/**
 *
 *
 * @param relR
 * @param relS
 * @param nthreads
 * @return
 */
long
hrpj(int32_t tid, relation_t *rel_R,
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

    uint32_t cur_step = 0;

    int64_t matches = 0;//number of matches.

    RippleJoiner joiner(rel_R, rel_S, 0);

    // indexed ripple join, assuming R and S have the same input rate.
    do {
        joiner.join(tid, &rel_S->tuples[cur_step], false, &matches, NULL, pVoid);
        joiner.join(tid, &rel_R->tuples[cur_step], true, &matches, NULL, pVoid);
        cur_step++;
//        DEBUGMSG(1, "JOINING: tid: %d, cur step: %d, matches: %d\n", tid, cur_step, matches)
    } while (cur_step < rel_R->num_tuples || cur_step < rel_S->num_tuples);

    destroy_hashtable(htR);
    destroy_hashtable(htS);
    return matches;
}


/**
 * PMJ algorithm to be used in each thread.
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
    fprintf(stdout, "tid: %d, tuple: %d, R?%d\n", tid, tuple->key, tuple_R);
    if (tuple_R) {
//        samList.t_windows->R_Window.push_back(tuple->key);
        samList.t_windows->R_Window.push_back(find_index(relR, tuple));
        match_single_tuple(samList.t_windows->S_Window, relS, tuple, matches, thread_fun);
    } else {
//        samList.t_windows->S_Window.push_back(tuple->key);
        samList.t_windows->S_Window.push_back(find_index(relS, tuple));
        match_single_tuple(samList.t_windows->R_Window, relR, tuple, matches, thread_fun);
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
RippleJoiner::RippleJoiner(relation_t *relR, relation_t *relS, int nthreads) : relR(
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
        samList.t_windows->R_Window.remove(find_index(relR, tuple));
    } else {
        samList.t_windows->S_Window.remove(find_index(relS, tuple));
    }
}


