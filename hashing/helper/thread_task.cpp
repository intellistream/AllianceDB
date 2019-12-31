//
// Created by Shuhao Zhang on 1/11/19.
//


#include <execinfo.h>
#include "localjoiner.h"
#include "thread_task.h"
#include "fetcher.h"
#include "shuffler.h"
#include "../utils/perf_counters.h"
#include "boost/stacktrace.hpp"
#include "../joins/shj_struct.h"

/**
 * a JOIN function that a joiner should apply
 * @param r_tuple
 * @param s_tuple
 * @param matches
 * @return
 */
//void* JOINFUNCTION(tuple_t *r_tuple, tuple_t *s_tuple, int64_t *matches) {
void *AGGFUNCTION(const tuple_t *r_tuple, const tuple_t *s_tuple, int64_t *matches) {
    if (r_tuple->key == s_tuple->key) {
//        (*matches)++;
        DEBUGMSG("matches: %d", *matches);
    }
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
    START_MEASURE((*(args->timer)))
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

    do {
        fetch_t *fetch = fetcher->next_tuple(args->tid);

        if (fetch != nullptr) {
            args->joiner->join(
                    args->tid,
                    fetch->tuple,
                    fetch->ISTuple_R,
                    args->matches,
                    AGGFUNCTION,
                    chainedbuf);//build and probe at the same time.
        }
    } while (!fetcher->finish());

    args->joiner->cleanup(
            args->tid,
            args->matches,
            AGGFUNCTION,
            chainedbuf);

    DEBUGMSG("args->num_results (%d): %ld\n", args->tid, *args->matches);

#ifdef JOIN_RESULT_MATERIALIZE
    args->threadresult->nresults = args->num_results;
    args->threadresult->threadid = args->tid;
    args->threadresult->results  = (void *) chainedbuf;
#endif

#ifndef NO_TIMING
    END_MEASURE((*(args->timer)))
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
    START_MEASURE((*(args->timer)))
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

//    //allocate two hashtables on each thread assuming input stream statistics are known.
//    uint32_t nbucketsR = (args->fetcher->relR->num_tuples / BUCKET_SIZE);
//    allocate_hashtable(&args->htR, nbucketsR);
//
//    uint32_t nbucketsS = (args->fetcher->relS->num_tuples / BUCKET_SIZE);
//    allocate_hashtable(&args->htS, nbucketsS);

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
            args->joiner->join(
                    args->tid,
                    fetch->tuple,
                    fetch->ISTuple_R,
                    args->matches,
                    AGGFUNCTION,
                    chainedbuf);//build and probe at the same time.
        }
#endif
    } while (!fetcher->finish());

    /* wait at a barrier until each thread finishes fetch*/
    BARRIER_ARRIVE(args->barrier, rv)
    do {
        fetch = shuffler->pull(args->tid, false);//re-fetch from its shuffler.
        if (fetch != nullptr) {
            args->joiner->join(
                    args->tid,
                    fetch->tuple,
                    fetch->ISTuple_R,
                    args->matches,
                    AGGFUNCTION,
                    chainedbuf);
        } else {
            args->joiner->cleanup(
                    args->tid,
                    args->matches,
                    AGGFUNCTION,
                    chainedbuf);
            break;
        }
    } while (true);

#ifdef JOIN_RESULT_MATERIALIZE
    args->threadresult->nresults = args->num_results;
    args->threadresult->threadid = args->tid;
    args->threadresult->results  = (void *) chainedbuf;
#endif

#ifndef NO_TIMING
    END_MEASURE((*(args->timer)))
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
    DEBUGMSG("args->num_results (%d): %ld\n", args->tid, *args->matches);
    fflush(stdout);
}


#define LEFT true
#define RIGHT false

//void clean(arg_t *arg, fetch_t *fetch, bool cleanR) {
//
//    if (cleanR) {
//        //if SHJ is used, we need to clean up hashtable of R.
//        debuild_hashtable_single(arg->htR, fetch->tuple, arg->htR->hash_mask, arg->htR->skip_bits);
//
////        printf( "tid: %d remove tuple r %d from R-window. \n", arg->tid, fetch->tuple->key);
//
//        if (arg->tid == 0) {
//            window0.R_Window.remove(fetch->tuple->key);
////            print_window(window0.R_Window, 0);
//        } else {
//            window1.R_Window.remove(fetch->tuple->key);
////            print_window(window1.R_Window, 1);
//        }
//    } else {
//        debuild_hashtable_single(arg->htS, fetch->tuple, arg->htS->hash_mask, arg->htS->skip_bits);
//
////        printf("tid: %d remove tuple s %d from S-window. \n", arg->tid, fetch->tuple->key);
//
//        if (arg->tid == 0) {
//            window0.S_Window.remove(fetch->tuple->key);
////            print_window(window0.S_Window, 0);
//        } else {
//            window1.S_Window.remove(fetch->tuple->key);
////            print_window(window1.S_Window, 1);
//        }
////        std::cout << boost::stacktrace::stacktrace() << std::endl;
//
//    }
//}

/**
 *
 * @param args
 * @param fetch
 */
void
processLeft(arg_t *args, fetch_t *fetch, int64_t *matches, void *chainedbuf) {
    if (fetch->ack) {/* msg is an acknowledgment message */
        //remove oldest tuple from S-window
        DEBUGMSG("remove s %d from S-window.", fetch->tuple->key)
//        clean(args, fetch, RIGHT);
        args->joiner->clean(args->tid, fetch->tuple, RIGHT);
    } else if (fetch->tuple) { //if msg contains a new tuple then
#ifdef DEBUG
        if (!fetch->ISTuple_R)//right must be tuple R.
        {
            DEBUGMSG("right must be tuple R. something is wrong \n");
        }
#else
        assert(fetch->flag);//left must be tuple R.
#endif
        //scan S-window to find tuples that match ri ;
        //insert ri into R-window ;
        args->joiner->join(
                args->tid,
                fetch->tuple,
                fetch->ISTuple_R,
                matches,
                AGGFUNCTION,
                chainedbuf);
    }
}

/**
 *
 * @param args
 * @param fetch
 */
void
processLeft_PMJ(arg_t *args, fetch_t *fetch, int64_t *matches, void *chainedbuf) {
    if (fetch->ack) {/* msg is an acknowledgment message */
        //remove oldest tuple from S-window
        DEBUGMSG("TID %d:remove S %s from S-window.", args->tid,
                 print_tuples(fetch->fat_tuple, fetch->fat_tuple_size).c_str())
        args->joiner->clean(args->tid, fetch->fat_tuple, fetch->fat_tuple_size, RIGHT);
    } else if (fetch->fat_tuple) { //if msg contains a new tuple then
#ifdef DEBUG
        if (!fetch->ISTuple_R)//right must be tuple R.
        {
            DEBUGMSG("right must be tuple R, something is wrong \n");
        }
#else
        assert(fetch->flag);//left must be tuple R.
#endif
        //scan S-window to find tuples that match ri ;
        //insert ri into R-window ;
        args->joiner->join(
                args->tid,
                fetch->fat_tuple,
                fetch->fat_tuple_size,
                fetch->ISTuple_R,
                matches,
                AGGFUNCTION,
                chainedbuf);
    }
}

void
processRight(baseShuffler *shuffler, arg_t *args, fetch_t *fetch, int64_t *matches, void *chainedbuf) {

    //if msg contains a new tuple then
    if (fetch->tuple) {
#ifdef DEBUG
        if (fetch->ISTuple_R)//right must be tuple S.
        {
            printf("tid:%d processRIGHT: something is wrong, %d \n", args->tid, fetch->tuple->key);
            fflush(stdout);
        }
#else
        assert(!fetch->flag);
#endif

//      scan R-window to find tuples that match si ;
//      insert si into S-window ;
        args->joiner->join(
                args->tid,
                fetch->tuple,
                fetch->ISTuple_R,
                matches,
                AGGFUNCTION,
                chainedbuf);//build and probe at the same time.
    }

    //place acknowledgment for si in rightSendQueue ;
    if (args->tid != args->nthreads - 1) {
        auto *ack = new fetch_t(fetch);
        ack->ack = true;
        DEBUGMSG("tid:%d pushes an acknowledgement of %d towards right\n", args->tid, fetch->tuple->key);
        shuffler->push(args->tid + 1, ack, LEFT);
    }

}

void
processRight_PMJ(baseShuffler *shuffler, arg_t *args, fetch_t *fetch, int64_t *matches, void *chainedbuf) {

    //if msg contains a new tuple then
    if (fetch->fat_tuple) {
#ifdef DEBUG
        if (fetch->ISTuple_R)//right must be tuple S.
        {
            printf("tid:%d processRIGHT: something is wrong, %d \n", args->tid, fetch->fat_tuple[0].key);
            fflush(stdout);
        }
#else
        assert(!fetch->flag);
#endif

//      scan R-window to find tuples that match si ;
//      insert si into S-window ;
        args->joiner->join(
                args->tid,
                fetch->fat_tuple,
                fetch->fat_tuple_size,
                fetch->ISTuple_R,
                matches,
                AGGFUNCTION,
                chainedbuf);//build and probe at the same time.
    }

    //place acknowledgment for si in rightSendQueue ;
    if (args->tid != args->nthreads - 1) {
        auto *ack = new fetch_t(fetch);
        ack->ack = true;
        DEBUGMSG("tid:%d pushes an acknowledgement of %d towards right\n", args->tid, fetch->fat_tuple[0].key);
        shuffler->push(args->tid + 1, ack, LEFT);
    }

}

void forward_tuples(baseShuffler *shuffler, arg_t *args, fetch_t *fetchR, fetch_t *fetchS) {
    //place oldest non-forwarded si into leftSendQueue ;

    if (args->tid != 0) {
        if (fetchS && fetchS->tuple != NULL && !fetchS->ack) {
            DEBUGMSG("tid:%d pushes S? %d towards left\n", args->tid, fetchS->tuple->key);
            shuffler->push(args->tid - 1, fetchS, RIGHT);//push S towards left.
        }
    }
    //mark si as forwarded;
    //do nothing.

    //place oldest ri into rightSendQueue ;
    if (args->tid != args->nthreads - 1) {
        if (fetchR && fetchR->tuple != NULL) {
            DEBUGMSG("tid:%d pushes R? %d towards right\n", args->tid, fetchR->tuple->key);
            shuffler->push(args->tid + 1, fetchR, LEFT);//push R towards right.
            DEBUGMSG("remove r %d from R-window.", fetchR->tuple->key)
//            clean(args, fetchR, LEFT);
            if (!fetchR->ack)
                args->joiner->clean(args->tid, fetchR->tuple, LEFT);
        }
    }
}

void forward_tuples_PMJ(baseShuffler *shuffler, arg_t *args, fetch_t *fetchR, fetch_t *fetchS) {
    //place oldest non-forwarded si into leftSendQueue ;

    if (args->tid != 0) {
        if (fetchS && !fetchS->ack) {
            DEBUGMSG("tid:%d pushes S? %d towards left\n", args->tid, fetchS->fat_tuple[0].key);
            shuffler->push(args->tid - 1, fetchS, RIGHT);//push S towards left.
        }
    }
    //mark si as forwarded;
    //do nothing.

    //place oldest ri into rightSendQueue ;
    if (args->tid != args->nthreads - 1) {
        if (fetchR) {
            DEBUGMSG("tid:%d pushes R? %d towards right\n", args->tid, fetchR->fat_tuple[0].key);
            shuffler->push(args->tid + 1, fetchR, LEFT);//push R towards right.
            DEBUGMSG("tid:%d remove r %d from R-window.", args->tid, fetchR->fat_tuple[0].key)
//            clean(args, fetchR, LEFT);
            if (!fetchR->ack)
                args->joiner->clean(args->tid, fetchR->fat_tuple, fetchR->fat_tuple_size, LEFT);
        }
    }
}

void
*THREAD_TASK_SHUFFLE_HS(void *param) {
    arg_t *args = (arg_t *) param;
#ifdef PERF_COUNTERS
    if (args->tid == 0) {
        PCM_initPerformanceMonitor(NULL, NULL);
        PCM_start();
    }
#endif

#ifndef NO_TIMING
    START_MEASURE((*(args->timer)))
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

    //call different data fetcher.
    baseFetcher *fetcher = args->fetcher;
    baseShuffler *shuffler = args->shuffler;


    fetch_t *fetchR;
    fetch_t *fetchS;

    int sizeR = args->fetcher->relR->num_tuples;
    int sizeS = args->fetcher->relS->num_tuples;
    int cntR = 0;
    int cntS = 0;

    do {

        //pull left queue.
        if (args->tid == 0) {
            fetchR = fetcher->next_tuple(args->tid);//
        } else {
            fetchR = shuffler->pull(args->tid, LEFT);//pull itself.
        }
        if (fetchR) {
            cntR++;
            if (fetchR->ack) {/* msg is an acknowledgment message */
                cntR--;
            }
#ifdef DEBUG
            printf("tid:%d, fetch R:%d, cntR:%d\n", args->tid, fetchR->tuple->key, cntR);
            fflush(stdout); // Will now print everything in the stdout buffer
#endif
            processLeft(args, fetchR, args->matches, chainedbuf);
        }

        //pull right queue.
        if (args->tid == args->nthreads - 1) {
            fetchS = fetcher->next_tuple(args->tid);//
        } else {
            fetchS = shuffler->pull(args->tid, RIGHT);//pull itself.
        }
        if (fetchS) {
            cntS++;
#ifdef DEBUG
            printf("tid:%d, fetch S:%d, ack:%d, cntS:%d\n", args->tid, fetchS->tuple->key, fetchS->ack, cntS);
            fflush(stdout); // Will now print everything in the stdout buffer
#endif
            processRight(shuffler, args, fetchS, args->matches, chainedbuf);
        }

        if (fetchR)
            if (!fetchR->ISTuple_R && !fetchR->ack) {
                printf("something is wrong.\n");
            }

        if (fetchS)
            if (fetchS->ISTuple_R) {
                printf("something is wrong.\n");
            }

        //forward tuple twice!
        forward_tuples(shuffler, args, fetchR, fetchS);
#ifdef DEBUG
        usleep(rand() % 100);
#endif
    } while (cntR < sizeR || cntS < sizeS);

    args->joiner->cleanup(
            args->tid,
            args->matches,
            AGGFUNCTION,
            chainedbuf);

#ifdef JOIN_RESULT_MATERIALIZE
    args->threadresult->nresults = args->num_results;
    args->threadresult->threadid = args->tid;
    args->threadresult->results  = (void *) chainedbuf;
#endif

#ifndef NO_TIMING
    END_MEASURE((*(args->timer)))
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

}

void
*THREAD_TASK_SHUFFLE_PMJHS(void *param) {
    arg_t *args = (arg_t *) param;
#ifdef PERF_COUNTERS
    if (args->tid == 0) {
        PCM_initPerformanceMonitor(NULL, NULL);
        PCM_start();
    }
#endif

#ifndef NO_TIMING
    /* the first thread checkpoints the start time */
    START_MEASURE((*(args->timer)))
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

    //call different data fetcher.
    baseFetcher *fetcher = args->fetcher;
    baseShuffler *shuffler = args->shuffler;

    fetch_t *fetchR;
    fetch_t *fetchS;

    int sizeR = args->fetcher->relR->num_tuples;
    int sizeS = args->fetcher->relS->num_tuples;
    int cntR = 0;
    int cntS = 0;

    do {
        //pull left queue.
        if (args->tid == 0) {
            fetchR = fetcher->next_tuple(args->tid);//
        } else {
//            std::cin.get();//expect enter
            fetchR = shuffler->pull(args->tid, LEFT);//pull itself.
        }
        if (fetchR) {
            cntR += fetchR->fat_tuple_size;
            if (fetchR->ack) {/* msg is an acknowledgment message */
                cntR -= fetchR->fat_tuple_size;
                DEBUGMSG("TID%d fetches an ack S[]: %s", args->tid,
                         print_tuples(fetchR->fat_tuple, fetchR->fat_tuple_size).c_str())
            } else {
                DEBUGMSG("TID%d fetches tuple R[]: %s", args->tid,
                         print_tuples(fetchR->fat_tuple, fetchR->fat_tuple_size).c_str())
            }
            processLeft_PMJ(args, fetchR, args->matches, chainedbuf);
        }

        //pull right queue.
        if (args->tid == args->nthreads - 1) {
            fetchS = fetcher->next_tuple(args->tid);//
        } else {
            fetchS = shuffler->pull(args->tid, RIGHT);//pull itself.
        }
        if (fetchS) {
            cntS += fetchS->fat_tuple_size;
            processRight_PMJ(shuffler, args, fetchS, args->matches, chainedbuf);
        }

        //forward tuple twice!
        forward_tuples_PMJ(shuffler, args, fetchR, fetchS);

    } while (cntR < sizeR || cntS < sizeS);

#ifdef JOIN_RESULT_MATERIALIZE
    args->threadresult->nresults = args->num_results;
    args->threadresult->threadid = args->tid;
    args->threadresult->results  = (void *) chainedbuf;
#endif

#ifndef NO_TIMING
    END_MEASURE((*(args->timer)))
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

}



