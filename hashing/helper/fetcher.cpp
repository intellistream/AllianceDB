//
// Created by Shuhao Zhang on 1/11/19.
//

#include <cstdio>
#include <assert.h>
#include "fetcher.h"
#include "pmj_helper.h"

fetch_t::fetch_t(fetch_t *fetch) {
    this->fat_tuple = fetch->fat_tuple;
    this->fat_tuple_size = fetch->fat_tuple_size;
    this->tuple = fetch->tuple;
    this->ISTuple_R = fetch->ISTuple_R;
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
    if (state->IsTupleR) {
        if (state->start_index_R < state->end_index_R) {
            state->fetch.tuple = &relR->tuples[state->start_index_R++];
            state->fetch.ISTuple_R = state->IsTupleR;
            (state->IsTupleR) ^= true;//flip flag
            return &(state->fetch);
        } else {
            (state->IsTupleR) ^= true;//flip flag
            return nullptr;
        }
    } else {
        if (state->start_index_S < state->end_index_S) {
            state->fetch.tuple = &relS->tuples[state->start_index_S++];
            state->fetch.ISTuple_R = state->IsTupleR;
            (state->IsTupleR) ^= true;//flip flag
            return &(state->fetch);
        } else {
            state->IsTupleR ^= true;//flip flag
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
    if (state->IsTupleR) {
        if (state->start_index_R < state->end_index_R) {
            state->fetch.tuple = new tuple_t();
            state->fetch.tuple->payloadID = relR->tuples[state->start_index_R].payloadID;
            state->fetch.tuple->key = relR->tuples[state->start_index_R++].key;
            state->fetch.ISTuple_R = state->IsTupleR;
            (state->IsTupleR) ^= true;//flip flag
            return &(state->fetch);
        } else {
            (state->IsTupleR) ^= true;//flip flag
            return nullptr;
        }
    } else {
        if (state->start_index_S < state->end_index_S) {
            state->fetch.tuple = new tuple_t();
            state->fetch.tuple->payloadID = relS->tuples[state->start_index_S].payloadID;
            state->fetch.tuple->key = relS->tuples[state->start_index_S++].key;
            state->fetch.ISTuple_R = state->IsTupleR;
            (state->IsTupleR) ^= true;//flip flag
            return &(state->fetch);
        } else {
            state->IsTupleR ^= true;//flip flag
            return nullptr;
        }
    }
}

fetch_t *JM_NP_Fetcher::next_tuple(int tid) {
    milliseconds min_gap = (milliseconds) INT32_MAX;
    tuple_t *readR = nullptr;
    tuple_t *readS;

    //try to read R first.
    if (state->start_index_R < state->end_index_R) {
        readR = &relR->tuples[state->start_index_R];
        //check the timestamp whether the tuple is ``ready" to be fetched.
        std::chrono::milliseconds timestamp = relR->payload->ts[readR->payloadID];
        auto timegap = RtimeGap(&timestamp);
        if (timegap.count() <= 0) {//if it's negative means our fetch is too slow.
            state->fetch.tuple = readR;
            state->fetch.ISTuple_R = true;
            state->start_index_R++;
            return &(state->fetch);
        } else {
            min_gap = timegap;
        }
    }

    //try to read S then.
    if (state->start_index_S < state->end_index_S) {
        readS = &relS->tuples[state->start_index_S];
        //check the timestamp whether the tuple is ``ready" to be fetched.
        std::chrono::milliseconds timestamp = relS->payload->ts[readS->payloadID];
        auto timegap = StimeGap(&timestamp);
        if (timegap.count() <= 0) {//if it's negative means our fetch is too slow.
            state->fetch.tuple = readS;
            state->fetch.ISTuple_R = false;
            state->start_index_S++;
            return &(state->fetch);
        } else {  //return the nearest tuple.
            if (min_gap > timegap) {//S is nearest.
                min_gap = timegap;
//                sleep(min_gap.count());
                this_thread::sleep_for(min_gap);
                state->fetch.tuple = readS;
                state->fetch.ISTuple_R = false;
                state->start_index_S++;
                return &(state->fetch);
            } else if (readR != nullptr) {//R is nearest.
//                sleep(min_gap.count());
                this_thread::sleep_for(min_gap);
                state->fetch.tuple = readR;
                state->fetch.ISTuple_R = true;
                state->start_index_R++;
                return &(state->fetch);
            }
        }
    }
    return nullptr;
//
//    fetch_t *rt = _next_tuple(state, relR, relS);
//
//    if (rt != nullptr) {
//        if (rt->ISTuple_R) {
//            auto timestamp = relR->payload[rt->tuple->payloadID].ts;
//            this->Rproceed(timestamp);
//        } else {
//            auto timestamp = relS->payload[rt->tuple->payloadID].ts;
//            this->Sproceed(timestamp);
//        }
//        return rt;
//    }
//
//    return nullptr;
}

fetch_t *JM_P_Fetcher::next_tuple(int tid) {
//not implemented.
}

fetch_t *JB_NP_Fetcher::next_tuple(int tid) {
    fetch_t *rt = _next_tuple(state, relR, relS);

    if (rt != nullptr) {
        if (rt->ISTuple_R) {
            auto timestamp = relR->payload[rt->tuple->payloadID].ts;
            this->Rproceed(timestamp);
        } else {
            auto timestamp = relS->payload[rt->tuple->payloadID].ts;
            this->Sproceed(timestamp);
        }
        return rt;
    }

    return nullptr;
}

fetch_t *_next_tuple_PMJ_HS(int tid, t_state *state, relation_t *relR, relation_t *relS) {
    if (tid == 0) {//thread 0 fetches R.
        if (state->start_index_R + progressive_step_tupleR < state->end_index_R) {
            state->fetch.fat_tuple_size = progressive_step_tupleR;
//            state->fetch.fat_tuple = new tuple_t[state->fetch.fat_tuple_size];
//            copy_tuple(state->fetch.fat_tuple, &relR->tuples[state->start_index_R], state->fetch.fat_tuple_size);
            state->fetch.fat_tuple = &relR->tuples[state->start_index_R];
        } else if (state->end_index_R - state->start_index_R > 0) {
            state->fetch.fat_tuple_size = state->end_index_R - state->start_index_R;//left-over..
            state->fetch.fat_tuple = &relR->tuples[state->start_index_R];
        } else {
            return nullptr;//nothing left-over.
        }
        state->start_index_R += state->fetch.fat_tuple_size;
        state->fetch.ISTuple_R = true;
        return &(state->fetch);
    } else {//thread n-1 fetches S.
        if (state->start_index_S + progressive_step_tupleS < state->end_index_S) {
            state->fetch.fat_tuple_size = progressive_step_tupleS;
            state->fetch.fat_tuple = &relS->tuples[state->start_index_S];
//            state->fetch.fat_tuple = new tuple_t[state->fetch.fat_tuple_size];
//            copy_tuple( state->fetch.fat_tuple, &relS->tuples[state->start_index_S], state->fetch.fat_tuple_size);
        } else if (state->end_index_S - state->start_index_S > 0) {
            state->fetch.fat_tuple_size = state->end_index_S - state->start_index_S;//left-over..
            state->fetch.fat_tuple = &relS->tuples[state->start_index_S];
        } else {
            return nullptr;
        }
        state->start_index_S += state->fetch.fat_tuple_size;

        state->fetch.ISTuple_R = false;
        return &(state->fetch);
    }
}


fetch_t *PMJ_HS_NP_Fetcher::next_tuple(int tid) {
    fetch_t *rt = _next_tuple_PMJ_HS(tid, state, relR, relS);

    if (rt != nullptr) {
        if (rt->ISTuple_R) {
            auto timestamp = relR->payload[rt->tuple->payloadID].ts;
            this->Rproceed(timestamp);
        } else {
            auto timestamp = relS->payload[rt->tuple->payloadID].ts;
            this->Sproceed(timestamp);
        }
        return rt;
    }

    return nullptr;

}

/**
 *
 * @param tid
 * @param state
 * @param relR
 * @param relS
 * @return
 */
fetch_t *_next_tuple_HS(int tid, t_state *state, relation_t *relR, relation_t *relS) {
    if (tid == 0) {//thread 0 fetches R.
        if (state->start_index_R < state->end_index_R) {
            state->fetch.tuple = &relR->tuples[state->start_index_R++];
            state->fetch.ISTuple_R = true;
            return &(state->fetch);
        } else {
            return nullptr;
        }
    } else {//thread n-1 fetches S.
        if (state->start_index_S < state->end_index_S) {
            state->fetch.tuple = &relS->tuples[state->start_index_S++];
            state->fetch.ISTuple_R = false;
            return &(state->fetch);
        } else {
            return nullptr;
        }
    }
}

/**
 * actually this tid is not needed. We put it here for program convenience only.
 * @param tid
 * @return
 */
fetch_t *HS_NP_Fetcher::next_tuple(int tid) {
    fetch_t *rt = _next_tuple_HS(tid, state, relR, relS);

    if (rt != nullptr) {
        if (rt->ISTuple_R) {
            auto timestamp = relR->payload[rt->tuple->payloadID].ts;
            this->Rproceed(timestamp);
        } else {
            auto timestamp = relS->payload[rt->tuple->payloadID].ts;
            this->Sproceed(timestamp);
        }
        return rt;
    }

    return nullptr;
}



