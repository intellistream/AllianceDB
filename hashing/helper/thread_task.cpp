//
// Created by Shuhao Zhang on 1/11/19.
//


#include "../joins/localjoiner.h"
#include "thread_task.h"
#include "fetcher.h"
#include "../utils/perf_counters.h"
#include "shuffler.h"

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
//    //call different data BaseFetcher.
//    JM_NP_Fetcher fetcher(0);
//    int64_t matches = 0;//number of matches.
//    do {
//        fetch_t *fetch = fetcher.next_tuple(args->tid);
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
THREAD_TASK_NOSHUFFLE(void *param) {
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

    //call different data BaseFetcher.
    BaseFetcher *fetcher = args->fetcher;
    int64_t matches = 0;//number of matches.

    //allocate two hashtables on each thread assuming input stream statistics are known.
    uint32_t nbucketsR = (args->fetcher->relR->num_tuples / BUCKET_SIZE);
    allocate_hashtable(&args->htR, nbucketsR);

    uint32_t nbucketsS = (args->fetcher->relS->num_tuples / BUCKET_SIZE);
    allocate_hashtable(&args->htS, nbucketsS);

    do {
        fetch_t *fetch = fetcher->next_tuple(args->tid);

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
    } while (!fetcher->finish(args->tid));
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


void
*THREAD_TASK_SHUFFLE(void *param) {
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

    //call different data BaseFetcher.
    BaseFetcher *fetcher = args->fetcher;
    int64_t matches = 0;//number of matches.

    //allocate two hashtables on each thread assuming input stream statistics are known.
    uint32_t nbucketsR = (args->fetcher->relR->num_tuples / BUCKET_SIZE);
    allocate_hashtable(&args->htR, nbucketsR);

    uint32_t nbucketsS = (args->fetcher->relS->num_tuples / BUCKET_SIZE);
    allocate_hashtable(&args->htS, nbucketsS);


    BaseShuffler *shuffler = args->shuffler;


    //fetch: pointer points to state.fetch (*fetch = &(state->fetch))
    fetch_t *fetch;
    do {
        fetch = fetcher->next_tuple(args->tid);
        if (fetch != nullptr) {
            shuffler->push(fetch->tuple->key, fetch);//pass-in pointer points to state.fetch
        }

#ifdef EAGER
        fetch = shuffler->pull(args->tid);//re-fetch from its shuffler.
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
#endif
    } while (!fetcher->finish(args->tid));

    /* wait at a barrier until each thread finishes fetch*/
    BARRIER_ARRIVE(args->barrier, rv)

    do {
        fetch = shuffler->pull(args->tid);//re-fetch from its shuffler.
        if (fetch != nullptr) {
            args->num_results = _shj(
                    args->tid,
                    fetch->tuple,
                    fetch->flag,
                    args->htR,
                    args->htS,
                    &matches,
                    chainedbuf, args->timer);//build and probe at the same time.
        } else break;
    } while (true);

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
    printf("args->num_results (%d): %ld\n", args->tid, args->num_results);
    fflush(stdout);
}
