//
// Created by Shuhao Zhang on 1/11/19.
//

#ifndef ALLIANCEDB_FETCHER_H
#define ALLIANCEDB_FETCHER_H

#include "../utils/types.h"
#include "../joins/common_functions.h"
#include <stdio.h>
#include <unistd.h>
#include <chrono>
#include <thread>

using namespace std::chrono;

enum fetcher {
    type_HS_NP_Fetcher, type_JM_NP_Fetcher, type_JB_NP_Fetcher, type_PMJ_HS_NP_Fetcher
};


struct fetch_t {
    fetch_t(fetch_t *fetch);

    fetch_t();

    tuple_t *tuple = nullptr;//normal tuples.

    tuple_t *fat_tuple = nullptr;//used for PMJ only.

    int fat_tuple_size = 0;

    bool ISTuple_R;//whether this tuple from input R (true) or S (false).

    bool ack = false;//whether this is just a message. Used in HS model.
};

//thread local structure
struct t_state {
    int start_index_R = 0;//configure pointer of start reading point.
    int end_index_R = 0;//configure pointer of end reading point.
    int start_index_S = 0;//configure pointer of start reading point.
    int end_index_S = 0;//configure pointer of end reading point.
    //read R/S alternatively.
    bool IsTupleR;
    fetch_t fetch;
};


inline milliseconds now() {
    milliseconds ms = duration_cast<milliseconds>(
            system_clock::now().time_since_epoch()
    );
    return ms;
}

class baseFetcher {
public:
    virtual fetch_t *next_tuple();

    relation_t *relR;//input relation
    relation_t *relS;//input relation

    milliseconds *RdataTime;
    milliseconds *SdataTime;
    milliseconds fetchStartTime;//initialize
    t_state *state;
    int tid;
    bool start = true;

    //used by HS.
    uint64_t cntR = 0;
    uint64_t cntS = 0;

    milliseconds RtimeGap(milliseconds *time) {
        if (start) {
            fetchStartTime = now();
            start = false;
        }
        return (*time - *RdataTime) -
               (now() - fetchStartTime);//if it's positive, the tuple is not ready yet.
    }

    milliseconds StimeGap(milliseconds *time) {
        if (start) {
            fetchStartTime = now();
            start = false;
        }
        return (*time - *SdataTime) -
               (now() - fetchStartTime);//if it's positive, the tuple is not ready yet.
    }

    virtual bool finish() = 0;

    baseFetcher(relation_t *relR, relation_t *relS, int tid) {
        this->tid = tid;
        this->relR = relR;
        this->relS = relS;
        RdataTime = relR->payload->ts;
        SdataTime = relS->payload->ts;
    }
};

inline bool last_thread(int i, int nthreads) {
    return i == (nthreads - 1);
}

class PMJ_HS_NP_Fetcher : public baseFetcher {
public:


    bool finish() {

        return cntR == relR->num_tuples && cntS == relS->num_tuples;
    }

    fetch_t *next_tuple() override;

    /**
     * Initialization
     * @param nthreads
     * @param relR
     * @param relS
     */
    PMJ_HS_NP_Fetcher(int nthreads, relation_t *relR, relation_t *relS, int tid)
            : baseFetcher(relR, relS, tid) {
        state = new t_state();

        //let first and last thread to read two streams.
        if (tid == 0) {
            state->IsTupleR = true;
            /* replicate relR to thread 0 */
            state->start_index_R = 0;
            state->end_index_R = relR->num_tuples;
        }
        if (tid == nthreads - 1) {
            /* replicate relS to thread [nthread-1] */
            state->start_index_S = 0;
            state->end_index_S = relS->num_tuples;
        }

        DEBUGMSG("TID:%d, R: start_index:%d, end_index:%d\n", tid, state->start_index_R, state->end_index_R);
        DEBUGMSG("TID:%d, S: start_index:%d, end_index:%d\n", tid, state->start_index_S, state->end_index_S);
    }
};

class HS_NP_Fetcher : public baseFetcher {
public:

    bool finish() {
        return cntR == relR->num_tuples && cntS == relS->num_tuples;
    }


    /**
     * Initialization
     * @param nthreads
     * @param relR
     * @param relS
     */
    HS_NP_Fetcher(int nthreads, relation_t *relR, relation_t *relS, int tid)
            : baseFetcher(relR, relS, tid) {
        state = new t_state();

        //let first and last thread to read two streams.
        if (tid == 0) {
            state->IsTupleR = true;
            /* replicate relR to thread 0 */
            state->start_index_R = 0;
            state->end_index_R = relR->num_tuples;
        }
        if (tid == nthreads - 1) {
            /* replicate relS to thread [nthread-1] */
            state->start_index_S = 0;
            state->end_index_S = relS->num_tuples;
        }

        DEBUGMSG("TID:%d, R: start_index:%d, end_index:%d\n", tid, state->start_index_R, state->end_index_R);
        DEBUGMSG("TID:%d, S: start_index:%d, end_index:%d\n", tid, state->start_index_S, state->end_index_S);
    }


};

class JM_NP_Fetcher : public baseFetcher {
public:


    bool finish() {
/*
 *      if (cntR == relR->num_tuples / 4) {
            printf("Thread %d has finished process input  0.25 R", tid);
        } else if (cntR == relR->num_tuples / 2) {
            printf("Thread %d has finished process input  0.5 R", tid);
        } else if (cntR == relR->num_tuples / 4 * 3) {
            printf("Thread %d has finished process input  0.75 R", tid);
        }

        if (cntS == relS->num_tuples / 4) {
            printf("Thread %d has finished process input  0.25 S", tid);
        } else if (cntS == relS->num_tuples / 2) {
            printf("Thread %d has finished process input  0.5 S", tid);
        } else if (cntS == relS->num_tuples / 4 * 3) {
            printf("Thread %d has finished process input  0.75 S", tid);
        }
 * */
        return state->start_index_R == state->end_index_R
               && state->start_index_S == state->end_index_S;
    }

    /**
     * Initialization
     * @param nthreads
     * @param relR
     * @param relS
     */
    JM_NP_Fetcher(int nthreads, relation_t *relR, relation_t *relS, int tid)
            : baseFetcher(relR, relS, tid) {
        state = new t_state();


        int numSthr = relS->num_tuples / nthreads;//replicate R, partition S.

        state->IsTupleR = true;
        /* replicate relR for next thread */
        state->start_index_R = 0;
        state->end_index_R = relR->num_tuples;

        /* assign part of the relS for next thread */
        state->start_index_S = numSthr * tid;
        state->end_index_S = (last_thread(tid, nthreads)) ? relS->num_tuples : numSthr * (tid + 1);

        DEBUGMSG("TID:%d, R: start_index:%d, end_index:%d\n", tid, state->start_index_R, state->end_index_R);
        DEBUGMSG("TID:%d, S: start_index:%d, end_index:%d\n", tid, state->start_index_S, state->end_index_S);
    }
};

class JB_NP_Fetcher : public baseFetcher {
public:


    bool finish() {
        return state->start_index_R == state->end_index_R
               && state->start_index_S == state->end_index_S;
    }

    JB_NP_Fetcher(int nthreads, relation_t *relR, relation_t *relS, int tid)
            : baseFetcher(relR, relS, tid) {
        state = new t_state[nthreads];
        int numRthr = relR->num_tuples / nthreads;// partition R,
        int numSthr = relS->num_tuples / nthreads;// partition S.

        state->IsTupleR = true;
        /* assign part of the relR for next thread */
        state->start_index_R = numRthr * tid;
        state->end_index_R = (last_thread(tid, nthreads)) ? relR->num_tuples : numRthr * (tid + 1);

        /* assign part of the relS for next thread */
        state->start_index_S = numSthr * tid;
        state->end_index_S = (last_thread(tid, nthreads)) ? relS->num_tuples : numSthr * (tid + 1);

        DEBUGMSG("TID:%d, R: start_index:%d, end_index:%d\n", tid, state->start_index_R, state->end_index_R);
        DEBUGMSG("TID:%d, S: start_index:%d, end_index:%d\n", tid, state->start_index_S, state->end_index_S);

    }
};

#endif //ALLIANCEDB_FETCHER_H
