//
// Created by Shuhao Zhang on 1/11/19.
//


#include "../joins/localjoiner.h"
#include "thread_task.h"
#include "fetcher.h"
#include "shuffler.h"
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
    baseFetcher *fetcher = args->fetcher;
    int64_t matches = 0;//number of matches.

    //allocate two hashtables on each thread assuming input stream statistics are known.
    uint32_t nbucketsR = (args->fetcher->relR->num_tuples / BUCKET_SIZE);
    allocate_hashtable(&args->htR, nbucketsR);

    uint32_t nbucketsS = (args->fetcher->relS->num_tuples / BUCKET_SIZE);
    allocate_hashtable(&args->htS, nbucketsS);

    do {
        fetch_t *fetch = fetcher->next_tuple(args->tid);

        if (fetch != nullptr) {
            args->results = _shj(
                    args->tid,
                    fetch->tuple,
                    fetch->flag,
                    args->htR,
                    args->htS,
                    &matches,
                    chainedbuf, args->timer);//build and probe at the same time.
        }
    } while (!fetcher->finish(args->tid));
    printf("args->num_results (%d): %ld\n", args->tid, args->results);

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
    baseFetcher *fetcher = args->fetcher;
    int64_t matches = 0;//number of matches.

    //allocate two hashtables on each thread assuming input stream statistics are known.
    uint32_t nbucketsR = (args->fetcher->relR->num_tuples / BUCKET_SIZE);
    allocate_hashtable(&args->htR, nbucketsR);

    uint32_t nbucketsS = (args->fetcher->relS->num_tuples / BUCKET_SIZE);
    allocate_hashtable(&args->htS, nbucketsS);


    baseShuffler *shuffler = args->shuffler;


    //fetch: pointer points to state.fetch (*fetch = &(state->fetch))
    fetch_t *fetch;
    do {
        fetch = fetcher->next_tuple(args->tid);
        if (fetch != nullptr) {
            shuffler->push(fetch->tuple->key, fetch, false);//pass-in pointer points to state.fetch
        }

#ifdef EAGER
        fetch = shuffler->pull(args->tid, false);//re-fetch from its shuffler.
        if (fetch != nullptr) {
            args->results = _shj(
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
        fetch = shuffler->pull(args->tid, false);//re-fetch from its shuffler.
        if (fetch != nullptr) {
            args->results = _shj(
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
    printf("args->num_results (%d): %ld\n", args->tid, args->results);
    fflush(stdout);
}


#define LEFT true
#define RIGHT false

void clean(arg_t *arg, fetch_t *fetch, bool cleanR) {

    if (cleanR) {
        //remove ri from R-window.
        //if SHJ is used, we need to clean up hashtable of R.
        debuild_hashtable_single(arg->htR, fetch->tuple, arg->htR->hash_mask, arg->htR->skip_bits);
    } else {
        debuild_hashtable_single(arg->htS, fetch->tuple, arg->htS->hash_mask, arg->htS->skip_bits);
    }
}

/**
 *
 * @param args
 * @param fetch
 */
void
processLeft(baseShuffler *shuffler, arg_t *args, fetch_t *fetch, int64_t *matches, void *chainedbuf) {
    if (fetch->tuple) { //if msg contains a new tuple then
        assert(fetch->flag);//left must be tuple R.
        //scan S-window to find tuples that match ri ;
        //insert ri into R-window ;
        args->results = _shj(
                args->tid,
                fetch->tuple,
                fetch->flag,
                args->htR,
                args->htS,
                matches,
                chainedbuf,
                args->timer);//build and probe at the same time.
    } else if (fetch->ack) {/* msg is an acknowledgment message */
        //remove oldest tuple from S-window
        clean(args, fetch, RIGHT);
    }
}

void
processRight(baseShuffler *shuffler, arg_t *args, fetch_t *fetch, int64_t *matches, void *chainedbuf) {

    //if msg contains a new tuple then
    if (fetch->tuple) {
        assert(!fetch->flag);//right must be tuple S.
//      scan R-window to find tuples that match si ;
//      insert si into S-window ;
        args->results = _shj(
                args->tid,
                fetch->tuple,
                fetch->flag,
                args->htR,
                args->htS,
                matches,
                chainedbuf,
                args->timer);//build and probe at the same time.

        //place acknowledgment for si in rightSendQueue ;
        if (args->tid != args->nthreads - 1) {
            fetch->ack = true;
            printf("tid:%d pushes an acknowledgement of %d towards right\n", args->tid, fetch->tuple->key);
            fflush(stdout); // Will now print everything in the stdout buffer
            shuffler->push(args->tid + 1, fetch, LEFT);
        }
    }

}

void forward_tuples(baseShuffler *shuffler, arg_t *args, fetch_t *fetchR, fetch_t *fetchS) {
    //place oldest non-forwarded si into leftSendQueue ;

    if (args->tid != 0) {
        if (fetchS) {
            printf("tid:%d pushes S towards left\n", args->tid);
            fflush(stdout); // Will now print everything in the stdout buffer
            shuffler->push(args->tid - 1, fetchS, RIGHT);//push S towards left.
        }
    }
    //mark si as forwarded;
    //do nothing.

    //place oldest ri into rightSendQueue ;
    if (args->tid != args->nthreads - 1) {
        if (fetchR) {
            printf("tid:%d pushes R of %d towards right\n", args->tid, fetchR->tuple->key);
            fflush(stdout); // Will now print everything in the stdout buffer
            shuffler->push(args->tid + 1, fetchR, LEFT);//push R towards right.
        }
        //remove ri from R-window.
        clean(args, fetchR, LEFT);
    }
}


void
*THREAD_TASK_HSSHUFFLE(void *param) {
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
    int64_t matches = 0;//number of matches.

    //allocate two hashtables on each thread assuming input stream statistics are known.
    uint32_t nbucketsR = (args->fetcher->relR->num_tuples / BUCKET_SIZE);
    allocate_hashtable(&args->htR, nbucketsR);

    uint32_t nbucketsS = (args->fetcher->relS->num_tuples / BUCKET_SIZE);
    allocate_hashtable(&args->htS, nbucketsS);

    //call different data fetcher.
    baseFetcher *fetcher = args->fetcher;
    baseShuffler *shuffler = args->shuffler;

    fetch_t *fetchR;
    fetch_t *fetchS;
    do {

        //pull left queue.
        if (args->tid == 0) {
            fetchR = fetcher->next_tuple(args->tid);//
        } else {
            fetchR = shuffler->pull(args->tid, LEFT);//pull itself.
        }
        if (fetchR) {
            printf("tid:%d, fetchR:%d\n", args->tid, fetchR->tuple->key);
            fflush(stdout); // Will now print everything in the stdout buffer
            processLeft(shuffler, args, fetchR, &matches, chainedbuf);
        }

        //pull right queue.
        if (args->tid == args->nthreads - 1) {
            fetchS = fetcher->next_tuple(args->tid);//
        } else {
            fetchS = shuffler->pull(args->tid, RIGHT);//pull itself.
        }
        if (fetchS) {
            printf("tid:%d, fetchS:%d\n", args->tid, fetchS->tuple->key);
            fflush(stdout); // Will now print everything in the stdout buffer
            processRight(shuffler, args, fetchS, &matches, chainedbuf);
        }
        //forward tuple twice!
        forward_tuples(shuffler, args, fetchR, fetchS);

    } while (fetchR || fetchS);

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
    printf("args->num_results (%d): %ld\n", args->tid, args->results);
    fflush(stdout);
}



