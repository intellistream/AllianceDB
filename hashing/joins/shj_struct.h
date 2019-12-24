//
// Created by Shuhao Zhang on 1/11/19.
//

#ifndef ALLIANCEDB_SHJ_STRUCT_H
#define ALLIANCEDB_SHJ_STRUCT_H

#include "npj_types.h"
#include "../utils/t_timer.h"
#include "../helper/fetcher.h"
#include "../helper/shuffler.h"
#include "../helper/localjoiner.h"

/**
 * \ingroup arguments to the threads
 */

//struct list {
//    std::list<int> *relR_list;
//    std::list<int> *relS_list;
//};

/**
 * Thread-Local Structure.
 */
struct arg_t {
    int32_t tid;
    int64_t nthreads;
//    int64_t results = 0;
    int64_t *matches;
    pthread_barrier_t *barrier;

    baseFetcher *fetcher;
    baseShuffler *shuffler;
    localJoiner *joiner;

    /* results of the thread */
    threadresult_t *threadresult;

#ifndef NO_TIMING
    T_TIMER *timer;
#endif
};

struct t_param {
    int64_t result;
    result_t *joinresult;

    pthread_attr_t *attr;
    pthread_barrier_t *barrier;

    arg_t *args;
    pthread_t *tid;

    enum fetcher fetcher;
    baseShuffler *shuffler;
    enum joiner_type joiner;

    t_param(int nthreads) {
        result = 0;
        joinresult = new result_t();//(result_t *) malloc(sizeof(result_t));
        attr = new pthread_attr_t();
        barrier = new pthread_barrier_t();
        args = new arg_t[nthreads];
        tid = new pthread_t[nthreads];
    }

};

#endif //ALLIANCEDB_SHJ_STRUCT_H
