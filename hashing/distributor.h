//
// Created by Shuhao Zhang on 1/11/19.
//

#ifndef ALLIANCEDB_DISTRIBUTOR_H
#define ALLIANCEDB_DISTRIBUTOR_H

#include "utils/types.h"


struct fetch_t {
    tuple_t *tuple;
    bool flag;

};

class Base_Distributor {
public:
    virtual fetch_t *next_tuple(int tid) = 0;

    relation_t *relR;//input relation
    relation_t *relS;//input relation
    virtual bool finish(int32_t tid) = 0;

    Base_Distributor(relation_t *relR, relation_t *relS) {
        this->relR = relR;
        this->relS = relS;
    }


};

inline bool last_thread(int i, int nthreads) {
    return i == (nthreads - 1);
}

//thread local structure
struct t_state {
    int start_index_R;//configure pointer of start reading point.
    int end_index_R;//configure pointer of end reading point.
    int start_index_S;//configure pointer of start reading point.
    int end_index_S;//configure pointer of end reading point.
    //read R/S alternatively.
    bool flag;
    fetch_t fetch;
};

class JM_NP_Distributor : public Base_Distributor {
public:
    fetch_t *next_tuple(int tid) override;

    t_state *state;

    bool finish(int tid) {
        return state[tid].start_index_R == state[tid].end_index_R
               && state[tid].start_index_S == state[tid].end_index_S;
    }

    JM_NP_Distributor(int nthreads, relation_t *relR, relation_t *relS)
            : Base_Distributor(relR, relS) {
        state = new t_state[nthreads];

        int numSthr = relS->num_tuples / nthreads;//replicate R, partition S.
        for (int i = 0; i < nthreads; i++) {

            state[i].flag = true;
            /* replicate relR for next thread */
            state[i].start_index_R = 0;
            state[i].end_index_R = relR->num_tuples;

            /* assign part of the relS for next thread */
            state[i].start_index_S = numSthr * i;
            state[i].end_index_S = (last_thread(i, nthreads)) ? relS->num_tuples : numSthr * (i + 1);


            printf("TID:%d, R: start_index:%d, end_index:%d\n", i, state[i].start_index_R, state[i].end_index_R);
            printf("TID:%d, S: start_index:%d, end_index:%d\n", i, state[i].start_index_S, state[i].end_index_S);
        }
    }


};

class JB_NP_Distributor : public Base_Distributor {
public:
    fetch_t *next_tuple(int tid) override;

    bool finish(int tid) {


    }

    JB_NP_Distributor(int nthreads, relation_t *relR, relation_t *relS)
            : Base_Distributor(relR, relS) {

    }
};

#endif //ALLIANCEDB_DISTRIBUTOR_H
