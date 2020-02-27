/**
 * @file    joincommon.h
 * @author  Cagri Balkesen <cagri.balkesen@inf.ethz.ch>
 * @date    Sat Dec 15 15:39:54 2012
 * @version $Id $
 *
 * @brief   Common structures, macros and functions of sort-merge join algorithms.
 *
 * (c) 2012-2014, ETH Zurich, Systems Group
 *
 * \ingroup Joins
 */

/**
 * @defgroup Joins Join implementations & related code
 */


#ifndef ALLIANCEDB_JOINCOMMON_H
#define ALLIANCEDB_JOINCOMMON_H

//#define DEBUG

#include <stdint.h>
#include <stdlib.h>             /* posix_memalign, EXIT_FAILURE */
#include <sys/time.h>           /* gettimeofday */
#include <stdio.h>              /* FILE */
#include <string>

#include "../util/types.h"             /* relation_t, tuple_t, result_t */
#include "../util/barrier.h"            /* pthread_barrier_* */
#include "../datagen/generator.h"          /* numa_localize() --> TODO: refactor */
#include "../util/params.h"             /* macro parameters */
#include "../util/t_timer.h"

#ifdef PERF_COUNTERS
#include "perf_counters.h"      /* PCM_x */
#endif

#define SKEW_HANDLING 1
#define SKEW_DECOMPOSE_MARGIN (1.10) /* 10% margin */
#define SKEW_DECOMPOSE_SAMPLES 64 /* nr. of samples for range partitioning. */
#define SKEW_MAX_HEAVY_HITTERS 16 /* max nr. of heavy hitters to detect. */
#define SKEW_HEAVY_HITTER_THR 0.5 /* heavy hitter threshold freq. */

#ifdef SKEW_HANDLING

#include "../util/generic_task_queue.h" /* taskqueue_t */


#endif

/** Global command line arguments, whether to execute scalar code */
//extern int scalarsortflag; /* by default AVX-code is executed */
//extern int scalarmergeflag;

/** Join thread arguments struct */
typedef struct arg_t arg_t;

/** To keep track of the input relation pairs fitting into L2 cache */
typedef struct relationpair_t relationpair_t;

/** Debug msg logging method */
#ifdef DEBUG
#define DEBUGMSG(COND, MSG, ...)                                        \
    if(COND) {                                                          \
        fprintf(stdout,                                                 \
                "[DEBUG @ %s:%d] " MSG, __FILE__, __LINE__, ## __VA_ARGS__); \
                fflush(stdout); \
    }
#else
#define DEBUGMSG(COND, MSG, ...)
#endif

/* In DEBUG mode, we also validate whether the sorting is successful. */
#ifdef DEBUG
#define DEBUG_SORT_CHECK 1
#endif

std::string print_relation(tuple_t *tuple, int length);

/** Initialize and run the given join algorithm with given number of threads */
result_t *
sortmergejoin_initrun(relation_t *relR, relation_t *relS, joinconfig_t *joincfg, void *(*jointhread)(void *),
                      int exp_id, const string algoName);

/** Print out timing stats for the given start and end timestamps */
void print_timing(uint64_t numtuples, struct timeval *start, struct timeval *end,
                  FILE *out);

/**
 * Does merge join on two sorted relations. Just a naive scalar
 * implementation. TODO: consider AVX for this code.
 *
 * @param rtuples sorted relation R
 * @param stuples sorted relation S
 * @param numR number of tuples in R
 * @param numS number of tuples in S
 * @param output join results, if JOIN_MATERIALIZE defined.
 */
uint64_t
merge_join(tuple_t *rtuples, tuple_t *stuples,
           const uint64_t numR, const uint64_t numS, void *output, T_TIMER *timer);

/**
 * Does merge join on two sorted relations with interpolation
 * searching in the beginning to find the search start index. Just a
 * naive scalar implementation. TODO: consider AVX for this code.
 *
 * @param rtuples sorted relation R
 * @param stuples sorted relation S
 * @param numR number of tuples in R
 * @param numS number of tuples in S
 * @param output join results, if JOIN_MATERIALIZE defined.
 */
uint64_t
merge_join_interpolation(tuple_t *rtuples, tuple_t *stuples,
                         const uint64_t numR, const uint64_t numS,
                         void *output);


int
is_sorted_helper(int64_t *items, uint64_t nitems);

/** utility method to check whether arrays are sorted */
void
check_sorted(int64_t *R, int64_t *S, uint64_t nR, uint64_t nS, int my_tid);

/** holds the arguments passed to each thread */
struct arg_t {
    tuple_t *relR;
    tuple_t *relS;

    /* temporary relations for partitioning output */
    tuple_t *tmp_partR;
    tuple_t *tmp_partS;

    /* temporary relations for sorting output */
    tuple_t *tmp_sortR;
    tuple_t *tmp_sortS;

    int32_t numR;
    int32_t numS;

    int32_t my_tid;
    int nthreads;
    /* join configuration parameters: */
    joinconfig_t *joincfg;

    pthread_barrier_t *barrier;
    int64_t result;

    relationpair_t **threadrelchunks;

    /** used for multi-way merging, shared by active threads in each NUMA. */
    tuple_t **sharedmergebuffer;

    /** arguments specific to mpsm-join: */
    uint32_t **histR;
    tuple_t *tmpRglobal;
    uint64_t totalR;

#ifdef SKEW_HANDLING
    /** skew handling task queues (1 per NUMA region). */
    taskqueue_t **numa_taskqueues;
#endif

#ifdef JOIN_MATERIALIZE
    /* results of the thread */
    threadresult_t * threadresult;
#endif

    /* timing stats */
    struct timeval start, end;
    uint64_t part, sort, mergedelta, merge, join;

    T_TIMER *timer;

    milliseconds *startTS;
} __attribute__((aligned(CACHE_LINE_SIZE)));


/** To keep track of the input relation pairs fitting into L2 cache */
struct relationpair_t {
    relation_t R;
    relation_t S;
};

#endif //ALLIANCEDB_JOINCOMMON_H
