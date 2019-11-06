//
// Created by Shuhao Zhang on 1/11/19.
//

#ifndef ALLIANCEDB_SHJ_STRUCT_H
#define ALLIANCEDB_SHJ_STRUCT_H


#include "../helper/fetcher.h"
#include "../helper/shuffler.h"

/**
 * \ingroup arguments to the threads
 */

struct list {
    std::list<int> *relR_list;
    std::list<int> *relS_list;
};

/**
 * Thread-Local Structure.
 */
struct arg_t {
    int32_t tid;
    int64_t num_results;

    hashtable_t *htR;
    hashtable_t *htS;

    //used in JB
    struct list *list;

    pthread_barrier_t *barrier;
    BaseFetcher *fetcher;
    BaseShuffler *shuffler;
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

    BaseFetcher *fetcher;
    BaseShuffler *shuffler;

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
