/**
 * @file    onlinejoins_st.c
 * @author  Shuhao Zhang <tonyzhang19900609@gmail.com>
 * @date    Sat Oct 26 2019
 * @version
 *
 * @brief  The implementation of online-join algorithms: SHJ,
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

#include "onlinejoins_st.h"
#include "npj_params.h"         /* constant parameters */
#include "npj_types.h"          /* bucket_t, hashtable_t, bucket_buffer_t */
#include "../utils/lock.h"               /* lock, unlock */
#include "../utils/cpu_mapping.h"        /* get_cpu_id */

#ifdef PERF_COUNTERS
#include "perf_counters.h"      /* PCM_x */
#endif


//#include "../../utils/affinity.h"           /* pthread_attr_setaffinity_np */
#include "../utils/generator.h"          /* numa_localize() */
#include "../utils/timer.h"  /* startTimer, stopTimer */
#include "common_functions.h"
#include "../utils/barrier.h"
#include <sched.h>

#ifdef JOIN_RESULT_MATERIALIZE
#include "tuple_buffer.h"       /* for materialization */
#endif

#ifndef HASH
#define HASH(X, MASK, SKIP) (((X) & MASK) >> SKIP)
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
 * As an example of join execution, consider a join with join predicate T1.attr1 = T2.attr2.
 * The join operator will incrementally load a hash table H1 for T1 by hashing attr1 using hash function f1,
 * and another hash table H2 for T2 by hashing attr2 using hash function f2.
 * The symmetric hash join operator starts by
 * (1): getting a tuple from T1, hashing its attr1 field using f1, and inserting it into H1.
 * (2): it probes H2 by applying f2 to attr1 of the current T1 tuple, returning any matched tuple pairs.
 *
 * (3): it gets a tuple from T2, hashes it by applying f2 to attr2, and inserts it into H2.
 * (4): it probes H1 by applying f1 to attr2 of the current T2 tuple, and returns any matches.
 * (5): This continues until all tuples from T1 and T2 have been consumed.
 *
 * @param relR
 * @param relS
 * @param nthreads
 * @return
 */
long build_probe_st(hashtable_t *ht_R, hashtable_t *ht_S, relation_t *rel_R, relation_t *rel_S,
                    void *pVoid) {
    uint32_t index_R = 0;//index of rel_R
    uint32_t index_S = 0;//index of rel_S

    const uint32_t hashmask_R = ht_R->hash_mask;
    const uint32_t skipbits_R = ht_R->skip_bits;

    const uint32_t hashmask_S = ht_S->hash_mask;
    const uint32_t skipbits_S = ht_S->skip_bits;

    int64_t matches = 0;//number of matches.

    do {
        if (index_R < rel_R->num_tuples) {
            build_hashtable_single(ht_R, rel_R, index_R, hashmask_R, skipbits_R);//(1)
            matches += proble_hashtable_single(ht_S, rel_R, index_R, hashmask_S, skipbits_S);//(2)
            index_R++;
        }

        if (index_S < rel_S->num_tuples) {
            build_hashtable_single(ht_S, rel_S, index_S, hashmask_S, skipbits_S);//(3)
            matches += proble_hashtable_single(ht_R, rel_S, index_S, hashmask_R, skipbits_R);//(4)
            index_S++;
        }
    } while (index_R < rel_R->num_tuples || index_S < rel_S->num_tuples);

    return matches;
}


result_t *
SHJ_st(relation_t *relR, relation_t *relS, int nthreads) {
    hashtable_t *htR;
    hashtable_t *htS;
    int64_t result = 0;
    result_t *joinresult;

#ifndef NO_TIMING
    struct timeval start, end;
    uint64_t timer1, timer2, timer3;
#endif

    //allocate two hashtables.
    uint32_t nbucketsR = (relR->num_tuples / BUCKET_SIZE);
    allocate_hashtable(&htR, nbucketsR);

    uint32_t nbucketsS = (relS->num_tuples / BUCKET_SIZE);
    allocate_hashtable(&htS, nbucketsS);

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

#ifdef JOIN_RESULT_MATERIALIZE
    chainedtuplebuffer_t * chainedbuf = chainedtuplebuffer_init();
#else
    void *chainedbuf = NULL;
#endif

    result = build_probe_st(htR, htS, relR, relS, chainedbuf);//build and probe at the same time.

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
    print_timing(timer1, timer2, timer3, relS->num_tuples, result, &start, &end);
#endif

    destroy_hashtable(htR);
    destroy_hashtable(htS);

    joinresult->totalresults = result;
    joinresult->nthreads = 1;

    return joinresult;
}


/**
 * Data partition then invovle SHJ_st.
 * @param relR
 * @param relS
 * @param nthreads
 * @return
 */
result_t *
SHJ(relation_t *relR, relation_t *relS, int nthreads) {
}

/** @}*/
