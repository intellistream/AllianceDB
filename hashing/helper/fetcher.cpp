//
// Created by Shuhao Zhang on 1/11/19.
//

#include <cstdio>
#include <assert.h>
#include "fetcher.h"

fetch_t::fetch_t(fetch_t *fetch) {
    this->tuple = fetch->tuple;
    this->flag = fetch->flag;
    this->ack = fetch->ack;
}

fetch_t::fetch_t() {}

/**
 * CORE Common Function.
 * @param state
 * @param relR
 * @param relS
 * @return
 */
fetch_t *_next_tuple(t_state *state, relation_t *relR, relation_t *relS) {
    if (state->flag) {
        if (state->start_index_R < state->end_index_R) {
            state->fetch.tuple = &relR->tuples[state->start_index_R++];
            state->fetch.flag = state->flag;
            (state->flag) ^= true;//flip flag
            return &(state->fetch);
        } else {
            (state->flag) ^= true;//flip flag
            return nullptr;
        }
    } else {
        if (state->start_index_S < state->end_index_S) {
            state->fetch.tuple = &relS->tuples[state->start_index_S++];
            state->fetch.flag = state->flag;
            (state->flag) ^= true;//flip flag
            return &(state->fetch);
        } else {
            state->flag ^= true;//flip flag
            return nullptr;
        }
    }
}

fetch_t *JB_NP_Fetcher::next_tuple(int tid) {
    return _next_tuple(&state[tid], relR, relS);

}

fetch_t *JM_NP_Fetcher::next_tuple(int tid) {
    return _next_tuple(&state[tid], relR, relS);
}

fetch_t *HS_NP_Fetcher::next_tuple(int tid) {
    if (tid == 0) {//thread 0 fetches R.
        if (state[tid].start_index_R < state[tid].end_index_R) {
            state[tid].fetch.tuple = &relR->tuples[state[tid].start_index_R++];
            state[tid].fetch.flag = true;
            return &(state[tid].fetch);
        } else {
            return nullptr;
        }
    } else {//thread n-1 fetches S.
        if (state[tid].start_index_S < state[tid].end_index_S) {
            state[tid].fetch.tuple = &relS->tuples[state[tid].start_index_S++];
            state[tid].fetch.flag = false;
            return &(state[tid].fetch);
        } else {
            return nullptr;
        }
    }


    return nullptr;
}
