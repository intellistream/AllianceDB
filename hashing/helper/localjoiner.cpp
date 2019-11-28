//
// Created by Shuhao Zhang on 1/11/19.
//


#include <vector>
#include <assert.h>
#include "avxsort.h"
#include "sort_common.h"
#include "localjoiner.h"
#include "pmj_helper.h"


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

    SHJJoiner joiner;

    do {
        if (index_R < rel_R->num_tuples) {
            joiner.join(tid, &rel_R->tuples[index_R], true, htR, htS, &matches, pVoid, timer);
            index_R++;
        }
        if (index_S < rel_S->num_tuples) {
            joiner.join(tid, &rel_S->tuples[index_S], false, htR, htS, &matches, pVoid, timer);
            index_S++;
        }
    } while (index_R < rel_R->num_tuples || index_S < rel_S->num_tuples);

    destroy_hashtable(htR);
    destroy_hashtable(htS);
    return matches;
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
long SHJJoiner::join(int32_t tid, tuple_t *tuple, bool tuple_R,
                     hashtable_t *htR, hashtable_t *htS, int64_t *matches,
                     void *pVoid, T_TIMER *timer) {

    const uint32_t hashmask_R = htR->hash_mask;
    const uint32_t skipbits_R = htR->skip_bits;

    const uint32_t hashmask_S = htS->hash_mask;
    const uint32_t skipbits_S = htS->skip_bits;

//    DEBUGMSG(1, "JOINING: tid: %d, tuple: %d, R?%d\n", tid, tuple->key, tuple_R)
    if (tuple_R) {
        build_hashtable_single(htR, tuple, hashmask_R, skipbits_R);//(1)
//        DEBUGMSG(1, "tid %d add tuple r %d to R-window. \n", tid, tuple->key)

        if (tid == 0) {
            END_MEASURE_BUILD_ACC((*timer))
        }
        if (tid == 0) {
            proble_hashtable_single_measure(htS, tuple, hashmask_S, skipbits_S, matches,
                                            timer->progressivetimer);//(2)
//            DEBUGMSG("matches:%ld, T0: Join R %d with %s", *matches, tuple->key,
//                     print_window(window0.S_Window).c_str());
        } else {
            proble_hashtable_single(htS, tuple, hashmask_S, skipbits_S, matches);//(4)
//            DEBUGMSG("matches:%ld, T1: Join R %d with %s", *matches, tuple->key,
//                     print_window(window1.S_Window).c_str());

        }
    } else {
        build_hashtable_single(htS, tuple, hashmask_S, skipbits_S);//(3)
//        DEBUGMSG(1, "tid %d add tuple s %d to S-window. \n", tid, tuple->key)

        if (tid == 0) {
            END_MEASURE_BUILD_ACC((*timer))
        }

//        DEBUGMSG(1, "BUILD TABLE FINISH: tid: %d, tuple: %d, R?%d\n", tid, tuple->key, tuple_R)

        if (tid == 0) {
            proble_hashtable_single_measure(htR, tuple, hashmask_R, skipbits_R, matches,
                                            timer->progressivetimer);//(4)
//                DEBUGMSG("matches:%ld, T0: Join S %d with %s", *matches, tuple->key,
//                         print_window(window0.R_Window).c_str());
        } else {
            proble_hashtable_single(htR, tuple, hashmask_R, skipbits_R, matches);//(4)
//                DEBUGMSG("matches:%ld, T1: Join S %d with %s", *matches, tuple->key,
//                         print_window(window1.R_Window).c_str());
        }
    }
//    DEBUGMSG(1, "JOINING FINISH: tid: %d, tuple: %d, R?%d\n", tid, tuple->key, tuple_R)
    return *matches;
}

/**
 * Clean state stored in local thread, basically used in HS mode
 * @param tid
 * @param tuple
 * @param htR
 * @param htS
 * @param cleanR
 */
void SHJJoiner::clean(int32_t tid, tuple_t *tuple, hashtable_t *htR, hashtable_t *htS, bool cleanR) {
    if (cleanR) {
        //if SHJ is used, we need to clean up hashtable of R.
        debuild_hashtable_single(htR, tuple, htR->hash_mask, htR->skip_bits);

//        printf( "tid: %d remove tuple r %d from R-window. \n", arg->tid, fetch->tuple->key);

        if (tid == 0) {
            window0.R_Window.remove(tuple->key);
//            print_window(window0.R_Window, 0);
        } else {
            window1.R_Window.remove(tuple->key);
//            print_window(window1.R_Window, 1);
        }
    } else {
        debuild_hashtable_single(htS, tuple, htS->hash_mask, htS->skip_bits);

//        printf("tid: %d remove tuple s %d from S-window. \n", arg->tid, fetch->tuple->key);

        if (tid == 0) {
            window0.S_Window.remove(tuple->key);
//            print_window(window0.S_Window, 0);
        } else {
            window1.S_Window.remove(tuple->key);
//            print_window(window1.S_Window, 1);
        }
//        std::cout << boost::stacktrace::stacktrace() << std::endl;

    }
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
long
pmj(int32_t tid, relation_t *rel_R, relation_t *rel_S, void *pVoid, T_TIMER *timer) {

    //Phase 1 ('Join during run creation')
    int sizeR = rel_R->num_tuples;
    int sizeS = rel_S->num_tuples;
    int i = 0;
    int j = 0;
    int matches = 0;
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
        sorting_phase(tid, rel_R, rel_S, sizeR, sizeS, progressive_stepR, progressive_stepS, &i, &j, &matches, &Q,
                      outptrR + i, outptrS + j);

    } while (i < sizeR - progressive_stepR || j < sizeS - progressive_stepS);//while R!=null, S!=null.

    /***Handling Left-Over***/
    progressive_stepR = sizeR - i;
    progressive_stepS = sizeS - j;
    sorting_phase(tid, rel_R, rel_S, sizeR, sizeS, progressive_stepR, progressive_stepS, &i, &j, &matches, &Q,
                  outptrR + i, outptrS + j);

    DEBUGMSG("Join during run creation:%d", matches)

    merging_phase(&matches, &Q);

    DEBUGMSG("Join during run merge matches:%d", matches)
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
long PMJJoiner::join(int32_t tid, tuple_t *tuple, bool tuple_R, hashtable_t *htR, hashtable_t *htS, int64_t *matches,
                     void *pVoid, T_TIMER *timer) {
    return 0;
}

/**
 * HS cleaner
 * @param tid
 * @param tuple
 * @param htR
 * @param htS
 * @param cleanR
 */
void PMJJoiner::clean(int32_t tid, tuple_t *tuple, hashtable_t *htR, hashtable_t *htS, bool cleanR) {
    return;
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
        joiner.join(tid, &rel_S->tuples[cur_step], false, htR, htS, &matches, pVoid, timer);
        joiner.join(tid, &rel_R->tuples[cur_step], true, htR, htS, &matches, pVoid, timer);
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

long RippleJoiner::join(int32_t tid, tuple_t *tuple, bool tuple_R, hashtable_t *htR, hashtable_t *htS, int64_t *matches,
                        void *pVoid, T_TIMER *timer) {
    fprintf(stdout, "tid: %d, tuple: %d, R?%d\n", tid, tuple->key, tuple_R);
    if (tuple_R) {
//        samList.t_windows[tid].R_Window.push_back(tuple->key);
        samList.t_windows[tid].R_Window.push_back(find_index(relR, tuple));
        match_single_tuple(samList.t_windows[tid].S_Window, relS, tuple, matches);
    } else {
//        samList.t_windows[tid].S_Window.push_back(tuple->key);
        samList.t_windows[tid].S_Window.push_back(find_index(relS, tuple));
        match_single_tuple(samList.t_windows[tid].R_Window, relR, tuple, matches);
    }

    // Compute estimation result

    long estimation_result = 0;
    if (samList.t_windows[tid].R_Window.size() > 0 && samList.t_windows[tid].S_Window.size() > 0) {
        estimation_result =
                ((int) relR->num_tuples * (int) relS->num_tuples)
                /
                ((int) samList.t_windows[tid].R_Window.size() * (int) samList.t_windows[tid].S_Window.size())
                *
                (int) (*matches);
    } else {
        estimation_result = *matches;
    }

//    fprintf(stdout, "estimation result: %d \n", estimation_result);

    return *matches;

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
    samList.t_windows = new t_window[nthreads];
}


/**
 *
 * @param tid
 * @param tuple
 * @param htR
 * @param htS
 * @param cleanR
 */
void RippleJoiner::clean(int32_t tid, tuple_t *tuple, hashtable_t *htR, hashtable_t *htS, bool cleanR) {
    if (cleanR) {
        samList.t_windows[tid].R_Window.remove(find_index(relR, tuple));
    } else {
        samList.t_windows[tid].S_Window.remove(find_index(relS, tuple));
    }
}


