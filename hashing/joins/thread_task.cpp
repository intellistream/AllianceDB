//
// Created by Shuhao Zhang on 1/11/19.
//


#include "localjoiner.h"
#include "thread_task.h"
#include "../distributor.h"
#include "../utils/perf_counters.h"

/**
 * Just a wrapper to call the _shj_st
 *
 * @param param the parameters of the thread, i.e. tid, ht, reln, ...
 *
 * @return
 */
void *
shj_thread_jb_np(void *param) {
//    int rv;
//    arg_t *args = (arg_t *) param;
//
//#ifdef PERF_COUNTERS
//    if(args->tid == 0){
//        PCM_initPerformanceMonitor(NULL, NULL);
//        PCM_start();
//    }
//#endif
////
////    /* wait at a barrier until each thread starts and start T_TIMER */
////    BARRIER_ARRIVE(args->barrier, rv);
//
//#ifndef NO_TIMING
//    /* the first thread checkpoints the start time */
//    if (args->tid == 0) {
//        START_MEASURE((*(args->timer)))
//    }
//#endif
//
//    //allocate two hashtables.
//    uint32_t nbucketsR = (args->relR.num_tuples / BUCKET_SIZE);
//    allocate_hashtable(&args->htR, nbucketsR);
//
//    uint32_t nbucketsS = (args->relS.num_tuples / BUCKET_SIZE);
//    allocate_hashtable(&args->htS, nbucketsS);
//
//#ifdef PERF_COUNTERS
//    if(args->tid == 0){
//      PCM_stop();
//      PCM_log("========== Build phase profiling results ==========\n");
//      PCM_printResults();
//      PCM_start();
//    }
//    /* Just to make sure we get consistent performance numbers */
//    BARRIER_ARRIVE(args->barrier, rv);
//#endif
//
//#ifdef JOIN_RESULT_MATERIALIZE
//    chainedtuplebuffer_t * chainedbuf = chainedtuplebuffer_init();
//#else
//    void *chainedbuf = NULL;
//#endif
//    //call different data Base_Distributor.
//    JM_NP_Distributor distributor(0);
//    int64_t matches = 0;//number of matches.
//    do {
//        fetch_t *fetch = distributor.next_tuple(args->tid);
//        if (fetch == nullptr) break;
//        else {
//            args->num_results = _shj(
//                    args->tid,
//                    fetch->tuple,
//                    fetch->flag,
//                    args->htR, args->htS,
//                    &matches, chainedbuf, args->timer);//build and probe at the same time.
//        }
//    } while (true);
//    printf("args->num_results (1): %ld\n", args->num_results);
//
//#ifdef JOIN_RESULT_MATERIALIZE
//    args->threadresult->nresults = args->num_results;
//    args->threadresult->threadid = args->tid;
//    args->threadresult->results  = (void *) chainedbuf;
//#endif
//
//#ifndef NO_TIMING
//    if (args->tid == 0) {
//        END_MEASURE((*(args->timer)))
//    }
//#endif
//
//#ifdef PERF_COUNTERS
//    if(args->tid == 0) {
//        PCM_stop();
//        PCM_log("========== Probe phase profiling results ==========\n");
//        PCM_printResults();
//        PCM_log("===================================================\n");
//        PCM_cleanup();
//    }
//    /* Just to make sure we get consistent performance numbers */
//    BARRIER_ARRIVE(args->barrier, rv);
//#endif
//    return 0;
}


/**
 * Just a wrapper to call the _shj_st
 *
 * @param param the parameters of the thread, i.e. tid, ht, reln, ...
 *
 * @return
 */
void *
thread_task(void *param) {
    arg_t *args = (arg_t *) param;
#ifdef PERF_COUNTERS
    if (args->tid == 0) {
        PCM_initPerformanceMonitor(NULL, NULL);
        PCM_start();
    }
#endif

#ifndef NO_TIMING
    /* the first thread checkpoints the start time */
    if (args->tid == 0) {
        START_MEASURE((*(args->timer)))
    }
#endif

    int rv;
#ifdef PERF_COUNTERS
    if (args->tid == 0) {
        PCM_stop();
        PCM_log("========== Build phase profiling results ==========\n");
        PCM_printResults();
        PCM_start();
    }
    /* Just to make sure we get consistent performance numbers */
    BARRIER_ARRIVE(args->barrier, rv);
#endif

#ifdef JOIN_RESULT_MATERIALIZE
    chainedtuplebuffer_t * chainedbuf = chainedtuplebuffer_init();
#else
    void *chainedbuf = NULL;
#endif

    //call different data Base_Distributor.
    Base_Distributor *distributor = args->distributor;
    int64_t matches = 0;//number of matches.

    //allocate two hashtables on each thread assuming input stream statistics are known.
    uint32_t nbucketsR = (args->distributor->relR->num_tuples / BUCKET_SIZE);
    allocate_hashtable(&args->htR, nbucketsR);

    uint32_t nbucketsS = (args->distributor->relS->num_tuples / BUCKET_SIZE);
    allocate_hashtable(&args->htS, nbucketsS);

    do {
        fetch_t *fetch = distributor->next_tuple(args->tid);
        if (fetch != nullptr) {
            args->num_results = _shj(
                    args->tid,
                    fetch->tuple,
                    fetch->flag,
                    args->htR,
                    args->htS,
                    &matches,
                    chainedbuf, args->timer);//build and probe at the same time.
        }
    } while (!distributor->finish(args->tid));
    printf("args->num_results (%d): %ld\n", args->tid, args->num_results);

#ifdef JOIN_RESULT_MATERIALIZE
    args->threadresult->nresults = args->num_results;
    args->threadresult->threadid = args->tid;
    args->threadresult->results  = (void *) chainedbuf;
#endif

#ifndef NO_TIMING
    if (args->tid == 0) {
        END_MEASURE((*(args->timer)))
    }
#endif

#ifdef PERF_COUNTERS
    if (args->tid == 0) {
        PCM_stop();
        PCM_log("========== Probe phase profiling results ==========\n");
        PCM_printResults();
        PCM_log("===================================================\n");
        PCM_cleanup();
    }
    /* Just to make sure we get consistent performance numbers */
    BARRIER_ARRIVE(args->barrier, rv);
#endif
    return 0;
}
