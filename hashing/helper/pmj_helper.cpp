//
// Created by Shuhao Zhang on 22/11/19.
//
#include "pmj_helper.h"
#include "sort_common.h"


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

void insert(std::vector<run> *Q, int startR, int lengthR, int startS, int lengthS) {
    std::vector<int> v(lengthR);
    std::iota(v.begin(), v.end(), startR);

    std::vector<int> u(lengthS);
    std::iota(u.begin(), u.end(), startS);
    Q->emplace_back(v, u);
}

void merging_phase(const struct relation_t *rel_R, const struct relation_t *rel_S, int *matches, std::vector<run> *Q)
        {
    do {
        //Let them be two empty runs.
        std::vector<int> sortedR;//only records the position.
        std::vector<int> sortedS;//only records the position.
        earlyJoinMergedRuns(rel_R->tuples, rel_S->tuples, Q, matches, &sortedR, &sortedS);
        Q->emplace_back(sortedR, sortedS);
    } while (Q->size() > 1);
}

void sorting_phase(int32_t tid, const struct relation_t *rel_R, const struct relation_t *rel_S, int sizeR, int sizeS,
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