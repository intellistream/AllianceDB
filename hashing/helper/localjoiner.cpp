//
// Created by Shuhao Zhang on 1/11/19.
//

#include <iostream>
#include <set>
#include <utility>
#include <vector>
#include <numeric>
#include <assert.h>
#include "avxsort.h"
#include "sort_common.h"
#include "localjoiner.h"
#include "../utils/params.h"

#define progressive_step 0.2 //percentile, 0.01 ~ 0.2.
#define merge_step 2 // number of ``runs" to merge in each round.

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


inline tuple_t *read(tuple_t *tuple, int length, int idx) {
    if (idx >= length) return nullptr;
    return &tuple[idx];
}

inline const tuple_t *read(const tuple_t *tuple, int idx) {
    return &tuple[idx];
}

inline bool EqualPredicate(const tuple_t *u, const tuple_t *v) {
    return u->key == v->key;
}

inline bool LessPredicate(const tuple_t *u, const tuple_t *v) {
    return u->key < v->key;
}

inline bool LessEqualPredicate(const tuple_t *u, const tuple_t *v) {
    return u->key <= v->key;
}

struct run {//a pair of subsequence (mask position only)
    std::vector<int> posR;//readable position of R. By default should be 0 to size of R/S.
    std::vector<int> posS;//readable position of S.
    run(std::vector<int> posR, std::vector<int> posS) {
        this->posR = std::move(posR);
        this->posS = std::move(posS);
    }

};

struct sweepArea {

    std::set<const tuple_t *> sx;

    void insert(const tuple_t *tuple) {
        sx.insert(tuple);
    }

/*
    bool within(tuple_t *const &x, tuple_t *List, std::vector<int> pos) {
        for (auto it = pos.begin(); it != pos.end(); it++) {
            auto tmpt = read(List, it.operator*());
            if (x == tmpt) {
                return true;
            }
        }
        return false;
    }

    void query(tuple_t *tuple, int *matches, std::vector<int> pos, tuple_t *tupleList) {

        //clean elements that are less than the current element.
        for (auto it = sx.begin(); it != sx.end();) {
            if (LessPredicate(it.operator*(), tuple)) {
                it = sx.erase(it);
            } else {  //perform join.
                if (EqualPredicate(it.operator*(), tuple)) {
                    if (!within(it.operator*(), tupleList, pos))//otherwise, they must have already matched before.
                        (*matches)++;
                }
                ++it;
            }
        }
    }
*/
    void query(const tuple_t *tuple, int *matches) {

        //clean elements that are less than the current element.
        for (auto it = sx.begin(); it != sx.end();) {
            if (LessPredicate(it.operator*(), tuple)) {
                it = sx.erase(it);
            } else {  //perform join.
                if (EqualPredicate(it.operator*(), tuple)) {
                    (*matches)++;
                }
                ++it;
            }
        }
    }
};

void earlyJoinInitialRuns(tuple_t *tupleR, tuple_t *tupleS, int length, int i, int *matches);


void earlyJoinInitialRuns(tuple_t *tupleR, tuple_t *tupleS, int lengthR, int lengthS, int *matches) {
//    //in early join
//    printf("Tuple R: %s\n", print_relation(tupleR, lengthR).c_str());
//    printf("Tuple S: %s\n", print_relation(tupleS, lengthS).c_str());
//    fflush(stdout);

    int r = 0;
    int s = 0;
    sweepArea RM;
    sweepArea SM;
    while (r < lengthR || s < lengthS) {
        tuple_t *tr = read(tupleR, lengthR, r);
        tuple_t *ts = read(tupleS, lengthS, s);
        if (s == lengthS || (r < lengthR && tr->key <= ts->key)) {
            RM.insert(tr); //similar to SHJ's build.
            SM.query(tr, matches); //similar to SHJ's probe.
            r++;//remove tr from tupleR.
        } else {
            SM.insert(ts);
            RM.query(ts, matches);
            s++;//remove ts from tupleS.
        }
    }
}


void earlyJoinMergedRuns(const tuple_t *tupleR, const tuple_t *tupleS, std::vector<run> *Q, int *matches,
                         std::vector<int> *sortedR, std::vector<int> *sortedS);

/**
 * Merges the input sequences into two larger sequences, which are then joined directly.
 *
 * @param tupleR
 * @param tupleS
 * @param matches
 */
void earlyJoinMergedRuns(const tuple_t *tupleR, const tuple_t *tupleS, std::vector<run> *Q, int *matches,
                         std::vector<int> *sortedR, std::vector<int> *sortedS) {
    bool findI;
    bool findJ;
    //following PMJ vldb'02 implementation.
    auto RM = new sweepArea[merge_step];
    auto SM = new sweepArea[merge_step];

    do {
        const tuple_t *minR = nullptr;
        const tuple_t *minS = nullptr;
        __gnu_cxx::__normal_iterator<run *, std::vector<run>> i;
        __gnu_cxx::__normal_iterator<run *, std::vector<run>> j;
        findI = false;
        findJ = false;

        //determine the smallest element of r and s from multiple (#merge_step) subsequences.
        //points to correct starting point.
        int run_i = 0;
        int run_j = 0;
        int m = 0;
        for (auto run_itr = Q->begin();
             run_itr < Q->begin() + merge_step; ++run_itr) {//iterate through several runs.
            auto posR = (run_itr).operator*().posR;
            if (!posR.empty()) {
                //the left most of each subsequence is the smallest item of the subsequence.
                const tuple_t *readR = read(tupleR, posR.at(0));
                if (!minR || minR->key > readR->key) {
                    minR = readR;
                    i = run_itr;//mark the subsequence to be updated.
                    findI = true;
                    run_i = m;
                }
            }
            auto posS = (run_itr).operator*().posS;
            if (!posS.empty()) {
                const tuple_t *readS = read(tupleS, posS.at(0));
                if (!minS || minS->key > readS->key) {
                    minS = readS;
                    j = run_itr;//mark the subsequence to be updated.
                    findJ = true;
                    run_j = m;
                }
            }
            m++;
        }

        if (!findI && !findJ) {
            Q->erase(Q->begin(), Q->begin() + merge_step);//clean Q.
            return;
        }
        if (!findJ || (findI && LessEqualPredicate(minR, minS))) {
            RM[run_i].insert(minR);
            for (auto run_itr = 0; run_itr < merge_step; run_itr++) {
                if (run_itr != run_i) {// except (r,x)| x belong to Si.
                    SM[run_itr].query(minR, matches);
                }
            }
            sortedR->push_back(i->posR.begin().operator*());//merge multiple subsequences into a longer sorted one.
            i->posR.erase(i->posR.begin());//remove the smallest element from subsequence.
        } else {
            SM[run_j].insert(minS);
            for (auto run_itr = 0; run_itr < merge_step; run_itr++) {
                if (run_itr != run_j) {// except (x,r)| x belong to Rj.
                    RM[run_itr].query(minS, matches);
                }
            }
            sortedS->push_back(j->posS.begin().operator*());//merge multiple subsequences into a longer sorted one.
            j->posS.erase(j->posS.begin());//remove the smallest element from subsequence.
        }
    } while (true);//must have merged all subsequences.
}


void insert(std::vector<run> *Q, int startR, int lengthR, int startS, int lengthS);

void sorting_phase(int32_t tid, const relation_t *rel_R, const relation_t *rel_S, int sizeR, int sizeS,
                   int progressive_stepR, int progressive_stepS, int *i, int *j, int *matches, std::vector<run> *Q);

void merging_phase(const relation_t *rel_R, const relation_t *rel_S, int *matches, std::vector<run> *Q);

void insert(std::vector<run> *Q, int startR, int lengthR, int startS, int lengthS) {
    std::vector<int> v(lengthR);
    std::iota(v.begin(), v.end(), startR);

    std::vector<int> u(lengthS);
    std::iota(u.begin(), u.end(), startS);
    Q->emplace_back(v, u);
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
    int progressive_stepR = progressive_step * sizeR;
    int progressive_stepS = progressive_step * sizeS;

    assert(progressive_stepR > 0 && progressive_stepS > 0);

    std::vector<run> Q;//let Q be an empty set;

    /***Sorting***/
    do {
        sorting_phase(tid, rel_R, rel_S, sizeR, sizeS, progressive_stepR, progressive_stepS, &i, &j, &matches, &Q);
    } while (i < sizeR - progressive_stepR || j < sizeS - progressive_stepS);//while R!=null, S!=null.

    /***Handling Left-Over***/
    progressive_stepR = sizeR - i;
    progressive_stepS = sizeS - j;
    sorting_phase(tid, rel_R, rel_S, sizeR, sizeS, progressive_stepR, progressive_stepS, &i, &j, &matches, &Q);


    DEBUGMSG("Join during run creation:%d", matches)
    fflush(stdout);

    merging_phase(rel_R, rel_S, &matches, &Q);

    DEBUGMSG("Join during run merge matches:%d", matches)
    return matches;
}

void merging_phase(const relation_t *rel_R, const relation_t *rel_S, int *matches, std::vector<run> *Q) {
    do {
        //Let them be two empty runs.
        std::vector<int> sortedR;//only records the position.
        std::vector<int> sortedS;//only records the position.
        earlyJoinMergedRuns(rel_R->tuples, rel_S->tuples, Q, matches, &sortedR, &sortedS);
        Q->emplace_back(sortedR, sortedS);
    } while (Q->size() > 1);
}

void sorting_phase(int32_t tid, const relation_t *rel_R, const relation_t *rel_S, int sizeR, int sizeS,
                   int progressive_stepR, int progressive_stepS, int *i, int *j, int *matches, std::vector<run> *Q) {
    tuple_t *inptrR = nullptr;
    tuple_t *inptrS = nullptr;

    /**** allocate temporary space for sorting ****/
    size_t relRsz;
    tuple_t *outptrR;//args->tmp_sortR + my_tid * CACHELINEPADDING(PARTFANOUT);
    tuple_t *outptrS;

    relRsz = progressive_stepR * sizeof(tuple_t)
             + RELATION_PADDING(1, CACHELINEPADDING(1));

    outptrR = (tuple_t *) malloc_aligned(relRsz);

    relRsz = progressive_stepS * sizeof(tuple_t)
             + RELATION_PADDING(1, CACHELINEPADDING(1));

    outptrS = (tuple_t *) malloc_aligned(relRsz);

    //take subset of R and S to sort and join.
    if (*i < sizeR) {
        inptrR = rel_R->tuples + *i;
        DEBUGMSG("Initial R [aligned:%d]: %s", is_aligned(inptrR, CACHE_LINE_SIZE),
                 print_relation(rel_R->tuples + *i, progressive_stepR).c_str())
        avxsort_tuples(&inptrR, &outptrR, progressive_stepR);// the method will swap input and output pointers.
        DEBUGMSG("Sorted R: %s",
                 print_relation(rel_R->tuples + (*i), progressive_stepR).c_str())
#ifdef DEBUG
        if (!is_sorted_helper((int64_t *) outptrR, progressive_stepR)) {
            DEBUGMSG("===> %d-thread -> R is NOT sorted, size = %f\n", tid, progressive_step)
        }
#endif
    }
    if (*j < sizeS) {
        inptrS = (rel_S->tuples) + *j;
        avxsort_tuples(&inptrS, &outptrS, progressive_stepS);
        DEBUGMSG("Sorted S: %s",
                 print_relation(rel_S->tuples + (*i), progressive_stepS).c_str())
#ifdef DEBUG
        if (!is_sorted_helper((int64_t *) outptrS, progressive_stepS)) {
            DEBUGMSG("===> %d-thread -> S is NOT sorted, size = %f\n", tid, progressive_step)
        }
#endif
    }
    earlyJoinInitialRuns(outptrR, outptrS, progressive_stepR, progressive_stepS, matches);
    insert(Q, *i, progressive_stepR, *j, progressive_stepS);
    *i += progressive_stepR;
    *j += progressive_stepS;
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

    // define the current relation that do predicate
//    relation_t *cur_rel = rel_S;
//    uint32_t *cur_rel_pos = &index_S;
//    bool is_inner_looping = true;

    int64_t matches = 0;//number of matches.

    RippleJoiner joiner;


    // square ripple join, but the nested loop has been reduced.
//    do { // loop until return is called
//        if (is_inner_looping) { // scaning side of a rectangle
//            while (*cur_rel_pos < cur_step) {
//                if (*cur_rel_pos < cur_step || cur_rel == rel_S) {
//                    (*cur_rel_pos)++;
//                    // update index
//                    DEBUGMSG(1, "JOINING: tid: %d, matches: %d, %d\n", tid, rel_R->tuples[index_R].key, rel_S->tuples[index_S].key)
//                    if (rel_R->tuples[index_R].key == rel_S->tuples[index_S].key) {
//                        matches++; // predicate match
//                        DEBUGMSG(1, "JOINING: tid: %d, matches: %d\n", tid, matches)
//                    }
//                } else {
//                    break;
//                }
//            }
//            is_inner_looping = false; // finish a side
//        } else { // done with one side of a rectangle
//            if (cur_rel == rel_S) {
//                cur_step++;
//            }// finished a step
//            (*cur_rel_pos)++; // set cur_rel to new cur_step
//            cur_rel = (cur_rel == rel_S) ? rel_R : rel_S; // toggle cur_rel
//            cur_rel_pos = (cur_rel_pos == &index_R) ? &index_S : &index_R;
//            *cur_rel_pos = 0;
//            is_inner_looping = true;
//        }
//    } while (cur_step < rel_R->num_tuples || cur_step < rel_S->num_tuples);

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

    RippleJoiner joiner;

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

    const uint32_t hashmask_R = htR->hash_mask;
    const uint32_t skipbits_R = htR->skip_bits;

    const uint32_t hashmask_S = htS->hash_mask;
    const uint32_t skipbits_S = htS->skip_bits;

//    DEBUGMSG(1, "JOINING: tid: %d, tuple: %d, R?%d\n", tid, tuple->key, tuple_R)
    if (tuple_R) {
        if (tid == 0) {
            BEGIN_MEASURE_BUILD_ACC((*timer))
//            window0.R_Window.push_back(tuple->key);
        } else {
//            window1.R_Window.push_back(tuple->key);
        }
        build_hashtable_single(htR, tuple, hashmask_R, skipbits_R);//(1)
//        DEBUGMSG(1, "tid %d add tuple r %d to R-window. \n", tid, tuple->key)

        if (tid == 0) {
            END_MEASURE_BUILD_ACC((*timer))
        }
        if (tid == 0) {
            proble_hashtable_single_measure(htS, tuple, hashmask_S, skipbits_S, matches,
                                            timer->progressivetimer);//(2)
//            DEBUGMSG(1, "matches:%ld, T0: Join R %d with %s", *matches, tuple->key,
//                     print_window(window0.S_Window).c_str());
        } else {
            proble_hashtable_single(htS, tuple, hashmask_S, skipbits_S, matches);//(4)
//            DEBUGMSG(1, "matches:%ld, T1: Join R %d with %s", *matches, tuple->key,
//                     print_window(window1.S_Window).c_str());

        }
    } else {
//        DEBUGMSG(1, "BUILD TABLE: tid: %d, tuple: %d, R?%d\n", tid, tuple->key, tuple_R)

        if (tid == 0) {
            BEGIN_MEASURE_BUILD_ACC((*timer))
//            window0.S_Window.push_back(tuple->key);
        } else {
//            window1.S_Window.push_back(tuple->key);
        }
        build_hashtable_single(htS, tuple, hashmask_S, skipbits_S);//(3)

//        DEBUGMSG(1, "tid %d add tuple s %d to S-window. \n", tid, tuple->key)

        if (tid == 0) {
            END_MEASURE_BUILD_ACC((*timer))
        }

//        DEBUGMSG(1, "BUILD TABLE FINISH: tid: %d, tuple: %d, R?%d\n", tid, tuple->key, tuple_R)

        if (tid == 0) {
            proble_hashtable_single_measure(htR, tuple, hashmask_R, skipbits_R, matches, timer->progressivetimer);//(4)
//            DEBUGMSG(1, "matches:%ld, T0: Join S %d with %s", *matches, tuple->key,
//                     print_window(window0.R_Window).c_str());
        } else {
            proble_hashtable_single(htR, tuple, hashmask_R, skipbits_R, matches);//(4)
//            DEBUGMSG(1, "matches:%ld, T1: Join S %d with %s", *matches, tuple->key,
//                     print_window(window1.R_Window).c_str());
        }
    }
    // TODO: Ripple join should output its approximate results, this is the only difference from SHJ
//    DEBUGMSG(1, "JOINING FINISH: tid: %d, tuple: %d, R?%d\n", tid, tuple->key, tuple_R)
    return *matches;
}
