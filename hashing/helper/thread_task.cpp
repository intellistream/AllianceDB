//
// Created by Shuhao Zhang on 1/11/19.
//

#include "thread_task.h"
#include "../joins/eagerjoin_struct.h"
#include "fetcher.h"
#include "localjoiner.h"
#include "shuffler.h"
#include <execinfo.h>
#include "../joins/batcher.h"
#define JOIN
/**
 * a JOIN function that a joiner should apply
 * @param r_tuple
 * @param s_tuple
 * @param matches
 * @return
 */
// void* JOINFUNCTION(tuple_t *r_tuple, tuple_t *s_tuple, int64_t *matches) {
#ifdef AGG_FUNCTION
void *AGGFUNCTION(const tuple_t *r_tuple, const tuple_t *s_tuple,
                  int64_t *matches) {
  if (r_tuple->key == s_tuple->key) {
    //        (*matches)++;
    DEBUGMSG("matches: r:%d with s:%d", r_tuple->key, s_tuple->key);
  }
}
#endif

/**
 * Just a wrapper to call the _shj_st
 *
 * @param param the parameters of the thread, i.e. tid, ht, reln, ...
 *
 * @return
 */
void *THREAD_TASK_NOSHUFFLE(void *param) {
    arg_t *args = (arg_t *) param;
    int lock;

#ifdef PROFILE_TOPDOWN
#ifdef JOIN_THREAD
    // do nothing
#else
    return nullptr;
#endif
#endif

#ifdef JOIN_RESULT_MATERIALIZE
    chainedtuplebuffer_t *chainedbuf = chainedtuplebuffer_init();
#else
    void *chainedbuf = NULL;
#endif
    // call different data BaseFetcher.
    baseFetcher *fetcher = args->fetcher;
    /* wait at a barrier until each thread started*/
    BARRIER_ARRIVE(args->barrier, lock)
    *args->startTS = curtick();
#ifndef NO_TIMING
    //        MSG(" *args->startTS :%lu\n", *args->startTS);
    START_MEASURE((args->timer))
#endif

    fetcher->fetchStartTime = args->startTS; // set the fetch starting time.
#ifdef PERF_COUNTERS
    if (args->tid == 0) {
        DEBUGMSG("Thread:%id initialize PCM", args->tid)
        PCM_initPerformanceMonitor(NULL, NULL);
        PCM_start();
        auto curtime = std::chrono::steady_clock::now();
        string path = EXP_DIR "/results/breakdown/time_start_" + std::to_string(args->exp_id) + ".txt";
        auto fp = fopen(path.c_str(), "w");
        fprintf(fp, "%ld\n", curtime);
        DEBUGMSG("Thread:%id initialized PCM", args->tid)
    }
    BARRIER_ARRIVE(args->barrier, lock)
#endif
    do {
        fetch_t *fetch = fetcher->next_tuple(); /*time to fetch, waiting time*/

        //batching

        if (fetch != nullptr) {
#ifdef JOIN
            args->joiner->join(/*time to join for one tuple*/
                    args->tid, fetch->tuple, fetch->ISTuple_R,
                    args->matches,
                    //                    AGGFUNCTION,
                    chainedbuf); // build and probe at the same time.
#endif
        }
    } while (!fetcher->finish());

    //    BEGIN_GARBAGE(args->timer)
    /* wait at a barrier until each thread finishes*/
    BARRIER_ARRIVE(args->barrier, lock)
    //    END_GARBAGE(args->timer)

#ifdef JOIN
    // only PMJ needs this.
    args->joiner->merge(args->tid, args->matches,
            //            AGGFUNCTION,
                        chainedbuf);
#endif

    DEBUGMSG("args->num_results (%d): %ld\n", args->tid, *args->matches);
#ifdef JOIN_RESULT_MATERIALIZE
    args->threadresult->nresults = *args->matches;
    args->threadresult->threadid = args->tid;
    args->threadresult->results = (void *)chainedbuf;
#endif

#ifdef PERF_COUNTERS
    if (args->tid == 0) {
        MSG("Thread:%id stops PCM", args->tid)
        PCM_stop();
        auto curtime = std::chrono::steady_clock::now();
        string path = EXP_DIR "/results/breakdown/time_end_" + std::to_string(args->exp_id) + ".txt";
        auto fp = fopen(path.c_str(), "w");
        fprintf(fp, "%ld\n", curtime);
        PCM_log("========== Entire phase profiling results ==========\n");
        PCM_printResults();
        PCM_log("===================================================\n");
        PCM_cleanup();
    }
#endif

    /* wait at a barrier until each thread finishes*/
    BARRIER_ARRIVE(args->barrier, lock)

#ifndef NO_TIMING
        END_MEASURE(args->timer)
    // time calibration
    //    args->timer->overall_timer -= args->timer->garbage_time;
    args->timer->partition_timer =
            args->timer->overall_timer - args->timer->wait_timer;
#endif

    DEBUGMSG("args->num_results (%d): %ld\n", args->tid, *args->matches);
    fflush(stdout);
    pthread_exit(NULL);
}

void *THREAD_TASK_NOSHUFFLE_BATCHED(void *param) {
    MSG("Using Batched Task")
    arg_t *args = (arg_t *) param;
    int lock;

#ifdef PROFILE_TOPDOWN
    #ifdef JOIN_THREAD
    // do nothing
#else
    return nullptr;
#endif
#endif

#ifdef JOIN_RESULT_MATERIALIZE
    chainedtuplebuffer_t *chainedbuf = chainedtuplebuffer_init();
#else
    void *chainedbuf = NULL;
#endif
    // call different data BaseFetcher.
    baseFetcher *fetcher = args->fetcher;
    /* wait at a barrier until each thread started*/
    BARRIER_ARRIVE(args->barrier, lock)
    *args->startTS = curtick();
#ifndef NO_TIMING
    //        MSG(" *args->startTS :%lu\n", *args->startTS);
    START_MEASURE((args->timer))
#endif

    fetcher->fetchStartTime = args->startTS; // set the fetch starting time.
#ifdef PERF_COUNTERS
    if (args->tid == 0) {
        DEBUGMSG("Thread:%id initialize PCM", args->tid)
        PCM_initPerformanceMonitor(NULL, NULL);
        PCM_start();
        auto curtime = std::chrono::steady_clock::now();
        string path = EXP_DIR "/results/breakdown/time_start_" + std::to_string(args->exp_id) + ".txt";
        auto fp = fopen(path.c_str(), "w");
        fprintf(fp, "%ld\n", curtime);
        DEBUGMSG("Thread:%id initialized PCM", args->tid)
    }
    BARRIER_ARRIVE(args->barrier, lock)
#endif
    Batch batch_r;
    Batch batch_s;
    do {
        fetch_t *fetch = fetcher->next_tuple(); /*time to fetch, waiting time*/
        if(fetch->ISTuple_R){
            // the arrival of new r tuple form a complete batch
            if (batch_r.add_tuple(fetch->tuple)) {
#ifdef JOIN
                args->joiner->join_batched(/*time to join for one tuple*/
                        args->tid, &batch_r, fetch->ISTuple_R,
                        args->matches,
                        //                    AGGFUNCTION,
                        chainedbuf);
                batch_r.reset();
#endif
            }
        }
        else{
            if(batch_s.add_tuple(fetch->tuple)){
                args->joiner->join_batched(/*time to join for one tuple*/
                        args->tid, &batch_s, fetch->ISTuple_R,
                        args->matches,
                        //                    AGGFUNCTION,
                        chainedbuf);
                batch_s.reset();
            }
        }
    } while (!fetcher->finish());
    // join the remaining none-complete batch
    args->joiner->join_batched(
            args->tid, &batch_r, true,
            args->matches,
            //                    AGGFUNCTION,
            chainedbuf);
    args->joiner->join_batched(
            args->tid, &batch_s, false,
            args->matches,
            chainedbuf);
    batch_s.reset();
    batch_r.reset();
    MSG("Thread %d: batch_s cnt: %d, batch_r cnt: %d",args->tid,batch_s.batch_cnt(),batch_r.batch_cnt())

    //    BEGIN_GARBAGE(args->timer)
    /* wait at a barrier until each thread finishes*/
    BARRIER_ARRIVE(args->barrier, lock)
    //    END_GARBAGE(args->timer)

#ifdef JOIN
    // only PMJ needs this.
    args->joiner->merge(args->tid, args->matches,
            //            AGGFUNCTION,
                        chainedbuf);
#endif

    DEBUGMSG("args->num_results (%d): %ld\n", args->tid, *args->matches);
#ifdef JOIN_RESULT_MATERIALIZE
    args->threadresult->nresults = *args->matches;
    args->threadresult->threadid = args->tid;
    args->threadresult->results = (void *)chainedbuf;
#endif

#ifdef PERF_COUNTERS
    if (args->tid == 0) {
        MSG("Thread:%id stops PCM", args->tid)
        PCM_stop();
        auto curtime = std::chrono::steady_clock::now();
        string path = EXP_DIR "/results/breakdown/time_end_" + std::to_string(args->exp_id) + ".txt";
        auto fp = fopen(path.c_str(), "w");
        fprintf(fp, "%ld\n", curtime);
        PCM_log("========== Entire phase profiling results ==========\n");
        PCM_printResults();
        PCM_log("===================================================\n");
        PCM_cleanup();
    }
#endif

    /* wait at a barrier until each thread finishes*/
    BARRIER_ARRIVE(args->barrier, lock)

#ifndef NO_TIMING
    END_MEASURE(args->timer)
    // time calibration
    //    args->timer->overall_timer -= args->timer->garbage_time;
    args->timer->partition_timer =
            args->timer->overall_timer - args->timer->wait_timer;
#endif

    DEBUGMSG("args->num_results (%d): %ld\n", args->tid, *args->matches);
    fflush(stdout);
    pthread_exit(NULL);
}

void *THREAD_TASK_SHUFFLE(void *param) {
    arg_t *args = (arg_t *) param;
    int lock;

#ifdef PROFILE_TOPDOWN
#ifdef JOIN_THREAD
    // do nothing
#else
    return nullptr;
#endif
#endif

#ifdef JOIN_RESULT_MATERIALIZE
    chainedtuplebuffer_t *chainedbuf = chainedtuplebuffer_init();
#else
    void *chainedbuf = NULL;
#endif
    // call different BaseFetcher.
    baseFetcher *fetcher = args->fetcher;
    baseShuffler *shuffler = args->shuffler;
    // fetch: pointer points to state.fetch (*fetch = &(state->fetch))
    fetch_t *fetch;
    /* wait at a barrier until each thread started*/
    BARRIER_ARRIVE(args->barrier, lock)
    *args->startTS = curtick();
#ifndef NO_TIMING
    START_MEASURE((args->timer))
#endif

    fetcher->fetchStartTime = args->startTS; // set the fetch starting time.
#ifdef PERF_COUNTERS
    if (args->tid == 0) {
        MSG("Thread:%id initialize PCM", args->tid)
        PCM_initPerformanceMonitor(NULL, NULL);
        PCM_start();
        auto curtime = std::chrono::steady_clock::now();
        string path = EXP_DIR "/results/breakdown/time_start_" + std::to_string(args->exp_id) + ".txt";
        auto fp = fopen(path.c_str(), "w");
        fprintf(fp, "%ld\n", curtime);
        MSG("Thread:%id initialized PCM", args->tid)
    }
    BARRIER_ARRIVE(args->barrier, lock)
#endif
    do {
        fetch = fetcher->next_tuple();
        if (fetch != nullptr) {
            shuffler->push(fetch->tuple->key, fetch,
                           false); // pass-in pointer points to state.fetch
        }
#ifdef EAGER
        fetch = shuffler->pull(args->tid, false); // re-fetch from its shuffler.
        if (fetch != nullptr) {
#ifdef JOIN
            args->joiner->join(args->tid, fetch->tuple, fetch->ISTuple_R,
                               args->matches,
                    //                    AGGFUNCTION,
                               chainedbuf); // build and probe at the same time.
#endif
        }
#endif
    } while (!fetcher->finish());

    //    BEGIN_GARBAGE(args->timer)
    /* wait at a barrier until each thread finishes fetch*/
    BARRIER_ARRIVE(args->barrier, lock)
    //    END_GARBAGE(args->timer)

    do {
        fetch = shuffler->pull(args->tid, false); // re-fetch from its shuffler.
        if (fetch != nullptr) {
#ifdef JOIN
            args->joiner->join(args->tid, fetch->tuple, fetch->ISTuple_R,
                               args->matches,
                    //                    AGGFUNCTION,
                               chainedbuf);
#endif
        }
    } while (fetch != nullptr);

#ifdef JOIN
    // only PMJ needs this.
    args->joiner->merge( // merge the left-over.
            args->tid, args->matches,
            //                    AGGFUNCTION,
            chainedbuf);
#endif

#ifdef JOIN_RESULT_MATERIALIZE
    args->threadresult->nresults = *args->matches;
    args->threadresult->threadid = args->tid;
    args->threadresult->results = (void *)chainedbuf;
#endif
#ifdef PERF_COUNTERS
    if (args->tid == 0) {
        MSG("Thread:%id stops PCM", args->tid)
        PCM_stop();
        auto curtime = std::chrono::steady_clock::now();
        string path = EXP_DIR "/results/breakdown/time_end_" + std::to_string(args->exp_id) + ".txt";
        auto fp = fopen(path.c_str(), "w");
        fprintf(fp, "%ld\n", curtime);
        PCM_log("========== Entire phase profiling results ==========\n");
        PCM_printResults();
        PCM_log("===================================================\n");
        PCM_cleanup();
    }
#endif
    /* wait at a barrier until each thread finishes*/
    BARRIER_ARRIVE(args->barrier, lock)
#ifndef NO_TIMING
        END_MEASURE(args->timer)
    // time calibration
    //    args->timer->overall_timer -= args->timer->garbage_time;
    args->timer->partition_timer =
            args->timer->overall_timer - args->timer->wait_timer;
#endif

    DEBUGMSG("args->num_results (%d): %ld\n", args->tid, *args->matches);
    fflush(stdout);
    pthread_exit(NULL);
}

#define LEFT true
#define RIGHT false

/**
 *
 * @param args
 * @param fetch
 */
void processLeft(arg_t *args, fetch_t *fetch, int64_t *matches,
                 void *chainedbuf) {
    if (fetch->ack) { /* msg is an acknowledgment message */
        // remove oldest tuple from S-window
        DEBUGMSG("TID %d: remove s %d from S-window because of ack.", args->tid,
                 fetch->tuple->key)
#ifdef JOIN
        args->joiner->clean(args->tid, fetch->tuple, RIGHT);
#endif
    } else if (fetch->tuple) { // if msg contains a new tuple then
#ifdef DEBUG

        if (!fetch->ISTuple_R) // right must be tuple R.
        {
          DEBUGMSG("right must be tuple R. something is wrong \n");
        }
#else
        assert(fetch->ISTuple_R); // left must be tuple R.
#endif
        // scan S-window to find tuples that match ri ;
        // insert ri into R-window ;
#ifdef JOIN
        args->joiner->join(args->tid, fetch->tuple, fetch->ISTuple_R, matches,
                //                AGGFUNCTION,
                           chainedbuf);
#endif
    }
}

/**
 *
 * @param args
 * @param fetch
 */
void processLeft_PMJ(arg_t *args, fetch_t *fetch, int64_t *matches,
                     void *chainedbuf) {
    if (fetch->ack) { /* msg is an acknowledgment message */
        // remove oldest tuple from S-window
        DEBUGMSG("TID %d:remove S %s from S-window.", args->tid,
                 print_tuples(fetch->fat_tuple, fetch->fat_tuple_size).c_str())
#ifdef JOIN
        args->joiner->clean(args->tid, fetch->fat_tuple, fetch->fat_tuple_size,
                            RIGHT);
#endif
    } else if (fetch->fat_tuple) { // if msg contains a new tuple then
#ifdef DEBUG
        if (!fetch->ISTuple_R) // right must be tuple R.
        {
          DEBUGMSG("right must be tuple R, something is wrong \n");
        }
#else
        assert(fetch->ISTuple_R); // left must be tuple R.
#endif
        // scan S-window to find tuples that match ri ;
        // insert ri into R-window ;
#ifdef JOIN
        args->joiner->join(args->tid, fetch->fat_tuple, fetch->fat_tuple_size,
                           fetch->ISTuple_R, matches,
                //                AGGFUNCTION,
                           chainedbuf);
#endif
    }
}

void processRight(baseShuffler *shuffler, arg_t *args, fetch_t *fetch,
                  int64_t *matches, void *chainedbuf) {

    // if msg contains a new tuple then
    if (fetch->tuple) {
#ifdef DEBUG
        if (fetch->ISTuple_R) // right must be tuple S.
        {
          MSG("tid:%d processRIGHT: something is wrong, %d \n", args->tid,
              fetch->tuple->key);
          fflush(stdout);
        }
#else
        assert(!fetch->ISTuple_R);
#endif

        //      scan R-window to find tuples that match si ;
        //      insert si into S-window ;
        args->joiner->join(args->tid, fetch->tuple, fetch->ISTuple_R, matches,
                //                AGGFUNCTION,
                           chainedbuf); // build and probe at the same time.
    }

    // place acknowledgment for si in rightSendQueue ;
    if (args->tid != args->nthreads - 1) {
        auto *ack = new fetch_t(fetch);
        ack->ack = true;
        DEBUGMSG("tid:%d pushes an acknowledgement of %d towards right\n",
                 args->tid, fetch->tuple->key);
        shuffler->push(args->tid + 1, ack, LEFT);
    }
}

void processRight_PMJ(baseShuffler *shuffler, arg_t *args, fetch_t *fetch,
                      int64_t *matches, void *chainedbuf) {

    // if msg contains a new tuple then
    if (fetch->fat_tuple) {
#ifdef DEBUG
        if (fetch->ISTuple_R) // right must be tuple S.
        {
          MSG("tid:%d processRIGHT: something is wrong, %d \n", args->tid,
              fetch->fat_tuple[0].key);
          fflush(stdout);
        }
#else
        assert(!fetch->ISTuple_R);
#endif

        //      scan R-window to find tuples that match si ;
        //      insert si into S-window ;
        args->joiner->join(args->tid, fetch->fat_tuple, fetch->fat_tuple_size,
                           fetch->ISTuple_R, matches,
                //                AGGFUNCTION,
                           chainedbuf); // build and probe at the same time.
    }

    // place acknowledgment for si in rightSendQueue ;
    if (args->tid != args->nthreads - 1) {
        auto *ack = new fetch_t(fetch);
        ack->ack = true;
        DEBUGMSG("tid:%d pushes an acknowledgement of %d towards right\n",
                 args->tid, fetch->fat_tuple[0].key);
        shuffler->push(args->tid + 1, ack, LEFT);
    }
}

void forward_tuples(baseShuffler *shuffler, arg_t *args, fetch_t *fetchR,
                    fetch_t *fetchS) {
    // place oldest non-forwarded si into leftSendQueue ;

    if (args->tid != 0) {
        if (fetchS && fetchS->tuple != NULL && !fetchS->ack) {
            DEBUGMSG("tid:%d pushes S? %d towards left\n", args->tid,
                     fetchS->tuple->key);
            shuffler->push(args->tid - 1, fetchS, RIGHT); // push S towards left.
        }
    }
    // mark si as forwarded;
    // do nothing.

    // place oldest ri into rightSendQueue ;
    if (args->tid != args->nthreads - 1) {
        if (fetchR && fetchR->tuple != NULL) {
            DEBUGMSG("tid:%d pushes R? %d towards right\n", args->tid,
                     fetchR->tuple->key);
            shuffler->push(args->tid + 1, fetchR, LEFT); // push R towards right.
            DEBUGMSG("remove r %d from R-window.", fetchR->tuple->key)
            //            clean(args, fetchR, LEFT);
            if (!fetchR->ack)
                args->joiner->clean(args->tid, fetchR->tuple, LEFT);
        }
    }
}

void forward_tuples_PMJ(baseShuffler *shuffler, arg_t *args, fetch_t *fetchR,
                        fetch_t *fetchS) {
    // place oldest non-forwarded si into leftSendQueue ;

    if (args->tid != 0) {
        if (fetchS && !fetchS->ack) {
            DEBUGMSG("tid:%d pushes S? %d towards left\n", args->tid,
                     fetchS->fat_tuple[0].key);
            shuffler->push(args->tid - 1, fetchS, RIGHT); // push S towards left.
        }
    }
    // mark si as forwarded;
    // do nothing.

    // place oldest ri into rightSendQueue ;
    if (args->tid != args->nthreads - 1) {
        if (fetchR) {
            DEBUGMSG("tid:%d pushes R? %d towards right\n", args->tid,
                     fetchR->fat_tuple[0].key);
            shuffler->push(args->tid + 1, fetchR, LEFT); // push R towards right.
            DEBUGMSG("tid:%d remove r %d from R-window.", args->tid,
                     fetchR->fat_tuple[0].key)
            //            clean(args, fetchR, LEFT);
            if (!fetchR->ack)
                args->joiner->clean(args->tid, fetchR->fat_tuple,
                                    fetchR->fat_tuple_size, LEFT);
        }
    }
}

/**
 * OUTDATED..
 * @param param
 * @return
 */
void *THREAD_TASK_SHUFFLE_HS(void *param) {
    arg_t *args = (arg_t *) param;
    int lock;

#ifdef PROFILE_TOPDOWN
#ifdef JOIN_THREAD
    // do nothing
#else
    return nullptr;
#endif
#endif

#ifdef JOIN_RESULT_MATERIALIZE
    chainedtuplebuffer_t *chainedbuf = chainedtuplebuffer_init();
#else
    void *chainedbuf = NULL;
#endif

    // call different data fetcher.
    baseFetcher *fetcher = args->fetcher;
    baseShuffler *shuffler = args->shuffler;

    fetcher->fetchStartTime = args->startTS; // set the fetch starting time.

    fetch_t *fetchR;
    fetch_t *fetchS;

    int sizeR = args->fetcher->relR->num_tuples;
    int sizeS = args->fetcher->relS->num_tuples;

    /* wait at a barrier until each thread started*/
    BARRIER_ARRIVE(args->barrier, lock)
    *args->startTS = curtick();

#ifndef NO_TIMING
    START_MEASURE((args->timer))
#endif


    fetcher->fetchStartTime = args->startTS; // set the fetch starting time.
#ifdef PERF_COUNTERS
    if (args->tid == 0) {
        PCM_initPerformanceMonitor(NULL, NULL);
        PCM_start();
        auto curtime = std::chrono::steady_clock::now();
        string path = EXP_DIR "/results/breakdown/time_start_" + std::to_string(args->exp_id) + ".txt";
        auto fp = fopen(path.c_str(), "w");
        fprintf(fp, "%ld\n", curtime);
    }
    BARRIER_ARRIVE(args->barrier, lock)
#endif

    do {
        // pull left queue.
        if (args->tid == 0) {
            fetchR = fetcher->next_tuple(); //
        } else {
            fetchR = shuffler->pull(args->tid, LEFT); // pull itself.
        }
        if (fetchR) {
            fetcher->cntR++;
            if (fetchR->ack) { /* msg is an acknowledgment message */
                fetcher->cntR--;
            } else {
#ifdef DEBUG
                MSG("tid:%d, fetch R:%d, cntR:%lu\n", args->tid, fetchR->tuple->key,
                    fetcher->cntR);
                fflush(stdout); // Will now print%luverything in the stdout buffer
#endif
            }
            processLeft(args, fetchR, args->matches, chainedbuf);
        }

        // pull right queue.
        if (args->tid == args->nthreads - 1) {
            fetchS = fetcher->next_tuple(); //
        } else {
            fetchS = shuffler->pull(args->tid, RIGHT); // pull itself.
        }
        if (fetchS) {
            fetcher->cntS++;
#ifdef DEBUG
            MSG("tid:%d, fetch S:%d, ack:%d, cntS:%d\n", args->tid,
                fetchS->tuple->key, fetchS->ack, fetcher->cntS);
            fflush(stdout); // Will now print everything in the stdout buffer
#endif
            processRight(shuffler, args, fetchS, args->matches, chainedbuf);
        }
#ifdef DEBUG
        if (fetchR)
          if (!fetchR->ISTuple_R && !fetchR->ack) {
            MSG("something is wrong.\n");
          }

        if (fetchS)
          if (fetchS->ISTuple_R) {
            MSG("something is wrong.\n");
          }
#endif
        // forward tuple twice!
        forward_tuples(shuffler, args, fetchR, fetchS);
#ifdef DEBUG
        usleep(rand() % 100);
#endif

    } while (fetcher->cntR < sizeR || fetcher->cntS < sizeS);
#ifdef JOIN
    args->joiner->merge(args->tid, args->matches,
            //            AGGFUNCTION,
                        chainedbuf);
#endif
#ifdef JOIN_RESULT_MATERIALIZE
    args->threadresult->nresults = *args->matches;
    args->threadresult->threadid = args->tid;
    args->threadresult->results = (void *)chainedbuf;
#endif

#ifndef NO_TIMING
    END_MEASURE(args->timer)
    // time calibration
    //    args->timer->overall_timer -= args->timer->garbage_time;
    args->timer->partition_timer =
            args->timer->overall_timer - args->timer->wait_timer;
#endif

#ifdef PERF_COUNTERS
    if (args->tid == 0) {
        PCM_stop();
        auto curtime = std::chrono::steady_clock::now();
        string path = EXP_DIR "/results/breakdown/time_end_" + std::to_string(args->exp_id) + ".txt";
        auto fp = fopen(path.c_str(), "w");
        fprintf(fp, "%ld\n", curtime);
        PCM_log("========== Probe phase profiling results ==========\n");
        PCM_printResults();
        PCM_log("===================================================\n");
        PCM_cleanup();
    }
    /* Just to make sure we get consistent performance numbers */
    BARRIER_ARRIVE(args->barrier, lock);
#endif
    /* wait at a barrier until each thread finishes*/
    BARRIER_ARRIVE(args->barrier, lock)
    DEBUGMSG("args->num_results (%d): %ld\n", args->tid, *args->matches);
    fflush(stdout);
    pthread_exit(NULL);
}

/**
 * OUTDATED..
 * @param param
 * @return
 */
void *THREAD_TASK_SHUFFLE_PMJHS(void *param) {
    arg_t *args = (arg_t *) param;

#ifdef PROFILE_TOPDOWN
#ifdef JOIN_THREAD
    // do nothing
#else
    return nullptr;
#endif
#endif

#ifdef PERF_COUNTERS
    if (args->tid == 0) {
        PCM_initPerformanceMonitor(NULL, NULL);
        PCM_start();
    }
#endif

#ifndef NO_TIMING
    /* the first thread checkpoints the start time */
    START_MEASURE((args->timer))
#endif

    int rv;
#ifdef PERF_COUNTERS
    if (args->tid == 0) {
        PCM_stop();
        PCM_log("========== Build phase profiling results ==========\n");
        PCM_printResults();
//      PCM_cleanup();
    }
    /* Just to make sure we get consistent performance numbers */
    BARRIER_ARRIVE(args->barrier, rv);
#endif

#ifdef JOIN_RESULT_MATERIALIZE
    chainedtuplebuffer_t *chainedbuf = chainedtuplebuffer_init();
#else
    void *chainedbuf = NULL;
#endif

    // call different data fetcher.
    baseFetcher *fetcher = args->fetcher;
    baseShuffler *shuffler = args->shuffler;

    fetch_t *fetchR;
    fetch_t *fetchS;

    do {
        // pull left queue.
        if (args->tid == 0) {
            fetchR = fetcher->next_tuple(); //
        } else {
            //            std::cin.get();//expect enter
            fetchR = shuffler->pull(args->tid, LEFT); // pull itself.
        }
        if (fetchR) {
            fetcher->cntR += fetchR->fat_tuple_size;
            if (fetchR->ack) { /* msg is an acknowledgment message */
                fetcher->cntR -= fetchR->fat_tuple_size;
                DEBUGMSG(
                        "TID%d fetches an ack S[]: %s", args->tid,
                        print_tuples(fetchR->fat_tuple, fetchR->fat_tuple_size).c_str())
            } else {
                DEBUGMSG(
                        "TID%d fetches tuple R[]: %s", args->tid,
                        print_tuples(fetchR->fat_tuple, fetchR->fat_tuple_size).c_str())
            }
            processLeft_PMJ(args, fetchR, args->matches, chainedbuf);
        }

        // pull right queue.
        if (args->tid == args->nthreads - 1) {
            fetchS = fetcher->next_tuple(); //
        } else {
            fetchS = shuffler->pull(args->tid, RIGHT); // pull itself.
        }
        if (fetchS) {
            fetcher->cntS += fetchS->fat_tuple_size;
            processRight_PMJ(shuffler, args, fetchS, args->matches, chainedbuf);
        }

        // forward tuple twice!
        forward_tuples_PMJ(shuffler, args, fetchR, fetchS);

        //        usleep(rand() % 100);

    } while (!fetcher->finish());

#ifdef JOIN_RESULT_MATERIALIZE
    args->threadresult->nresults = *args->matches;
    args->threadresult->threadid = args->tid;
    args->threadresult->results = (void *)chainedbuf;
#endif

#ifndef NO_TIMING
    END_MEASURE(args->timer)
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
    pthread_exit(NULL);
}