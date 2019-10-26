/**
 * @file    no_partitioning_join.c
 * @author  Cagri Balkesen <cagri.balkesen@inf.ethz.ch>
 * @date    Sun Feb  5 20:16:58 2012
 * @version $Id: no_partitioning_join.c 4419 2013-10-21 16:24:35Z bcagri $
 *
 * @brief  The implementation of NPO, No Partitioning Optimized join algortihm.
 *
 * (c) 2012, ETH Zurich, Systems Group
 *
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sched.h>              /* CPU_ZERO, CPU_SET */
#include <pthread.h>            /* pthread_* */
#include <string.h>             /* memset */
#include <stdio.h>              /* printf */
#include <stdlib.h>             /* memalign */
#include <sys/time.h>           /* gettimeofday */

#include "no_partitioning_join.h"
#include "npj_params.h"         /* constant parameters */
#include "npj_types.h"          /* bucket_t, hashtable_t, bucket_buffer_t */
#include "../utils/lock.h"               /* lock, unlock */
#include "../utils/cpu_mapping.h"        /* get_cpu_id */

#ifdef PERF_COUNTERS
#include "perf_counters.h"      /* PCM_x */
#endif

#include "../utils/barrier.h"            /* pthread_barrier_* */
//#include "../../utils/affinity.h"           /* pthread_attr_setaffinity_np */
#include "../utils/generator.h"          /* numa_localize() */
#include "../utils/timer.h"
#include "common_functions.h"
#include <sched.h>

#ifdef JOIN_RESULT_MATERIALIZE
#include "tuple_buffer.h"       /* for materialization */
#endif

#ifndef BARRIER_ARRIVE
/** barrier wait macro */
#define BARRIER_ARRIVE(B,RV)                            \
    RV = pthread_barrier_wait(B);                       \
    if(RV !=0 && RV != PTHREAD_BARRIER_SERIAL_THREAD){  \
        printf("Couldn't wait on barrier\n");           \
        exit(EXIT_FAILURE);                             \
    }
#endif




/** Debug msg logging method */
#ifdef DEBUG
#define DEBUGMSG(COND, MSG, ...)                                    \
    if(COND) { fprintf(stdout, "[DEBUG] "MSG, ## __VA_ARGS__); }
#else
#define DEBUGMSG(COND, MSG, ...)
#endif


/**
 * \ingroup NPO arguments to the threads
 */
typedef struct arg_t arg_t;

struct arg_t {
    int32_t tid;
    hashtable_t *ht;
    relation_t relR;
    relation_t relS;
    pthread_barrier_t *barrier;
    int64_t num_results;

    /* results of the thread */
    threadresult_t *threadresult;

#ifndef NO_TIMING
    /* stats about the thread */
    uint64_t timer1, timer2, timer3;
    struct timeval start, end;
#endif
};

/**
 * @defgroup OverflowBuckets Buffer management for overflowing buckets.
 * Simple buffer management for overflow-buckets organized as a
 * linked-list of bucket_buffer_t.
 * @{
 */

/**
 * @defgroup OverflowBuckets Buffer management for overflowing buckets.
 * Simple buffer management for overflow-buckets organized as a
 * linked-list of bucket_buffer_t.
 * @{
 */

/**
 * Initializes a new bucket_buffer_t for later use in allocating
 * buckets when overflow occurs.
 *
 * @param ppbuf [in,out] bucket buffer to be initialized
 */
void
init_bucket_buffer(bucket_buffer_t **ppbuf) {
    bucket_buffer_t *overflowbuf;
    overflowbuf = (bucket_buffer_t *) malloc(sizeof(bucket_buffer_t));
    overflowbuf->count = 0;
    overflowbuf->next = NULL;

    *ppbuf = overflowbuf;
}

/**
 * Returns a new bucket_t from the given bucket_buffer_t.
 * If the bucket_buffer_t does not have enough space, then allocates
 * a new bucket_buffer_t and adds to the list.
 *
 * @param result [out] the new bucket
 * @param buf [in,out] the pointer to the bucket_buffer_t pointer
 */
static inline void
get_new_bucket(bucket_t **result, bucket_buffer_t **buf) {
    if ((*buf)->count < OVERFLOW_BUF_SIZE) {
        *result = (*buf)->buf + (*buf)->count;
        (*buf)->count++;
    } else {
        /* need to allocate new buffer */
        bucket_buffer_t *new_buf = (bucket_buffer_t *)
                malloc(sizeof(bucket_buffer_t));
        new_buf->count = 1;
        new_buf->next = *buf;
        *buf = new_buf;
        *result = new_buf->buf;
    }
}



/** @} */


/**
 * @defgroup NPO The No Partitioning Optimized Join Implementation
 * @{
 */

/** \copydoc NPO_st */
result_t *
NPO_st(relation_t *relR, relation_t *relS, int nthreads) {
    hashtable_t *ht;
    int64_t result = 0;
    result_t *joinresult;

#ifndef NO_TIMING
    struct timeval start, end;
    uint64_t timer1, timer2, timer3;
#endif
    uint32_t nbuckets = (relR->num_tuples / BUCKET_SIZE);
    allocate_hashtable(&ht, nbuckets);

    joinresult = (result_t *) malloc(sizeof(result_t));
#ifdef JOIN_RESULT_MATERIALIZE
    joinresult->resultlist = (threadresult_t *) malloc(sizeof(threadresult_t));
#endif

#ifndef NO_TIMING
    gettimeofday(&start, NULL);
    startTimer(&timer1);
    startTimer(&timer2);
    timer3 = 0; /* no partitioning */
#endif

    build_hashtable_st(ht, relR);

#ifndef NO_TIMING
    stopTimer(&timer2); /* for build */
#endif

#ifdef JOIN_RESULT_MATERIALIZE
    chainedtuplebuffer_t * chainedbuf = chainedtuplebuffer_init();
#else
    void *chainedbuf = NULL;
#endif

    result = probe_hashtable(ht, relS, chainedbuf);

#ifdef JOIN_RESULT_MATERIALIZE
    threadresult_t * thrres = &(joinresult->resultlist[0]);/* single-thread */
    thrres->nresults = result;
    thrres->threadid = 0;
    thrres->results  = (void *) chainedbuf;
#endif

#ifndef NO_TIMING
    stopTimer(&timer1); /* over all */
    gettimeofday(&end, NULL);
    /* now print the timing results: */
    print_timing(timer1, timer2, timer3, relS->num_tuples, result, &start, &end, nullptr);
#endif

    destroy_hashtable(ht);

    joinresult->totalresults = result;
    joinresult->nthreads = 1;

    return joinresult;
}

/**
 * Multi-thread hashtable build method, ht is pre-allocated.
 * Writes to buckets are synchronized via latches.
 *
 * @param ht hastable to be built
 * @param rel the build relationO
 * @param overflowbuf pre-allocated chunk of buckets for overflow use.
 */
void
build_hashtable_mt(hashtable_t *ht, relation_t *rel,
                   bucket_buffer_t **overflowbuf) {
    uint32_t i;
    const uint32_t hashmask = ht->hash_mask;
    const uint32_t skipbits = ht->skip_bits;

#ifdef PREFETCH_NPJ
    size_t prefetch_index = PREFETCH_DISTANCE;
#endif

    for (i = 0; i < rel->num_tuples; i++) {
        tuple_t *dest;
        bucket_t *curr, *nxt;

#ifdef PREFETCH_NPJ
        if (prefetch_index < rel->num_tuples) {
            intkey_t idx_prefetch = HASH(rel->tuples[prefetch_index++].key,
                                         hashmask, skipbits);
            __builtin_prefetch(ht->buckets + idx_prefetch, 1, 1);
        }
#endif

        int32_t idx = HASH(rel->tuples[i].key, hashmask, skipbits);
        /* copy the tuple to appropriate hash bucket */
        /* if full, follow nxt pointer to find correct place */
        curr = ht->buckets + idx;
        lock(&curr->latch);
        nxt = curr->next;

        if (curr->count == BUCKET_SIZE) {
            if (!nxt || nxt->count == BUCKET_SIZE) {
                bucket_t *b;
                /* b = (bucket_t*) calloc(1, sizeof(bucket_t)); */
                /* instead of calloc() everytime, we pre-allocate */
                get_new_bucket(&b, overflowbuf);
                curr->next = b;
                b->next = nxt;
                b->count = 1;
                dest = b->tuples;
            } else {
                dest = nxt->tuples + nxt->count;
                nxt->count++;
            }
        } else {
            dest = curr->tuples + curr->count;
            curr->count++;
        }

        *dest = rel->tuples[i];
        unlock(&curr->latch);
    }

}

/**
 * Just a wrapper to call the build and probe for each thread.
 *
 * @param param the parameters of the thread, i.e. tid, ht, reln, ...
 *
 * @return
 */
void *
npo_thread(void *param) {
    int rv;
    arg_t *args = (arg_t *) param;

    /* allocate overflow buffer for each thread */
    bucket_buffer_t *overflowbuf;
    init_bucket_buffer(&overflowbuf);

#ifdef PERF_COUNTERS
    if(args->tid == 0){
        PCM_initPerformanceMonitor(NULL, NULL);
        PCM_start();
    }
#endif

    /* wait at a barrier until each thread starts and start timer */
    BARRIER_ARRIVE(args->barrier, rv);

#ifndef NO_TIMING
    /* the first thread checkpoints the start time */
    if (args->tid == 0) {
        gettimeofday(&args->start, NULL);
        startTimer(&args->timer1);
        startTimer(&args->timer2);
        args->timer3 = 0; /* no partitionig phase */
    }
#endif

    /* insert tuples from the assigned part of relR to the ht */
    build_hashtable_mt(args->ht, &args->relR, &overflowbuf);

    /* wait at a barrier until each thread completes build phase */
    BARRIER_ARRIVE(args->barrier, rv);

#ifdef PERF_COUNTERS
    if(args->tid == 0){
      PCM_stop();
      PCM_log("========== Build phase profiling results ==========\n");
      PCM_printResults();
      PCM_start();
    }
    /* Just to make sure we get consistent performance numbers */
    BARRIER_ARRIVE(args->barrier, rv);
#endif


#ifndef NO_TIMING
    /* build phase finished, thread-0 checkpoints the time */
    if (args->tid == 0) {
        stopTimer(&args->timer2);
    }
#endif

#ifdef JOIN_RESULT_MATERIALIZE
    chainedtuplebuffer_t * chainedbuf = chainedtuplebuffer_init();
#else
    void *chainedbuf = NULL;
#endif

    /* probe for matching tuples from the assigned part of relS */
    args->num_results = probe_hashtable(args->ht, &args->relS, chainedbuf);

#ifdef JOIN_RESULT_MATERIALIZE
    args->threadresult->nresults = args->num_results;
    args->threadresult->threadid = args->tid;
    args->threadresult->results  = (void *) chainedbuf;
#endif

#ifndef NO_TIMING
    /* for a reliable timing we have to wait until all finishes */
    BARRIER_ARRIVE(args->barrier, rv);

    /* probe phase finished, thread-0 checkpoints the time */
    if (args->tid == 0) {
        stopTimer(&args->timer1);
        gettimeofday(&args->end, NULL);
    }
#endif

#ifdef PERF_COUNTERS
    if(args->tid == 0) {
        PCM_stop();
        PCM_log("========== Probe phase profiling results ==========\n");
        PCM_printResults();
        PCM_log("===================================================\n");
        PCM_cleanup();
    }
    /* Just to make sure we get consistent performance numbers */
    BARRIER_ARRIVE(args->barrier, rv);
#endif

    /* clean-up the overflow buffers */
    free_bucket_buffer(overflowbuf);

    return 0;
}

/** \copydoc NPO */
result_t *
NPO(relation_t *relR, relation_t *relS, int nthreads) {
    hashtable_t *ht;
    int64_t result = 0;
    int32_t numR, numS, numRthr, numSthr; /* total and per thread num */
    int i, rv;
    cpu_set_t set;
    arg_t args[nthreads];
    pthread_t tid[nthreads];
    pthread_attr_t attr;
    pthread_barrier_t barrier;

    result_t *joinresult = 0;
    joinresult = (result_t *) malloc(sizeof(result_t));

#ifdef JOIN_RESULT_MATERIALIZE
    joinresult->resultlist = (threadresult_t *) malloc(sizeof(threadresult_t)
                                                       * nthreads);
#endif

    uint32_t nbuckets = (relR->num_tuples / BUCKET_SIZE);
    allocate_hashtable(&ht, nbuckets);

    numR = relR->num_tuples;
    numS = relS->num_tuples;
    numRthr = numR / nthreads;
    numSthr = numS / nthreads;

    rv = pthread_barrier_init(&barrier, NULL, nthreads);
    if (rv != 0) {
        printf("Couldn't create the barrier\n");
        exit(EXIT_FAILURE);
    }

    pthread_attr_init(&attr);
    for (i = 0; i < nthreads; i++) {
        int cpu_idx = get_cpu_id(i);

        DEBUGMSG(1, "Assigning thread-%d to CPU-%d\n", i, cpu_idx);

        CPU_ZERO(&set);
        CPU_SET(cpu_idx, &set);
        pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &set);

        args[i].tid = i;
        args[i].ht = ht;
        args[i].barrier = &barrier;

        /* assing part of the relR for next thread */
        args[i].relR.num_tuples = (i == (nthreads - 1)) ? numR : numRthr;
        args[i].relR.tuples = relR->tuples + numRthr * i;
        numR -= numRthr;

        /* assing part of the relS for next thread */
        args[i].relS.num_tuples = (i == (nthreads - 1)) ? numS : numSthr;
        args[i].relS.tuples = relS->tuples + numSthr * i;
        numS -= numSthr;

        args[i].threadresult = &(joinresult->resultlist[i]);

        rv = pthread_create(&tid[i], &attr, npo_thread, (void *) &args[i]);
        if (rv) {
            printf("ERROR; return code from pthread_create() is %d\n", rv);
            exit(-1);
        }

    }

    for (i = 0; i < nthreads; i++) {
        pthread_join(tid[i], NULL);
        /* sum up results */
        result += args[i].num_results;
    }
    joinresult->totalresults = result;
    joinresult->nthreads = nthreads;


#ifndef NO_TIMING
    /* now print the timing results: */
    print_timing(args[0].timer1, args[0].timer2, args[0].timer3,
                 relS->num_tuples, result,
                 &args[0].start, &args[0].end, nullptr);
#endif

    destroy_hashtable(ht);

    return joinresult;
}

/** @}*/
