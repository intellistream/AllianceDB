//
// Created by Shuhao Zhang on 1/11/19.
//

#include <cstdio>
#include <assert.h>
#include "fetcher.h"
#include "pmj_helper.h"

fetch_t::fetch_t(fetch_t *fetch) {
    this->fat_tuple=fetch->fat_tuple;
    this->tuple = fetch->tuple;
    this->flag = fetch->flag;
    this->ack = fetch->ack;
}

fetch_t::fetch_t() {}

/**
 * CORE Common Function. Alternatively fetch R or S.
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

/**
 * CORE Common Function. Alternatively fetch R or S with copying
 * @param state
 * @param relR
 * @param relS
 * @return
 */
fetch_t *_next_tuple_CP(t_state *state, relation_t *relR, relation_t *relS) {
    if (state->flag) {
        if (state->start_index_R < state->end_index_R) {
            state->fetch.tuple = new tuple_t();
            state->fetch.tuple->payload = relR->tuples[state->start_index_R].payload;
            state->fetch.tuple->key = relR->tuples[state->start_index_R++].key;
            state->fetch.flag = state->flag;
            (state->flag) ^= true;//flip flag
            return &(state->fetch);
        } else {
            (state->flag) ^= true;//flip flag
            return nullptr;
        }
    } else {
        if (state->start_index_S < state->end_index_S) {
            state->fetch.tuple = new tuple_t();
            state->fetch.tuple->payload = relS->tuples[state->start_index_S].payload;
            state->fetch.tuple->key = relS->tuples[state->start_index_S++].key;
            state->fetch.flag = state->flag;
            (state->flag) ^= true;//flip flag
            return &(state->fetch);
        } else {
            state->flag ^= true;//flip flag
            return nullptr;
        }
    }
}

fetch_t *JM_NP_Fetcher::next_tuple(int tid) {
    return _next_tuple(state, relR, relS);
}

fetch_t *JM_P_Fetcher::next_tuple(int tid) {
    return _next_tuple_CP(state, relR, relS);
}

fetch_t *JB_NP_Fetcher::next_tuple(int tid) {
    return _next_tuple(state, relR, relS);
}

fetch_t *PMJ_HS_NP_Fetcher::next_tuple(int tid) {
    if (tid == 0) {//thread 0 fetches R.
        state->fetch.fat_tuple = new tuple_t *[progressive_step_tupleR];
        for (auto i = 0; i < progressive_step_tupleR; i++) {
            if (state->start_index_R < state->end_index_R) {
                state->fetch.fat_tuple[i] = &relR->tuples[state->start_index_R++];
            } else {
//                return nullptr;
                state->fetch.fat_tuple[i] = nullptr;
            }
        }
        state->fetch.flag = true;
        return &(state->fetch);

    } else {//thread n-1 fetches S.
        state->fetch.fat_tuple = new tuple_t *[progressive_step_tupleR];
        for (auto i = 0; i < progressive_step_tupleS; i++) {
            if (state->start_index_S < state->end_index_S) {
                state->fetch.fat_tuple[i] = &relS->tuples[state->start_index_S++];
            } else {
                state->fetch.fat_tuple[i] = nullptr;
            }
        }
        state->fetch.flag = false;
        return &(state->fetch);
    }
}

fetch_t *HS_NP_Fetcher::next_tuple(int tid) {
    if (tid == 0) {//thread 0 fetches R.
        if (state->start_index_R < state->end_index_R) {
            state->fetch.tuple = &relR->tuples[state->start_index_R++];
            state->fetch.flag = true;
            return &(state->fetch);
        } else {
            return nullptr;
        }
    } else {//thread n-1 fetches S.
        if (state->start_index_S < state->end_index_S) {
            state->fetch.tuple = &relS->tuples[state->start_index_S++];
            state->fetch.flag = false;
            return &(state->fetch);
        } else {
            return nullptr;
        }
    }
}



