//
// Created by Shuhao Zhang on 22/11/19.
//

#ifndef ALLIANCEDB_PMJ_HELPER_H
#define ALLIANCEDB_PMJ_HELPER_H

#include <assert.h>
#include <numeric>
#include <vector>
#include <utility>
#include <set>
#include <iostream>
#include "../utils/types.h"
#include "../utils/params.h"
#include "../joins/common_functions.h"
#include "avxsort.h"
#include "../utils/tuple_buffer.h"


//#define progressive_step 0.05 //percentile, 0.01 ~ 0.2.
//#define progressive_step_tupleR 640 //progressive #tuples. must be multiple cacheline size (64).
//#define progressive_step_tupleS 640 //progressive #tuples.
//#define merge_step 1000000 // number of ``runs" to merge in each round.


inline struct tuple_t *read(tuple_t *tuple, int length, int idx) {
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


struct run {//a pair of runs

    //used during sorting phase and initial merging phase..
    tuple_t *R = nullptr;
    tuple_t *S = nullptr;

    int posR;//readable position of R. By default should be 0 and up to size of R.
    int posS;//readable position of S.
    int lengthR;
    int lengthS;

    bool merged = false;

    run(tuple_t *run_R, tuple_t *run_S, int lengthR, int lengthS) {
        R = run_R;//only one pair initially.
        S = run_S;//only one pair initially.
        posR = 0;
        posS = 0;
        this->lengthR = lengthR;
        this->lengthS = lengthS;
    }

    //used during later merging phase..
    std::vector<tuple_t *> mergedR;
    std::vector<tuple_t *> mergedS;

    run() {
        posR = 0;
        posS = 0;
        lengthR = 0;
        lengthS = 0;
    }

};

struct sweepArea {

    std::list<const tuple_t *> sx;

    void insert(const tuple_t *tuple) {
        sx.push_back(tuple);
    }

    void query(const tuple_t *tuple, int64_t *matches, T_TIMER *timer, bool ISTupleR,
               chainedtuplebuffer_t *chainedbuf) {
        for (auto it = sx.begin(); it != sx.end();) {
            if (LessPredicate(it.operator*(), tuple)) {  //clean elements that are less than the current element.
                it = sx.erase(it);
            } else {  //perform join.
                if (EqualPredicate(it.operator*(), tuple)) {
                    (*matches)++;
#ifdef JOIN_RESULT_MATERIALIZE
                    tuple_t *joinres = cb_next_writepos(chainedbuf);
                    joinres->key = tuple->key;
                    joinres->payloadID = tuple->payloadID;
#endif
#ifndef NO_TIMING
//                    if (tuple->payloadID > 100000 || tuple->payloadID < 0) {
//                        printf("wrong.");
//                    }
//                    else
//                        printf("payloadID:%d\n", tuple->payloadID);
//                    if (tuple->payloadID < 0) {
//                        printf("???");
//                    }
                    END_PROGRESSIVE_MEASURE(tuple->payloadID, timer, ISTupleR)
#endif
                }
                ++it;
            }
        }
    }
};


/**
 * Actually used in online join algorithm.
 * @param tid
 * @param inptrR
 * @param sizeR
 * @param inptrS
 * @param sizeS
 * @param matches
 * @param Q
 * @param outputR
 * @param outputS
 * @param timer
 * @param chainedbuf
 */
void sorting_phase(int32_t tid, tuple_t *inptrR, int sizeR, tuple_t *inptrS, int sizeS, int64_t *matches,
                   std::vector<run> *Q, tuple_t *outputR, tuple_t *outputS, T_TIMER *timer,
                   chainedtuplebuffer_t *chainedbuf);
/**
 * Testing purpose: pass in entire relation.
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
                   T_TIMER *timer, chainedtuplebuffer_t *chainedbuf);

void
merging_phase(int64_t *matches, std::vector<run> *Q, T_TIMER *timer, chainedtuplebuffer_t *chainedbuf, int merge_step);


#endif //ALLIANCEDB_PMJ_HELPER_H
