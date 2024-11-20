//
// Created by Shuhao Zhang on 1/11/19.
//

#ifndef ALLIANCEDB_EAGERJOIN_STRUCT_H
#define ALLIANCEDB_EAGERJOIN_STRUCT_H

#include "npj_types.h"
#include "../timer/t_timer.h"
#include "../helper/fetcher.h"
#include "../helper/shuffler.h"
#include "../helper/localjoiner.h"

/**
 * \ingroup arguments to the threads
 */

/**
 * Thread-Local Structure.
 */
struct arg_t {
    int32_t tid;
    int64_t nthreads;
//    int64_t results = 0;
    int64_t *matches;
    pthread_barrier_t *barrier;

    // legacy fetcher, shuffler and joiner of the framework
    baseFetcher *fetcher;
    baseShuffler *shuffler;
    baseJoiner *joiner;

    // special fields for fetching and shuffling
    relation_t* relation_left;
    relation_t* relation_right;
    SHJShuffleQueueGroup* left_group_shared_ptr;
    SHJShuffleQueueGroup* right_group_shared_ptr;



    /* results of the thread */
    threadresult_t *threadresult;

//#ifndef NO_TIMING
    T_TIMER *timer;
    uint64_t *startTS;
//#endif

    int exp_id; // for perf stat
};

struct t_param {
    string algo_name;

    int exp_id;
    int record_gap;

    int64_t result;
    result_t *joinresult;


    pthread_attr_t *attr;
    pthread_barrier_t *barrier;

    arg_t *args;
    pthread_t *tid;

    enum fetcher fetcher;
    baseShuffler *shuffler;
    enum joiner_type joiner;


    //parameters of PMJ
    int progressive_step = 1;//percentile of tuples to sort at each iteration. It must be multiple cacheline size (64).
    int merge_step = 2;//#runs to merge at each iteration.

    t_param(int nthreads) {
        result = 0;
        joinresult = new result_t();//(result_t *) malloc(sizeof(result_t));
        attr = new pthread_attr_t();
        barrier = new pthread_barrier_t();
        args = new arg_t[nthreads];
        tid = new pthread_t[nthreads];
        for (auto i = 0; i < nthreads; i++) {
            tid[i] = -1;
        }
    }



};

#endif //ALLIANCEDB_EAGERJOIN_STRUCT_H
