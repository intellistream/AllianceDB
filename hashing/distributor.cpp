//
// Created by Shuhao Zhang on 1/11/19.
//

#include <cstdio>
#include "distributor.h"


fetch_t *JB_NP_Distributor::next_tuple(int tid) {


}
//          focusing on stream input, we can't use such ``static" data distribution.
//        /* replicate relR for next thread */
//        param.args[i].relR.num_tuples = numR;
//        param.args[i].relR.tuples = relR->tuples;//configure pointer of start point.
//
//        /* assign part of the relS for next thread */
//        param.args[i].relS.num_tuples = (last_thread(i, nthreads)) ? numS : numSthr;
//        param.args[i].relS.tuples = relS->tuples + numSthr * i;
//        numS -= numSthr;





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

fetch_t *JM_NP_Distributor::next_tuple(int tid) {
    return _next_tuple(&state[tid], relR, relS);
}
