//
// Created by Shuhao Zhang on 22/11/19.
//
#include <zconf.h>
#include "pmj_helper.h"
#include "sort_common.h"
#include "localjoiner.h"
#include <algorithm>

void
earlyJoinInitialRuns(tuple_t *tupleR, tuple_t *tupleS, int lengthR, int lengthS, int64_t *matches, T_TIMER *timer,
                     chainedtuplebuffer_t *chainedbuf) {
//    //in early join

    int r = 0;
    int s = 0;
    sweepArea RM;
    sweepArea SM;
    while (r < lengthR || s < lengthS) {
        tuple_t *tr = read(tupleR, lengthR, r);
        tuple_t *ts = read(tupleS, lengthS, s);
        if (s == lengthS || (r < lengthR && tr->key <= ts->key)) {
            RM.insert(tr); //similar to SHJ's build.
            SM.query(tr, matches, timer, true, chainedbuf); //similar to SHJ's probe.
            r++;//remove tr from tupleR.
        } else {
            SM.insert(ts);
            RM.query(ts, matches, timer, false, chainedbuf);
            s++;//remove ts from tupleS.
        }
    }
}

/**
 * Merges the input sequences into two larger sequences, which are then joined directly.
 *
 * @param tupleR
 * @param tupleS
 * @param matches
 */
void earlyJoinMergedRuns(std::vector<run> *Q, int64_t *matches, run *newRun, T_TIMER *timer,
                         chainedtuplebuffer_t *chainedbuf) {
    bool findI;
    bool findJ;

    auto RM = new sweepArea[Q->size()];
    auto SM = new sweepArea[Q->size()];
    do {
        //following PMJ vldb'02 implementation.
        const tuple_t *minR = nullptr;
        const tuple_t *minS = nullptr;
        __gnu_cxx::__normal_iterator<run *, std::vector<run>> i;
        int mark_pi = 0;
        __gnu_cxx::__normal_iterator<run *, std::vector<run>> j;
        int mark_pj = 0;
        findI = false;
        findJ = false;

        //determine the smallest element of r and s from multiple (#merge_step) subsequences.
        //points to correct starting point.
        int run_i = 0;
        int run_j = 0;
        int m = 0;

        int actual_merge_step = (int) Q->size();//std::min((int) Q->size(), merge_step);
        //  MSG("actual_merge_step:%d\n", actual_merge_step);
        for (auto run_itr = Q->begin();
             run_itr < Q->begin() + actual_merge_step; ++run_itr) {//iterate through several runs.
            if (Q->size() < 2) { break; }

            /***HANDLING R.***/
            auto posR = (run_itr).operator*().posR;
            auto lengthR = (run_itr).operator*().lengthR;
            tuple_t *readR = nullptr;
            if (run_itr.operator*().merged) {
                auto runR = (run_itr).operator*().mergedR;//get Rs in each run.
                if (posR < lengthR) {
                    //the left most of each subsequence is the smallest item of the subsequence.
                    readR = runR.at(posR);

                }
            } else {
                tuple_t *runR = (run_itr).operator*().R;//get Rs in each run.
                if (posR < lengthR) {
                    //the left most of each subsequence is the smallest item of the subsequence.
                    readR = &runR[posR];

                }
            }
            if (readR && (!minR || minR->key > readR->key)) {
                minR = readR;
                i = run_itr;//mark the subsequence to be updated.
                findI = true;
                run_i = m;
                mark_pi = posR;
            }
            /***HANDLING S.***/
            auto posS = (run_itr).operator*().posS;
            auto lengthS = (run_itr).operator*().lengthS;
            tuple_t *readS = nullptr;
            if (run_itr.operator*().merged) {
                auto runS = (run_itr).operator*().mergedS;//get Rs in each run.
                if (posS < lengthS) {
                    //the left most of each subsequence is the smallest item of the subsequence.
                    readS = runS.at(posS);
                }
            } else {
                tuple_t *runS = (run_itr).operator*().S;//get Rs in each run.
                if (posS < lengthS) {
                    //the left most of each subsequence is the smallest item of the subsequence.
                    readS = &runS[posS];
                }
            }
            if (readS && (!minS || minS->key > readS->key)) {
                minS = readS;
                j = run_itr;//mark the subsequence to be updated.
                findJ = true;
                run_j = m;
                mark_pj = posS;
            }
            m++;
        }

        if (!findI && !findJ) {
            if (Q->size() < 2) { return; }
//            MSG("[before] Q SIZE: %zu, actual_merge_step %d\n", Q->size(), actual_merge_step);
            Q->erase(Q->begin(), Q->begin() + actual_merge_step);//clean Q.
//            MSG("[after] Q SIZE: %zu, actual_merge_step %d\n", Q->size(), actual_merge_step);
            newRun->merged = true;
            delete[](RM);
            delete[](SM);
            return;
        }
        if (!findJ || (findI && LessEqualPredicate(minR, minS))) {
#ifdef MATCH
            RM[run_i].insert(minR);
            for (auto run_itr = 0; run_itr < actual_merge_step; run_itr++) {
                if (run_itr != run_i) {// except (r,x)| x belong to Si.
                    SM[run_itr].query(minR, matches, timer, false, chainedbuf);
                }
            }
#endif
            if (i.operator*().merged) {
                newRun->mergedR.push_back(
                        i.operator*().mergedR.at(mark_pi));//merge multiple subsequences into a longer sorted one.
            } else {
                newRun->mergedR.push_back(
                        &i.operator*().R[mark_pi]);//merge multiple subsequences into a longer sorted one.
            }
            newRun->lengthR++;
            i.operator*().posR++;//i->posR.erase(i->posR.begin());
            // remove the smallest element from subsequence.
        } else {
#ifdef MATCH
            SM[run_j].insert(minS);
            for (auto run_itr = 0; run_itr < actual_merge_step; run_itr++) {
                if (run_itr != run_j) {// except (x,r)| x belong to Rj.
                    RM[run_itr].query(minS, matches, timer, false, chainedbuf);
                }
            }
#endif
            if (j.operator*().merged) {
                newRun->mergedS.push_back(
                        j.operator*().mergedS.at(mark_pj));//merge multiple subsequences into a longer sorted one.
            } else {
                newRun->mergedS.push_back(
                        &j.operator*().S[mark_pj]);//merge multiple subsequences into a longer sorted one.
            }
            newRun->lengthS++;
            j.operator*().posS++;
        }
    } while (true);//must have merged all subsequences.
}

void insert(std::vector<run> *Q, tuple_t *run_R, int lengthR, tuple_t *run_S, int lengthS) {
#ifdef DEBUG
    if (run_R->payloadID < 0) {
        MSG("wrong");
    }
    if (run_S->payloadID < 0) {
        MSG("wrong");
    }
#endif
    Q->push_back(run(run_R, run_S, lengthR, lengthS));
}

void
merging_phase(int64_t *matches, std::vector<run> *Q, T_TIMER *timer, chainedtuplebuffer_t *chainedbuf) {
#ifdef MERGE
    do {
        run *newRun = new run();//empty run
        earlyJoinMergedRuns(Q, matches, newRun, timer, chainedbuf);
        Q->push_back(*newRun);
    } while (Q->size() > 1);
#endif
}

static inline int
compare_tuples(const tuple_t a, const tuple_t b)
{
    return (a.key < b.key);
}

void scalarsort_tuples(tuple_t **inputptr, tuple_t **outputptr, uint64_t nitems) {
    tuple_t * in  = *inputptr;
    tuple_t * out = *outputptr;

    std::sort(in, in + nitems, compare_tuples);

    *inputptr = out;
    *outputptr = in;
}

void sorting_phase(int32_t tid, tuple_t *inptrR, int sizeR, tuple_t *inptrS, int sizeS, int64_t *matches,
                   std::vector<run> *Q, tuple_t *outputR, tuple_t *outputS, T_TIMER *timer,
                   chainedtuplebuffer_t *chainedbuf) {

//    DEBUGMSG("TID:%d, Initial R [aligned:%d]: %s", tid, is_aligned(inptrR, CACHE_LINE_SIZE),
//             print_relation(inptrR, sizeR).c_str())
    // if (scalarflag)
         scalarsort_tuples(&inptrR, &outputR, sizeR);
    // else
    //     avxsort_tuples(&inptrR, &outputR, sizeR);// the method will swap input and output pointers.
//    DEBUGMSG("TID:%d, Sorted R: %s", tid, print_relation(outputR, sizeR).c_str())

#ifdef DEBUG
    if (!is_sorted_helper((int64_t *) outputR, sizeR)) {
        DEBUGMSG("===> %d-thread -> R is NOT sorted, size = %d\n", tid, sizeR)
    }
#endif
//    DEBUGMSG("%d-thread Initial S [aligned:%d]: %s", tid, is_aligned(inptrS, CACHE_LINE_SIZE),
//             print_relation(inptrS, sizeS).c_str())

    //if (scalarflag)
        scalarsort_tuples(&inptrS, &outputS, sizeS);
    //else
    //    avxsort_tuples(&inptrS, &outputS, sizeS);// the method will swap input and output pointers.

//    DEBUGMSG("Sorted S: %s", print_relation(outputS, sizeS).c_str())

#ifdef DEBUG
    if (!is_sorted_helper((int64_t *) outputS, sizeS)) {
        DEBUGMSG("===> %d-thread -> S is NOT sorted, size = %d\n", tid, sizeS)
    }
#endif

#ifdef MATCH
    earlyJoinInitialRuns(outputR, outputS, sizeR, sizeS, matches, timer, chainedbuf);
#endif

#ifdef MERGE
    //this is considered as part of ``others" overhead.
//    DEBUGMSG("Insert Q.")
    insert(Q, outputR, sizeR, outputS, sizeS);
#endif
}

/**
 * This is used for debug-only.
 * @param tid
 * @param rel_R
 * @param rel_S
 * @param sizeR
 * @param sizeS
 * @param progressive_stepR
 * @param progressive_stepS
 * @param i
 * @param j
 * @param matches
 * @param Q
 * @param outptrR
 * @param outptrS
 * @param timer
 * @param chainedbuf
 */
void sorting_phase(int32_t tid, const relation_t *rel_R, const relation_t *rel_S, int sizeR, int sizeS,
                   int progressive_stepR, int progressive_stepS, int *i, int *j, int64_t *matches, std::vector<run> *Q,
                   tuple_t *outptrR, tuple_t *outptrS,
                   T_TIMER *timer, chainedtuplebuffer_t *chainedbuf) {

    tuple_t *inptrR = nullptr;
    tuple_t *inptrS = nullptr;

    //take subset of R and S to sort and join.
    if (*i < sizeR) {
        inptrR = rel_R->tuples + *i;
        DEBUGMSG("Initial R [aligned:%d]: %s", is_aligned(inptrR, CACHE_LINE_SIZE),
                 print_relation(rel_R->tuples + *i, progressive_stepR).c_str())


        //if (scalarflag)
            scalarsort_tuples(&inptrR, &outptrR, progressive_stepR);
        //else
        //    avxsort_tuples(&inptrR, &outptrR, progressive_stepR);// the method will swap input and output pointers.


        DEBUGMSG("Sorted R: %s",
                 print_relation(outptrR, progressive_stepR).c_str())
#ifdef DEBUG
        //        DEBUGMSG("Address of rel_R: %p, outptrR:%p ", rel_R->tuples, outptrR)
        if (!is_sorted_helper((int64_t *) outptrR, progressive_stepR)) {
            DEBUGMSG("===> %d-thread -> R is NOT sorted, size = %d\n", tid, progressive_stepR)
        }
#endif
    }
    if (*j < sizeS) {
        inptrS = (rel_S->tuples) + *j;

        //if (scalarflag)
            scalarsort_tuples(&inptrS, &outptrS, progressive_stepS);
        //else
        //    avxsort_tuples(&inptrS, &outptrS, progressive_stepS);

        DEBUGMSG("Sorted S: %s",
                 print_relation(outptrS, progressive_stepS).c_str())
#ifdef DEBUG
        //        DEBUGMSG("Address of rel_S: %p, outptrS:%p ", rel_S->tuples, outptrS)
        if (!is_sorted_helper((int64_t *) outptrS, progressive_stepS)) {
            DEBUGMSG("===> %d-thread -> S is NOT sorted, size = %d\n", tid, progressive_stepS)
        }
#endif
    }
    earlyJoinInitialRuns(outptrR, outptrS, progressive_stepR, progressive_stepS, matches, timer, chainedbuf);
    insert(Q, outptrR, progressive_stepR, outptrS, progressive_stepS);
    *i += progressive_stepR;
    *j += progressive_stepS;
}
