//
// Created by Shuhao Zhang on 1/11/19.
//

#ifndef ALLIANCEDB_FETCHER_H
#define ALLIANCEDB_FETCHER_H

#include "../utils/types.h"
#include "../joins/common_functions.h"
#include <stdio.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include "../utils/generator.h"          /* numa_localize() */

using namespace std::chrono;

enum fetcher {
    type_HS_NP_Fetcher, type_JM_NP_Fetcher, type_JM_P_Fetcher, type_JB_NP_Fetcher, type_PMJ_HS_NP_Fetcher
};

struct fetch_t {
    fetch_t(fetch_t* fetch);

    fetch_t();

    tuple_t* tuple = nullptr;//normal tuples.

    tuple_t* fat_tuple = nullptr;//used for PMJ only.

    int fat_tuple_size = 0;

    bool ISTuple_R;//whether this tuple from input R (true) or S (false).

    bool ack = false;//whether this is just a message. Used in HS model.
};

//thread local structure
struct t_state {
    int current_index_R = 0;//configure pointer of start reading point.
    int end_index_R = 0;//configure pointer of end reading point.
    int current_index_S = 0;//configure pointer of start reading point.
    int end_index_S = 0;//configure pointer of end reading point.
    fetch_t current_fetch;
};

class baseFetcher {
  public:
    virtual fetch_t* next_tuple();
    virtual fetch_t* _next_tuple();//helper function to avoid recurrsion
    relation_t* relR;//input relation
    relation_t* relS;//input relation
    bool tryR = true;

    uint64_t* fetchStartTime;//initialize
    T_TIMER* timer;
    t_state* state;
    int tid;

    //used by HS.
    uint64_t cntR = 0;
    uint64_t cntS = 0;

    virtual bool finish() = 0;

    baseFetcher(relation_t* relR, relation_t* relS, int tid, T_TIMER* timer, bool Physical_Partition) {
        this->tid = tid;
        this->relR = relR;
        this->relS = relS;
        this->timer = timer;
    }
};

inline bool last_thread(int i, int nthreads) {
    return i == (nthreads - 1);
}

class PMJ_HS_NP_Fetcher : public baseFetcher {
  public:

    fetch_t* next_tuple() override;
    bool finish() { return false; };
    /**
     * Initialization
     * @param nthreads
     * @param relR
     * @param relS
     */
    PMJ_HS_NP_Fetcher(int nthreads, relation_t* relR, relation_t* relS, int tid, T_TIMER* timer)
        : baseFetcher(relR, relS, tid, timer, false) {
        state = new t_state();

        //let first and last thread to read two streams.
        if (tid == 0) {
            /* replicate relR to thread 0 */
            state->current_index_R = 0;
            state->end_index_R = relR->num_tuples;
        }
        if (tid == nthreads - 1) {
            /* replicate relS to thread [nthread-1] */
            state->current_index_S = 0;
            state->end_index_S = relS->num_tuples;
        }
        DEBUGMSG("TID:%d, R: start_index:%d, end_index:%d\n", tid, state->current_index_R, state->end_index_R);
        DEBUGMSG("TID:%d, S: start_index:%d, end_index:%d\n", tid, state->current_index_S, state->end_index_S);
    }
};

class HS_NP_Fetcher : public baseFetcher {
  public:
    fetch_t* next_tuple() override;
    bool finish() { return false; };
    /**
     * Initialization
     * @param nthreads
     * @param relR
     * @param relS
     */
    HS_NP_Fetcher(int nthreads, relation_t* relR, relation_t* relS, int tid, T_TIMER* timer)
        : baseFetcher(relR, relS, tid, timer, false) {
        state = new t_state();

        //let first and last thread to read two streams.
        if (tid == 0) {
            /* replicate relR to thread 0 */
            state->current_index_R = 0;
            state->end_index_R = relR->num_tuples;
        }
        if (tid == nthreads - 1) {
            /* replicate relS to thread [nthread-1] */
            state->current_index_S = 0;
            state->end_index_S = relS->num_tuples;
        }

        DEBUGMSG("TID:%d, R: start_index:%d, end_index:%d\n", tid, state->current_index_R, state->end_index_R);
        DEBUGMSG("TID:%d, S: start_index:%d, end_index:%d\n", tid, state->current_index_S, state->end_index_S);
    }

};

class JM_NP_Fetcher : public baseFetcher {
  public:

    bool finish() {
        return state->current_index_R == state->end_index_R
            && state->current_index_S == state->end_index_S;
    }

    /**
     * Initialization
     * @param nthreads
     * @param relR
     * @param relS
     */
    JM_NP_Fetcher(int nthreads, relation_t* relR, relation_t* relS, int tid, T_TIMER* timer)
        : baseFetcher(relR, relS, tid, timer, false) {
        state = new t_state();

        int numSthr = relS->num_tuples/nthreads;//replicate R, partition S.

        /* replicate relR for next thread */
        state->current_index_R = 0;
        state->end_index_R = relR->num_tuples; //remember the last index is not count!

        /* assign part of the relS for next thread */
        state->current_index_S = numSthr*tid;
        state->end_index_S =
            (last_thread(tid, nthreads)) ? relS->num_tuples : numSthr*(tid + 1);//remember the last index is not count!

        //        uint64_t ts = 0;
        //        for (auto i = state->current_index_S; i < state->end_index_S; i++) {
        //            auto read = &relS->tuples[i];
        //            auto read_ts = relS->payload->ts[read->payloadID];
        //            if (read_ts >= ts) {
        //                ts = read_ts;
        //            } else {
        //                printf("\nts is not monotonically increasing since:%d, "
        //                       "tid:%d, S:%lu\n", i, tid, read_ts);
        //                break;
        //            }
        //        }
        //        fflush(stdout);
    }
};

class JM_P_Fetcher : public JM_NP_Fetcher {
  public:
    //thread_local copy of input tuples.
    tuple_t* tmpRelR;
    tuple_t* tmpRelS;
    int idx_R = 0;
    int idx_S = 0;

    fetch_t* next_tuple() override {

        auto fetch = baseFetcher::next_tuple();
        if (fetch != nullptr) {
            //copy tuple and exchange pointer.
            if (fetch->ISTuple_R) {
                tmpRelR[idx_R] = fetch->tuple[0];
                fetch->tuple = &tmpRelR[idx_R++];
            } else {
                tmpRelS[idx_S] = fetch->tuple[0];
                fetch->tuple = &tmpRelS[idx_S++];
            }
            return fetch;
        } else return nullptr;
    }

    JM_P_Fetcher(int nthreads, relation_t* relR, relation_t* relS, int tid, T_TIMER* timer) :
        JM_NP_Fetcher(nthreads, relR, relS, tid, timer) {

        /* allocate temporary space for partitioning */
        tmpRelR = (tuple_t*) alloc_aligned(state->end_index_R*sizeof(tuple_t));
        tmpRelS = (tuple_t*) alloc_aligned(state->end_index_S*sizeof(tuple_t));

    }
};

class JB_NP_Fetcher : public baseFetcher {
  public:

    bool finish() {
        return state->current_index_R == state->end_index_R
            && state->current_index_S == state->end_index_S;
    }

    JB_NP_Fetcher(int nthreads, relation_t* relR, relation_t* relS, int tid, T_TIMER* timer)
        : baseFetcher(relR, relS, tid, timer, false) {
        state = new t_state[nthreads];
        int numRthr = relR->num_tuples/nthreads;// partition R,
        int numSthr = relS->num_tuples/nthreads;// partition S.

        /* assign part of the relR for next thread */
        state->current_index_R = numRthr*tid;
        state->end_index_R = (last_thread(tid, nthreads)) ? relR->num_tuples : numRthr*(tid + 1);

        /* assign part of the relS for next thread */
        state->current_index_S = numSthr*tid;
        state->end_index_S = (last_thread(tid, nthreads)) ? relS->num_tuples : numSthr*(tid + 1);

        DEBUGMSG("TID:%d, R: start_index:%d, end_index:%d\n", tid, state->current_index_R, state->end_index_R);
        DEBUGMSG("TID:%d, S: start_index:%d, end_index:%d\n", tid, state->current_index_S, state->end_index_S);

    }
};

#endif //ALLIANCEDB_FETCHER_H
