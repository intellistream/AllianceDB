/**
 * @file    parallel_radix_join.c
 * @author  Cagri Balkesen <cagri.balkesen@inf.ethz.ch>
 * @date    Sun Feb 20:19:51 2012
 * @version $Id: parallel_radix_join.c 4548 2013-12-07 16:05:16Z bcagri $
 *
 * @brief  Provides implementations for several variants of Radix Hash Join.
 *
 * (c) 2012, ETH Zurich, Systems Group
 *
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <pthread.h>   /* pthread_* */
#include <sched.h>     /* CPU_ZERO, CPU_SET */
#include <smmintrin.h> /* simd only for 32-bit keys – SSE4.1 */
#include <stdio.h>     /* printf */
#include <stdlib.h>    /* malloc, posix_memalign */
#include <sys/time.h>  /* gettimeofday */

#include "../utils/cpu_mapping.h" /* get_cpu_id */
#include "../utils/task_queue.h"  /* task_queue_* */
#include "parallel_radix_join.h"
#include "prj_params.h" /* constant parameters */

#include "../utils/barrier.h" /* pthread_barrier_* */
#include <sched.h>
//#include "../utils/affinity.h"           /* pthread_attr_setaffinity_np */
#include "../timer/t_timer.h"   /* startTimer, stopTimer */
#include "../utils/generator.h" /* numa_localize() */
#include "../utils/perf_counters.h"
#include <algorithm>
#include <immintrin.h>
#include "unistd.h"

#ifdef JOIN_RESULT_MATERIALIZE

#include "../utils/tuple_buffer.h" /* for materialization */

#endif

/** \internal */

#ifndef BARRIER_ARRIVE
/** barrier wait macro */
#define BARRIER_ARRIVE(B, RV)                                                  \
  RV = pthread_barrier_wait(B);                                                \
  if (RV != 0 && RV != PTHREAD_BARRIER_SERIAL_THREAD) {                        \
    MSG("Couldn't wait on barrier\n");                                         \
    exit(EXIT_FAILURE);                                                        \
  }
#endif

/** checks malloc() result */
#ifndef MALLOC_CHECK
#define MALLOC_CHECK(M)                                                        \
  if (!M) {                                                                    \
    MSG("[ERROR] MALLOC_CHECK: %s : %d\n", __FILE__, __LINE__);                \
    perror(": malloc() failed!\n");                                            \
    exit(EXIT_FAILURE);                                                        \
  }
#endif

/* #define RADIX_HASH(V)  ((V>>7)^(V>>13)^(V>>21)^V) */
#define HASH_BIT_MODULO(K, MASK, NBITS) (((K)&MASK) >> NBITS)

#ifndef NEXT_POW_2
/**
 *  compute the next number, greater than or equal to 32-bit unsigned v.
 *  taken from "bit twiddling hacks":
 *  http://graphics.stanford.edu/~seander/bithacks.html
 */
#define NEXT_POW_2(V)                                                          \
  do {                                                                         \
    V--;                                                                       \
    V |= V >> 1;                                                               \
    V |= V >> 2;                                                               \
    V |= V >> 4;                                                               \
    V |= V >> 8;                                                               \
    V |= V >> 16;                                                              \
    V++;                                                                       \
  } while (0)
#endif

#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

#ifdef SYNCSTATS
#define SYNC_TIMERS_START(A, TID)                                              \
  do {                                                                         \
    uint64_t tnow;                                                             \
    startTimer(&tnow);                                                         \
    A->localtimer.sync1[0] = tnow;                                             \
    A->localtimer.sync1[1] = tnow;                                             \
    A->localtimer.sync3 = tnow;                                                \
    A->localtimer.sync4 = tnow;                                                \
    A->localtimer.finish_time = tnow;                                          \
    if (TID == 0) {                                                            \
      A->globaltimer->sync1[0] = tnow;                                         \
      A->globaltimer->sync1[1] = tnow;                                         \
      A->globaltimer->sync3 = tnow;                                            \
      A->globaltimer->sync4 = tnow;                                            \
      A->globaltimer->finish_time = tnow;                                      \
    }                                                                          \
  } while (0)

#define SYNC_TIMER_STOP(T) stopTimer(T)
#define SYNC_GLOBAL_STOP(T, TID)                                               \
  if (TID == 0) {                                                              \
    stopTimer(T);                                                              \
  }
#else
#define SYNC_TIMERS_START(A, TID)
#define SYNC_TIMER_STOP(T)
#define SYNC_GLOBAL_STOP(T, TID)
#endif

/* just to enable compilation with g++ */
#if defined(__cplusplus)
#define restrict __restrict__
#endif

/** An experimental feature to allocate input relations numa-local */
extern int numalocalize; /* defined in generator.c */

typedef struct arg_t arg_t;
typedef struct part_t part_t;
typedef struct synctimer_t synctimer_t;

typedef int64_t (*JoinFunction)(const relation_t *const,
                                const relation_t *const, relation_t *const,
                                void *output, T_TIMER *timer);

/** holds the arguments passed to each thread */
struct arg_t {
  int32_t **histR;
  tuple_t *relR;
  tuple_t *tmpR;
  int32_t **histS;
  tuple_t *relS;
  tuple_t *tmpS;

  int32_t numR;
  int32_t numS;
  int64_t totalR;
  int64_t totalS;

  uint64_t *startTS;

  task_queue_t **join_queue;
  task_queue_t **part_queue;
#ifdef SKEW_HANDLING
  task_queue_t *skew_queue;
  task_t **skewtask;
#endif
  pthread_barrier_t *barrier;
  JoinFunction join_function;
  int64_t result;
  int32_t tid;
  int nthreads;

  /* results of the thread */
  threadresult_t *threadresult;

  /* stats about the thread */
  int32_t parts_processed;
  //#ifndef NO_TIMING
  T_TIMER *timer;
  //#endif

  int exp_id; // for perf stat

#ifdef SYNCSTATS
  /** Thread local timers : */
  synctimer_t localtimer;
  /** Global synchronization timers, only filled in by thread-0 */
  synctimer_t *globaltimer;
#endif
} __attribute__((aligned(CACHE_LINE_SIZE)));
//
///** holds the arguments passed to each thread */
// struct arg_t {
//    int32_t **histR;
//    tuple_t *relR;
//    tuple_t *tmpR;
//    int32_t **histS;
//    tuple_t *relS;
//    tuple_t *tmpS;
//
//    int32_t numR;
//    int32_t numS;
//    int64_t totalR;
//    int64_t totalS;
//
//    task_queue_t **join_queue;
//    task_queue_t **part_queue;
//#ifdef SKEW_HANDLING
//    task_queue_t *      skew_queue;
//    task_t **           skewtask;
//#endif
//    pthread_barrier_t *barrier;
////    int64_t (*)(const relation_t *const, const relation_t *const, relation_t
///*const, void *) join_function;
//    int64_t result;
//    int32_t my_tid;
//    int nthreads;
//
//    /* results of the thread */
//    threadresult_t *threadresult;
//
//    /* stats about the thread */
//    int32_t parts_processed;
//
//#ifndef NO_TIMING
//    T_TIMER *timer;
//#endif
//#ifdef SYNCSTATS
//    /** Thread local timers : */
//    synctimer_t localtimer;
//    /** Global synchronization timers, only filled in by thread-0 */
//    synctimer_t * globaltimer;
//#endif
//} __attribute__((aligned(CACHE_LINE_SIZE)));

#ifdef SYNCSTATS
/** holds syncronization timing stats if configured with --enable-syncstats */
struct synctimer_t {
  /** Barrier for computation of thread-local histogram */
  uint64_t sync1[3]; /* for rel R and for rel S */
  /** Barrier for end of radix partit. pass-1 */
  uint64_t sync3;
  /** Barrier before join (build-probe) begins */
  uint64_t sync4;
  /** Finish time */
  uint64_t finish_time;
};
#endif

/** holds arguments passed for partitioning */
struct part_t {
  tuple_t *rel;
  tuple_t *tmp;
  int32_t **hist;
  int64_t *output;
  arg_t *thrargs;
  uint64_t total_tuples;
  uint32_t num_tuples;
  int32_t R;
  uint32_t D;
  int relidx; /* 0: R, 1: S */
  uint32_t padding;
} __attribute__((aligned(CACHE_LINE_SIZE)));

// void *
// alloc_aligned(size_t size) {
//    void *ret;
//    int rv;
//    rv = posix_memalign((void **) &ret, CACHE_LINE_SIZE, size);
//
//    if (rv) {
//        perror("alloc_aligned() failed: out of memory");
//        return 0;
//    }
//
//    return ret;
//}

/** \endinternal */

/**
 * @defgroup Radix Radix Join Implementation Variants
 * @{
 */

/**
 *  This algorithm builds the hashtable using the bucket chaining idea and used
 *  in PRO implementation. Join between given two relations is evaluated using
 *  the "bucket chaining" algorithm proposed by Manegold et al. It is used after
 *  the partitioning phase, which is common for all algorithms. Moreover, R and
 *  S typically fit into L2 or at least R and |R|*sizeof(int) fits into L2
 * cache.
 *
 * @param R input relation R
 * @param S input relation S
 * @param output join results, if JOIN_RESULT_MATERIALIZE defined.
 *
 * @return number of result tuples
 */
int64_t bucket_chaining_join(const relation_t *const R,
                             const relation_t *const S, relation_t *const tmpR,
                             void *output, T_TIMER *timer) {
  int *next, *bucket;
  const uint32_t numR = R->num_tuples;
  uint32_t N = numR;
  int64_t matches = 0;

  NEXT_POW_2(N);
  /* N <<= 1; */
  const uint32_t MASK = (N - 1) << (NUM_RADIX_BITS);

  next = (int *)malloc(sizeof(int) * numR);
  /* posix_memalign((void**)&next, CACHE_LINE_SIZE, numR * sizeof(int)); */
  bucket = (int *)calloc(N, sizeof(int));

  const tuple_t *const Rtuples = R->tuples;
  for (uint32_t i = 0; i < numR;) {
    uint32_t idx = HASH_BIT_MODULO(R->tuples[i].key, MASK, NUM_RADIX_BITS);
    next[i] = bucket[idx];
    bucket[idx] = ++i; /* we start pos's from 1 instead of 0 */

    /* Enable the following tO avoid the code elimination
       when running probe only for the time break-down experiment */
    /*matches += idx;*/
  }

  const tuple_t *const Stuples = S->tuples;
  const uint32_t numS = S->num_tuples;

#ifdef JOIN_RESULT_MATERIALIZE
  chainedtuplebuffer_t *chainedbuf = (chainedtuplebuffer_t *)output;
#endif
  /* Disable the following loop for no-probe for the break-down experiments */
  /* PROBE-LOOP */
  for (uint32_t i = 0; i < numS; i++) {
    uint32_t idx = HASH_BIT_MODULO(Stuples[i].key, MASK, NUM_RADIX_BITS);

    for (int hit = bucket[idx]; hit > 0; hit = next[hit - 1]) {

      if (Stuples[i].key == Rtuples[hit - 1].key) {

#ifdef JOIN_RESULT_MATERIALIZE
        /* copy to the result buffer, we skip it */
        tuple_t *joinres = cb_next_writepos(chainedbuf);
        joinres->key = Stuples[i].key;
        joinres->payloadID = Stuples[i].payloadID;
#endif
        matches++;
#ifndef NO_TIMING
        END_PROGRESSIVE_MEASURE(Stuples[i].payloadID, (timer),
                                false) // assume S as the input tuple.
#endif
      }
    }
  }
  /* PROBE-LOOP END  */

  /* clean up temp */
  free(bucket);
  free(next);

  return matches;
}

/** computes and returns the histogram size for join */
inline uint32_t get_hist_size(uint32_t relSize) __attribute__((always_inline));

inline uint32_t get_hist_size(uint32_t relSize) {
  NEXT_POW_2(relSize);
  relSize >>= 2;
  if (relSize < 4)
    relSize = 4;
  return relSize;
}

/**
 * Histogram-based hash table build method together with relation re-ordering as
 * described by Kim et al. It joins partitions Ri, Si of relations R & S.
 * This is version is not optimized with SIMD and prefetching. The parallel
 * radix join implementation using this function is PRH.
 */
int64_t histogram_join(const relation_t *const R, const relation_t *const S,
                       relation_t *const tmpR, void *output, T_TIMER *timer) {
  int32_t *restrict hist;
  const tuple_t *restrict const Rtuples = R->tuples;
  const uint32_t numR = R->num_tuples;
  uint32_t Nhist = get_hist_size(numR);
  const uint32_t MASK = (Nhist - 1) << NUM_RADIX_BITS;

  hist = (int32_t *)calloc(Nhist + 2, sizeof(int32_t));

  for (uint32_t i = 0; i < numR; i++) {

    uint32_t idx = HASH_BIT_MODULO(Rtuples[i].key, MASK, NUM_RADIX_BITS);

    hist[idx + 2]++;
  }

  /* prefix sum on histogram */
  for (uint32_t i = 2, sum = 0; i <= Nhist + 1; i++) {
    sum += hist[i];
    hist[i] = sum;
  }

  tuple_t *const tmpRtuples = tmpR->tuples;
  /* reorder tuples according to the prefix sum */
  for (uint32_t i = 0; i < numR; i++) {

    uint32_t idx = HASH_BIT_MODULO(Rtuples[i].key, MASK, NUM_RADIX_BITS) + 1;

    tmpRtuples[hist[idx]] = Rtuples[i];

    hist[idx]++;
  }

  int64_t match = 0;
  const uint32_t numS = S->num_tuples;
  const tuple_t *const Stuples = S->tuples;
  /* now comes the probe phase, TODO: implement prefetching */
  for (uint32_t i = 0; i < numS; i++) {

    uint32_t idx = HASH_BIT_MODULO(Stuples[i].key, MASK, NUM_RADIX_BITS);

    int j = hist[idx], end = hist[idx + 1];

    /* Scalar comparisons */
    for (; j < end; j++) {

      if (Stuples[i].key == tmpRtuples[j].key) {

        ++match;
        /* TODO: we do not output results */
      }
    }
  }

  /* clean up */
  free(hist);

  return match;
}

/** software prefetching function */
inline void prefetch(void *addr) __attribute__((always_inline));

inline void prefetch(void *addr) {
  /* #ifdef __x86_64__ */
  __asm__ __volatile__("prefetcht0 %0" ::"m"(*(uint32_t *)addr));
  /* _mm_prefetch(addr, _MM_HINT_T0); */
  /* #endif */
}

/**
 * Histogram-based hash table build method together with relation re-ordering as
 * described by Kim et al. It joins partitions Ri, Si of relations R & S.
 * This is version includes SIMD and prefetching optimizations as described by
 * Kim et al. The parallel radix join implementation using this function is
 * PRHO. Note: works only for 32-bit keys.
 */
int64_t histogram_optimized_join(const relation_t *const R,
                                 const relation_t *const S,
                                 relation_t *const tmpR, void *output,
                                 T_TIMER *timer) {
#ifdef KEY_8B
#warning SIMD comparison for 64-bit keys are not implemented!
  return 0;
#else
  int32_t *restrict hist;
  const tuple_t *restrict const Rtuples = R->tuples;
  const uint32_t numR = R->num_tuples;
  uint32_t Nhist = get_hist_size(numR);
  const uint32_t mask = (Nhist - 1) << NUM_RADIX_BITS;

  hist = (int32_t *)calloc(Nhist + 2, sizeof(int32_t));

  /* compute histogram */
  for (uint32_t i = 0; i < numR; i++) {
    uint32_t idx = HASH_BIT_MODULO(Rtuples[i].key, mask, NUM_RADIX_BITS);
    hist[idx + 2]++;
  }

  /* prefix sum on histogram */
  for (uint32_t i = 2, sum = 0; i <= Nhist + 1; i++) {
    sum += hist[i];
    hist[i] = sum;
  }

  tuple_t *restrict const tmpRtuples = tmpR->tuples;
  /* reorder tuples according to the prefix sum */
  for (uint32_t i = 0; i < numR; i++) {
    uint32_t idx = HASH_BIT_MODULO(Rtuples[i].key, mask, NUM_RADIX_BITS) + 1;
    tmpRtuples[hist[idx]] = Rtuples[i];
    hist[idx]++;
  }

#ifdef SMALL_PADDING_TUPLES
  /* if there is a padding between sub-relations,clear last 3 keys for SIMD */
  for (uint32_t i = numR + 3; i >= numR; i--) {
    tmpRtuples[numR].key = 0;
  }
#endif

  intkey_t key_buffer[PROBE_BUFFER_SIZE];
  uint32_t hash_buffer[PROBE_BUFFER_SIZE];

  int64_t match = 0;
  const uint32_t numS = S->num_tuples;
  const tuple_t *restrict const Stuples = S->tuples;
  __m128i counter = _mm_setzero_si128();

  for (uint32_t i = 0; i < numS / PROBE_BUFFER_SIZE; i++) {

    for (int k = 0; k < PROBE_BUFFER_SIZE; k++) {
      const intkey_t skey = Stuples[i * PROBE_BUFFER_SIZE + k].key;
      const uint32_t idx = HASH_BIT_MODULO(skey, mask, NUM_RADIX_BITS);
      /* now we issue a prefetch for element at S[idx] */
      prefetch(tmpR->tuples + hist[idx]);
      key_buffer[k] = skey;
      hash_buffer[k] = idx;
    }

    for (int k = 0; k < PROBE_BUFFER_SIZE; k++) {

      /* SIMD comparisons in groups of 2 (8B x 2 = 128 bits) */
      int j = hist[hash_buffer[k]];
      int end = hist[hash_buffer[k] + 1];
      __m128i search_key = _mm_set1_epi32(key_buffer[k]);

      for (; j < end; j += 2) {

        __m128i keyvals = _mm_loadu_si128((__m128i const *)(tmpRtuples + j));
        keyvals = _mm_cmpeq_epi32(keyvals, search_key);
        counter = _mm_add_epi32(keyvals, counter);
        /* TODO: we're just counting, not materializing results */
      }
    }
  }

  for (uint32_t i = numS - (numS % PROBE_BUFFER_SIZE); i < numS; i++) {
    const intkey_t skey = Stuples[i].key;
    const uint32_t idx = HASH_BIT_MODULO(skey, mask, NUM_RADIX_BITS);

    /* SIMD comparisons in groups of 2 (8B x 2 = 128 bits) */
    int j = hist[idx];
    int end = hist[idx + 1];
    __m128i search_key = _mm_set1_epi32(skey);

    for (; j < end; j += 2) {

      __m128i keyvals = _mm_loadu_si128((__m128i const *)(tmpRtuples + j));
      keyvals = _mm_cmpeq_epi32(keyvals, search_key);
      counter = _mm_add_epi32(keyvals, counter);
      /* TODO: we're just counting, not outputting anything */
    }
  }

  match += -(_mm_extract_epi32(counter, 0) + _mm_extract_epi32(counter, 2));

  /* clean up */
  free(hist);

  return match;
#endif
}

/**
 * Radix clustering algorithm (originally described by Manegold et al)
 * The algorithm mimics the 2-pass radix clustering algorithm from
 * Kim et al. The difference is that it does not compute
 * prefix-sum, instead the sum (offset in the code) is computed iteratively.
 *
 * @warning This method puts padding between clusters, see
 * radix_cluster_nopadding for the one without padding.
 *
 * @param outRel [out] result of the partitioning
 * @param inRel [in] input relation
 * @param hist [out] number of tuples in each partition
 * @param R cluster bits
 * @param D radix bits per pass
 * @returns tuples per partition.
 */
void radix_cluster(relation_t *restrict outRel, relation_t *restrict inRel,
                   int32_t *restrict hist, int R, int D) {
  uint32_t i;
  uint32_t M = ((1 << D) - 1) << R;
  uint32_t offset;
  uint32_t fanOut = 1 << D;

  /* the following are fixed size when D is same for all the passes,
     and can be re-used from call to call. Allocating in this function
     just in case D differs from call to call. */
  uint32_t dst[fanOut];

  /* count tuples per cluster */
  for (i = 0; i < inRel->num_tuples; i++) {
    uint32_t idx = HASH_BIT_MODULO(inRel->tuples[i].key, M, R);
    hist[idx]++;
  }

  offset = 0;
  /* determine the start and end of each cluster depending on the counts. */
  for (i = 0; i < fanOut; i++) {
    /* dst[i]      = outRel->tuples + offset; */
    /* determine the beginning of each partitioning by adding some
       padding to avoid L1 conflict misses during scatter. */
    dst[i] = offset + i * SMALL_PADDING_TUPLES;
    offset += hist[i];
  }

  //    DEBUGMSG("Thread exit %d, %d \n", outRel->num_tuples,
  //    outRel->tuples[0].key)

  /* copy tuples to their corresponding clusters at appropriate offsets */
  for (i = 0; i < inRel->num_tuples; i++) {
    //        DEBUGMSG("Thread enters\n")
    uint32_t idx = HASH_BIT_MODULO(inRel->tuples[i].key, M, R);
    //        DEBUGMSG("Thread exit %d\n", idx)
    //        DEBUGMSG("Thread exit %d\n", dst[idx])
    //        DEBUGMSG("Thread exit %d\n", outRel->tuples[0].key)
    outRel->tuples[dst[idx]] = inRel->tuples[i];
    ++dst[idx];
  }
}

/**
 * Radix clustering algorithm which does not put padding in between
 * clusters. This is used only by single threaded radix join implementation RJ.
 *
 * @param outRel
 * @param inRel
 * @param hist
 * @param R
 * @param D
 */
void radix_cluster_nopadding(relation_t *outRel, relation_t *inRel, int R,
                             int D) {
  tuple_t **dst;
  tuple_t *input;
  /* tuple_t ** dst_end; */
  uint32_t *tuples_per_cluster;
  uint32_t i;
  uint32_t offset;
  const uint32_t M = ((1 << D) - 1) << R;
  const uint32_t fanOut = 1 << D;
  const uint32_t ntuples = inRel->num_tuples;

  tuples_per_cluster = (uint32_t *)calloc(fanOut, sizeof(uint32_t));
  /* the following are fixed size when D is same for all the passes,
     and can be re-used from call to call. Allocating in this function
     just in case D differs from call to call. */
  dst = (tuple_t **)malloc(sizeof(tuple_t *) * fanOut);
  /* dst_end = (tuple_t**)malloc(sizeof(tuple_t*)*fanOut); */

  input = inRel->tuples;
  /* count tuples per cluster */
  for (i = 0; i < ntuples; i++) {
    uint32_t idx = (uint32_t)(HASH_BIT_MODULO(input->key, M, R));
    tuples_per_cluster[idx]++;
    input++;
  }

  offset = 0;
  /* determine the start and end of each cluster depending on the counts. */
  for (i = 0; i < fanOut; i++) {
    dst[i] = outRel->tuples + offset;
    offset += tuples_per_cluster[i];
    /* dst_end[i]  = outRel->tuples + offset; */
  }

  input = inRel->tuples;
  /* copy tuples to their corresponding clusters at appropriate offsets */
  for (i = 0; i < ntuples; i++) {
    uint32_t idx = (uint32_t)(HASH_BIT_MODULO(input->key, M, R));
    *dst[idx] = *input;
    ++dst[idx];
    input++;
    /* we pre-compute the start and end of each cluster, so the following
       check is unnecessary */
    /* if(++dst[idx] >= dst_end[idx]) */
    /*     REALLOCATE(dst[idx], dst_end[idx]); */
  }

  /* clean up temp */
  /* free(dst_end); */
  free(dst);
  free(tuples_per_cluster);
}

/**
 * This function implements the radix clustering of a given input
 * relations. The relations to be clustered are defined in task_t and after
 * clustering, each partition pair is added to the join_queue to be joined.
 *
 * @param task description of the relation to be partitioned
 * @param join_queue task queue to add join tasks after clustering
 */
void serial_radix_partition(task_t *const task, task_queue_t *join_queue,
                            const int R, const int D) {

  int i;
  uint32_t offsetR = 0, offsetS = 0;
  const int fanOut = 1 << D; /*(NUM_RADIX_BITS / NUM_PASSES);*/
  int32_t *outputR, *outputS;

  outputR = (int32_t *)calloc(fanOut + 1, sizeof(int32_t));
  outputS = (int32_t *)calloc(fanOut + 1, sizeof(int32_t));

  //    DEBUGMSG("%d, %d ", task->tmpR.tuples[0], task->relR.tuples[0])
  /* TODO: measure the effect of memset() */
  /* memset(outputR, 0, fanOut * sizeof(int32_t)); */
  radix_cluster(&task->tmpR, &task->relR, outputR, R, D);
  /* memset(outputS, 0, fanOut * sizeof(int32_t)); */
  radix_cluster(&task->tmpS, &task->relS, outputS, R, D);

  /* task_t t; */
  for (i = 0; i < fanOut; i++) {
    if (outputR[i] > 0 && outputS[i] > 0) {
      task_t *t = task_queue_get_slot_atomic(join_queue);
      t->relR.num_tuples = outputR[i];
      t->relR.tuples = task->tmpR.tuples + offsetR + i * SMALL_PADDING_TUPLES;
      t->tmpR.tuples = task->relR.tuples + offsetR + i * SMALL_PADDING_TUPLES;
      offsetR += outputR[i];

      t->relS.num_tuples = outputS[i];
      t->relS.tuples = task->tmpS.tuples + offsetS + i * SMALL_PADDING_TUPLES;
      t->tmpS.tuples = task->relS.tuples + offsetS + i * SMALL_PADDING_TUPLES;
      offsetS += outputS[i];

      /* task_queue_copy_atomic(join_queue, &t); */
      task_queue_add_atomic(join_queue, t);
    } else {
      offsetR += outputR[i];
      offsetS += outputS[i];
    }
  }
  free(outputR);
  free(outputS);
}

/**
 * This function implements the parallel radix partitioning of a given input
 * relation. Parallel partitioning is done by histogram-based relation
 * re-ordering as described by Kim et al. Parallel partitioning method is
 * commonly used by all parallel radix join algorithms.
 *
 * @param part description of the relation to be partitioned
 */
void parallel_radix_partition(part_t *const part) {
  const tuple_t *restrict rel = part->rel;
  int32_t **hist = part->hist;
  int64_t *restrict output = part->output;

  const uint32_t my_tid = part->thrargs->tid;
  const uint32_t nthreads = part->thrargs->nthreads;
  const uint32_t num_tuples = part->num_tuples;

  const int32_t R = part->R;
  const int32_t D = part->D;
  const uint32_t fanOut = 1 << D;
  const uint32_t MASK = (fanOut - 1) << R;
  const uint32_t padding = part->padding;

  int64_t sum = 0;
  uint32_t i, j;
  int rv;

  int64_t dst[fanOut + 1];

  /* compute local histogram for the assigned region of rel */
  /* compute histogram */
  int32_t *my_hist = hist[my_tid];

  for (i = 0; i < num_tuples; i++) {
    uint32_t idx = HASH_BIT_MODULO(rel[i].key, MASK, R);
    my_hist[idx]++;
  }

  /* compute local prefix sum on hist */
  for (i = 0; i < fanOut; i++) {
    sum += my_hist[i];
    my_hist[i] = sum;
  }

  //    SYNC_TIMER_STOP(&part->thrargs->localtimer.sync1[part->relidx]);
  /* wait at a barrier until each thread complete histograms */
  BARRIER_ARRIVE(part->thrargs->barrier, rv);
  /* barrier global sync point-1 */
  //    SYNC_GLOBAL_STOP(&part->thrargs->globaltimer->sync1[part->relidx],
  //    my_tid);

  /* determine the start and end of each cluster */
  for (i = 0; i < my_tid; i++) {
    for (j = 0; j < fanOut; j++)
      output[j] += hist[i][j];
  }
  for (i = my_tid; i < nthreads; i++) {
    for (j = 1; j < fanOut; j++)
      output[j] += hist[i][j - 1];
  }

  for (i = 0; i < fanOut; i++) {
    output[i] += i * padding; // PADDING_TUPLES;
    dst[i] = output[i];
  }
  output[fanOut] = part->total_tuples + fanOut * padding; // PADDING_TUPLES;

  tuple_t *restrict tmp = part->tmp;

  /* Copy tuples to their corresponding clusters */
  for (i = 0; i < num_tuples; i++) {
    uint32_t idx = HASH_BIT_MODULO(rel[i].key, MASK, R);
    tmp[dst[idx]] = rel[i];
    ++dst[idx];
  }
}

/**
 * @defgroup SoftwareManagedBuffer Optimized Partitioning Using SW-buffers
 * @{
 */
typedef union {
  struct {
    tuple_t tuples[CACHE_LINE_SIZE / sizeof(tuple_t)];
  } tuples;
  struct {
    tuple_t tuples[CACHE_LINE_SIZE / sizeof(tuple_t) - 1];
    int64_t slot;
  } data;
} cacheline_t;

#define TUPLESPERCACHELINE (CACHE_LINE_SIZE / sizeof(tuple_t))

/**
 * Makes a non-temporal write of 64 bytes from src to dst.
 * Uses vectorized non-temporal stores if available, falls
 * back to assignment copy.
 *
 * @param dst
 * @param src
 *
 * @return
 */
static inline void store_nontemp_64B(void *dst, void *src) {
#ifdef __AVX__
   __m256i *d1 = (__m256i *)dst;
   __m256i s1 = *((__m256i *)src);
   __m256i *d2 = d1 + 1;
   __m256i s2 = *(((__m256i *)src) + 1);

  _mm256_stream_si256(d1, s1);
  _mm256_stream_si256(d2, s2);

#elif defined(__SSE2__)

   __m128i *d1 = (__m128i *)dst;
   __m128i *d2 = d1 + 1;
   __m128i *d3 = d1 + 2;
   __m128i *d4 = d1 + 3;
   __m128i s1 = *(__m128i *)src;
   __m128i s2 = *((__m128i *)src + 1);
   __m128i s3 = *((__m128i *)src + 2);
   __m128i s4 = *((__m128i *)src + 3);

  _mm_stream_si128(d1, s1);
  _mm_stream_si128(d2, s2);
  _mm_stream_si128(d3, s3);
  _mm_stream_si128(d4, s4);

#else
  /* just copy with assignment */
  *(cacheline_t *)dst = *(cacheline_t *)src;

#endif
}

/**
 * This function implements the parallel radix partitioning of a given input
 * relation. Parallel partitioning is done by histogram-based relation
 * re-ordering as described by Kim et al. Parallel partitioning method is
 * commonly used by all parallel radix join algorithms. However this
 * implementation is further optimized to benefit from write-combining and
 * non-temporal writes.
 *
 * @param part description of the relation to be partitioned
 */
void parallel_radix_partition_optimized(part_t *const part) {
  const tuple_t *restrict rel = part->rel;
  int32_t **hist = part->hist;
  int64_t *restrict output = part->output;

  const uint32_t my_tid = part->thrargs->tid;
  const uint32_t nthreads = part->thrargs->nthreads;
  const uint32_t num_tuples = part->num_tuples;

  const int32_t R = part->R;
  const int32_t D = part->D;
  const uint32_t fanOut = 1 << D;
  const uint32_t MASK = (fanOut - 1) << R;
  const uint32_t padding = part->padding;

  int64_t sum = 0;
  uint32_t i, j;
  int rv;

  /* compute local histogram for the assigned region of rel */
  /* compute histogram */
  int32_t *my_hist = hist[my_tid];

  for (i = 0; i < num_tuples; i++) {
    uint32_t idx = HASH_BIT_MODULO(rel[i].key, MASK, R);
    my_hist[idx]++;
  }

  /* compute local prefix sum on hist */
  for (i = 0; i < fanOut; i++) {
    sum += my_hist[i];
    my_hist[i] = sum;
  }

  SYNC_TIMER_STOP(&part->thrargs->localtimer.sync1[part->relidx]);
  /* wait at a barrier until each thread complete histograms */
  BARRIER_ARRIVE(part->thrargs->barrier, rv);
  /* barrier global sync point-1 */
  SYNC_GLOBAL_STOP(&part->thrargs->globaltimer->sync1[part->relidx], my_tid);

  /* determine the start and end of each cluster */
  for (i = 0; i < my_tid; i++) {
    for (j = 0; j < fanOut; j++)
      output[j] += hist[i][j];
  }
  for (i = my_tid; i < nthreads; i++) {
    for (j = 1; j < fanOut; j++)
      output[j] += hist[i][j - 1];
  }

  /* uint32_t pre; /\* nr of tuples to cache-alignment *\/ */
  tuple_t *restrict tmp = part->tmp;
  /* software write-combining buffer */
  cacheline_t buffer[fanOut] __attribute__((aligned(CACHE_LINE_SIZE)));

  for (i = 0; i < fanOut; i++) {
    uint64_t off = output[i] + i * padding;
    /* pre        = (off + TUPLESPERCACHELINE) & ~(TUPLESPERCACHELINE-1); */
    /* pre       -= off; */
    output[i] = off;
    buffer[i].data.slot = off;
  }
  output[fanOut] = part->total_tuples + fanOut * padding;

  /* Copy tuples to their corresponding clusters */
  for (i = 0; i < num_tuples; i++) {
    uint32_t idx = HASH_BIT_MODULO(rel[i].key, MASK, R);
    uint64_t slot = buffer[idx].data.slot;
    tuple_t *tup = (tuple_t *)(buffer + idx);
    uint32_t slotMod = (slot) & (TUPLESPERCACHELINE - 1);
    tup[slotMod] = rel[i];

    if (slotMod == (TUPLESPERCACHELINE - 1)) {
      /* write out 64-Bytes with non-temporal store */
      store_nontemp_64B((tmp + slot - (TUPLESPERCACHELINE - 1)),
                        (buffer + idx));
      /* writes += TUPLESPERCACHELINE; */
    }

    buffer[idx].data.slot = slot + 1;
  }
  /* _mm_sfence (); */

  /* write out the remainders in the buffer */
  for (i = 0; i < fanOut; i++) {
    uint64_t slot = buffer[i].data.slot;
    uint32_t sz = (slot) & (TUPLESPERCACHELINE - 1);
    slot -= sz;
    for (uint32_t j = 0; j < sz; j++) {
      tmp[slot] = buffer[i].data.tuples[j];
      slot++;
    }
  }
}

/**
 * The main thread of parallel radix join. It does partitioning in parallel with
 * other threads and during the join phase, picks up join tasks from the task
 * queue and calls appropriate JoinFunction to compute the join task.
 *
 * @param param
 *
 * @return
 */
void *prj_thread(void *param) {

  arg_t *args = (arg_t *)param;
  int32_t my_tid = args->tid;
  int rv;


  const int fanOut = 1 << (NUM_RADIX_BITS / NUM_PASSES); // PASS1RADIXBITS;
  const int R = (NUM_RADIX_BITS / NUM_PASSES);           // PASS1RADIXBITS;
  const int D =
      (NUM_RADIX_BITS - (NUM_RADIX_BITS / NUM_PASSES)); // PASS2RADIXBITS;

  uint64_t results = 0;
  int i;


  part_t part;
  task_t *task;
  task_queue_t *part_queue;
  task_queue_t *join_queue;

#ifdef SKEW_HANDLING
  task_queue_t *skew_queue;
#endif

  int64_t *outputR = (int64_t *)calloc((fanOut + 1), sizeof(int64_t));
  int64_t *outputS = (int64_t *)calloc((fanOut + 1), sizeof(int64_t));
  MALLOC_CHECK((outputR && outputS));

  int numaid = get_numa_id(my_tid);
  part_queue = args->part_queue[numaid];
  join_queue = args->join_queue[numaid];

#ifdef SKEW_HANDLING
  skew_queue = args->skew_queue;
#endif

  args->histR[my_tid] = (int32_t *)calloc(fanOut, sizeof(int32_t));
  args->histS[my_tid] = (int32_t *)calloc(fanOut, sizeof(int32_t));

  /* in the first pass, partitioning is done together by all threads */

  args->parts_processed = 0;

#ifdef PROFILE_TOPDOWN
#ifdef JOIN_THREAD
    if (my_tid == 0) {
        sleep(1);
    }
    BARRIER_ARRIVE(args->barrier, rv);
#else
    return nullptr;
#endif
#endif

#ifdef OVERVIEW // overview counters
#ifdef PERF_COUNTERS
  if (my_tid == 0) {
    PCM_initPerformanceMonitor(NULL, NULL);
    PCM_start();
      auto curtime = std::chrono::steady_clock::now();
      string path = EXP_DIR "/results/breakdown/time_start_" + std::to_string(args->exp_id) + ".txt";
      auto fp = fopen(path.c_str(), "w");
      fprintf(fp, "%ld\n", curtime);
      sleep(1);
  }
#endif
#endif

#ifndef OVERVIEW // non-overview case
#ifdef NO_JOIN // partition only
#ifdef PERF_COUNTERS
  if (my_tid == 0) {
    PCM_initPerformanceMonitor(NULL, NULL);
    PCM_start();
  }
#endif
#endif
#endif

  //    /* wait at a barrier until each thread starts and then start the T_TIMER
  //    */
  BARRIER_ARRIVE(args->barrier, rv)
  /* if monitoring synchronization stats */

#ifndef NO_TIMING
  if (args->tid == 0) {
    *args->startTS = curtick(); // assign the start timestamp
    START_MEASURE((args->timer))
    BEGIN_MEASURE_PARTITION((args->timer)) /* partitioning start */
  }
#endif

  /********** 1st pass of multi-pass partitioning ************/
  part.R = 0;
  part.D = NUM_RADIX_BITS / NUM_PASSES; // PASS1RADIXBITS
  part.thrargs = args;
  part.padding = PADDING_TUPLES;

  /* 1. partitioning for relation R */
  part.rel = args->relR;
  part.tmp = args->tmpR;
  part.hist = args->histR;
  part.output = outputR;
  part.num_tuples = args->numR;
  part.total_tuples = args->totalR;
  part.relidx = 0;

#ifdef USE_SWWC_OPTIMIZED_PART
  parallel_radix_partition_optimized(&part);
#else
  parallel_radix_partition(&part);
#endif

  /* 2. partitioning for relation S */
  part.rel = args->relS;
  part.tmp = args->tmpS;
  part.hist = args->histS;
  part.output = outputS;
  part.num_tuples = args->numS;
  part.total_tuples = args->totalS;
  part.relidx = 1;

#ifdef USE_SWWC_OPTIMIZED_PART
  parallel_radix_partition_optimized(&part);
#else
  parallel_radix_partition(&part);
#endif

  /* wait at a barrier until each thread copies out */
  BARRIER_ARRIVE(args->barrier, rv);

  /********** end of 1st partitioning phase ******************/

#ifdef SKEW_HANDLING
  /* experimental skew threshold */
  /* const int thresh1 = MAX((1<<D), (1<<R)) * THRESHOLD1(args->nthreads); */
  /* const int thresh1 = MAX(args->totalR, arg->totalS)/MAX((1<<D),(1<<R)); */
  const int thresh1 = 64 * THRESHOLD1(args->nthreads);
#endif

  /* 3. first thread creates partitioning tasks for 2nd pass */
  if (my_tid == 0) {
    /* For Debugging: */
    /* int numnuma = get_num_numa_regions(); */
    /* int correct_numa_mapping = 0, wrong_numa_mapping = 0; */
    /* int counts[4] = {0, 0, 0, 0}; */
    for (i = 0; i < fanOut; i++) {
      int32_t ntupR = outputR[i + 1] - outputR[i] - PADDING_TUPLES;
      int32_t ntupS = outputS[i + 1] - outputS[i] - PADDING_TUPLES;

#ifdef SKEW_HANDLING

      if (ntupR > thresh1 || ntupS > thresh1) {
        DEBUGMSG(1, "Adding to skew_queue= R:%d, S:%d\n", ntupR, ntupS);

        task_t *t = task_queue_get_slot(skew_queue);

        t->relR.num_tuples = t->tmpR.num_tuples = ntupR;
        t->relR.tuples = args->tmpR + outputR[i];
        t->tmpR.tuples = args->relR + outputR[i];

        t->relS.num_tuples = t->tmpS.num_tuples = ntupS;
        t->relS.tuples = args->tmpS + outputS[i];
        t->tmpS.tuples = args->relS + outputS[i];

        task_queue_add(skew_queue, t);
      } else
#endif
          if (ntupR > 0 && ntupS > 0) {
        /* Determine the NUMA node of each partition: */
        void *ptr = (void *)&((args->tmpR + outputR[i])[0]);
        int pq_idx = get_numa_node_of_address(ptr);

        /* For Debugging: */
        /* void * ptr2 = (void*)&((args->tmpS + outputS[i])[0]); */
        /* int pq_idx2 = get_numa_node_of_address(ptr2); */
        /* if(pq_idx != pq_idx2) */
        /*     wrong_numa_mapping ++; */
        /* else */
        /*     correct_numa_mapping ++; */
        /* int numanode_of_mem = get_numa_node_of_address(ptr); */
        /* int pq_idx = i / (fanOut / numnuma); */
        /* if(numanode_of_mem == pq_idx) */
        /*     correct_numa_mapping ++; */
        /* else */
        /*     wrong_numa_mapping ++; */
        /* pq_idx = numanode_of_mem; */
        /* counts[numanode_of_mem] ++; */
        /* counts[pq_idx] ++; */

        task_queue_t *numalocal_part_queue = args->part_queue[pq_idx];
        task_t *t = task_queue_get_slot(numalocal_part_queue);

        t->relR.num_tuples = t->tmpR.num_tuples = ntupR;
        t->relR.tuples = args->tmpR + outputR[i];
        t->tmpR.tuples = args->relR + outputR[i];

        t->relS.num_tuples = t->tmpS.num_tuples = ntupS;
        t->relS.tuples = args->tmpS + outputS[i];
        t->tmpS.tuples = args->relS + outputS[i];

        task_queue_add(numalocal_part_queue, t);
      }
    }

    /* debug partitioning task queue */
#ifdef DEBUG
    DEBUGMSG("Pass-2: # partitioning tasks = %d\n", part_queue->count)
#endif

    /* DEBUG NUMA MAPPINGS */
    /* MSG("Correct NUMA-mappings = %d, Wrong = %d\n", */
    /*        correct_numa_mapping, wrong_numa_mapping); */
    /* MSG("Counts -- 0=%d, 1=%d, 2=%d, 3=%d\n",  */
    /*        counts[0], counts[1], counts[2], counts[3]); */
  }

  SYNC_TIMER_STOP(&args->localtimer.sync3);
  /* wait at a barrier until first thread adds all partitioning tasks */
  BARRIER_ARRIVE(args->barrier, rv);
  /* global barrier sync point-3 */
  SYNC_GLOBAL_STOP(&args->globaltimer->sync3, my_tid);

  /************ 2nd pass of multi-pass partitioning ********************/
  /* 4. now each thread further partitions and add to join task queue **/

#if NUM_PASSES == 1
  /* If the partitioning is single pass we directly add tasks from pass-1 */
  task_queue_t *swap = join_queue;
  join_queue = part_queue;
  /* part_queue is used as a temporary queue for handling skewed parts */
  part_queue = swap;

#elif NUM_PASSES == 2

  while ((task = task_queue_get_atomic(part_queue))) {
    serial_radix_partition(task, join_queue, R, D);
  }

#else
#warning Only 2-pass partitioning is implemented, set NUM_PASSES to 2!
#endif

#ifdef SKEW_HANDLING
  /* Partitioning pass-2 for skewed relations */
  part.R = R;
  part.D = D;
  part.thrargs = args;
  part.padding = SMALL_PADDING_TUPLES;

  while (1) {
    if (my_tid == 0) {
      *args->skewtask = task_queue_get_atomic(skew_queue);
    }
    BARRIER_ARRIVE(args->barrier, rv);
    if (*args->skewtask == NULL)
      break;

    DEBUGMSG((my_tid == 0), "Got skew task = R: %d, S: %d\n",
             (*args->skewtask)->relR.num_tuples,
             (*args->skewtask)->relS.num_tuples);

    int32_t numperthr = (*args->skewtask)->relR.num_tuples / args->nthreads;
    const int fanOut2 = (1 << D);

    free(outputR);
    free(outputS);

    outputR = (int64_t *)calloc(fanOut2 + 1, sizeof(int64_t));
    outputS = (int64_t *)calloc(fanOut2 + 1, sizeof(int64_t));

    free(args->histR[my_tid]);
    free(args->histS[my_tid]);

    args->histR[my_tid] = (int32_t *)calloc(fanOut2, sizeof(int32_t));
    args->histS[my_tid] = (int32_t *)calloc(fanOut2, sizeof(int32_t));

    /* wait until each thread allocates memory */
    BARRIER_ARRIVE(args->barrier, rv);

    /* 1. partitioning for relation R */
    part.rel = (*args->skewtask)->relR.tuples + my_tid * numperthr;
    part.tmp = (*args->skewtask)->tmpR.tuples;
    part.hist = args->histR;
    part.output = outputR;
    part.num_tuples =
        (my_tid == (args->nthreads - 1))
            ? ((*args->skewtask)->relR.num_tuples - my_tid * numperthr)
            : numperthr;
    part.total_tuples = (*args->skewtask)->relR.num_tuples;
    part.relidx = 2; /* meaning this is pass-2, no syncstats */
    parallel_radix_partition(&part);

    numperthr = (*args->skewtask)->relS.num_tuples / args->nthreads;
    /* 2. partitioning for relation S */
    part.rel = (*args->skewtask)->relS.tuples + my_tid * numperthr;
    part.tmp = (*args->skewtask)->tmpS.tuples;
    part.hist = args->histS;
    part.output = outputS;
    part.num_tuples =
        (my_tid == (args->nthreads - 1))
            ? ((*args->skewtask)->relS.num_tuples - my_tid * numperthr)
            : numperthr;
    part.total_tuples = (*args->skewtask)->relS.num_tuples;
    part.relidx = 2; /* meaning this is pass-2, no syncstats */
    parallel_radix_partition(&part);

    /* wait at a barrier until each thread copies out */
    BARRIER_ARRIVE(args->barrier, rv);

    /* first thread adds join tasks */
    if (my_tid == 0) {
      const int THR1 = THRESHOLD1(args->nthreads);

      for (i = 0; i < fanOut2; i++) {
        int32_t ntupR = outputR[i + 1] - outputR[i] - SMALL_PADDING_TUPLES;
        int32_t ntupS = outputS[i + 1] - outputS[i] - SMALL_PADDING_TUPLES;
        if (ntupR > THR1 || ntupS > THR1) {

          DEBUGMSG(1, "Large join task = R: %d, S: %d\n", ntupR, ntupS);

          /* use part_queue temporarily */
          for (int k = 0; k < args->nthreads; k++) {
            int ns = (k == args->nthreads - 1)
                         ? (ntupS - k * (ntupS / args->nthreads))
                         : (ntupS / args->nthreads);
            task_t *t = task_queue_get_slot(part_queue);

            t->relR.num_tuples = t->tmpR.num_tuples = ntupR;
            t->relR.tuples = (*args->skewtask)->tmpR.tuples + outputR[i];
            t->tmpR.tuples = (*args->skewtask)->relR.tuples + outputR[i];

            t->relS.num_tuples = t->tmpS.num_tuples = ns; // ntupS;
            t->relS.tuples = (*args->skewtask)->tmpS.tuples + outputS[i] //;
                             + k * (ntupS / args->nthreads);
            t->tmpS.tuples = (*args->skewtask)->relS.tuples + outputS[i] //;
                             + k * (ntupS / args->nthreads);

            task_queue_add(part_queue, t);
          }
        } else if (ntupR > 0 && ntupS > 0) {
          task_t *t = task_queue_get_slot(join_queue);

          t->relR.num_tuples = t->tmpR.num_tuples = ntupR;
          t->relR.tuples = (*args->skewtask)->tmpR.tuples + outputR[i];
          t->tmpR.tuples = (*args->skewtask)->relR.tuples + outputR[i];

          t->relS.num_tuples = t->tmpS.num_tuples = ntupS;
          t->relS.tuples = (*args->skewtask)->tmpS.tuples + outputS[i];
          t->tmpS.tuples = (*args->skewtask)->relS.tuples + outputS[i];

          task_queue_add(join_queue, t);

          DEBUGMSG(1, "Join added = R: %d, S: %d\n", t->relR.num_tuples,
                   t->relS.num_tuples);
        }
      }
    }
  }

  /* add large join tasks in part_queue to the front of the join queue */
  if (my_tid == 0) {
    while ((task = task_queue_get_atomic(part_queue)))
      task_queue_add(join_queue, task);
  }

#endif

  free(outputR);
  free(outputS);

  SYNC_TIMER_STOP(&args->localtimer.sync4);
  /* wait at a barrier until all threads add all join tasks */
  BARRIER_ARRIVE(args->barrier, rv);
  /* global barrier sync point-4 */
  SYNC_GLOBAL_STOP(&args->globaltimer->sync4, my_tid);

#ifndef NO_TIMING
  if (args->tid == 0) {
    END_MEASURE_PARTITION((args->timer)); /* partitioning finished */
  }
//    BEGIN_MEASURE_BUILD( (args->timer) )
#endif

#ifndef OVERVIEW // non-overview case
#ifdef NO_JOIN // partition only
#ifdef PERF_COUNTERS
  if (my_tid == 0) {
    PCM_stop();
    PCM_log("======= Partitioning phase profiling results ======\n");
    PCM_printResults();
    PCM_log("===================================================\n");
    PCM_cleanup();
  }
  /* Just to make sure we get consistent performance numbers */
  BARRIER_ARRIVE(args->barrier, rv);
#endif
#endif
#endif

#ifdef JOIN_RESULT_MATERIALIZE
  chainedtuplebuffer_t *chainedbuf = chainedtuplebuffer_init();
#else
  void *chainedbuf = NULL;
#endif

#ifndef OVERVIEW // non-overview case
#ifdef JOIN // everything
#ifdef PERF_COUNTERS
  if (my_tid == 0) {
    PCM_initPerformanceMonitor(NULL, NULL);
    PCM_start();
      auto curtime = std::chrono::steady_clock::now();
      string path = EXP_DIR "/results/breakdown/time_start_" + std::to_string(args->exp_id) + ".txt";
      auto fp = fopen(path.c_str(), "w");
      fprintf(fp, "%ld\n", curtime);
  }
#endif
#endif
#endif

#ifndef NO_TIMING
  if (args->tid == 0) {
    BEGIN_MEASURE_JOIN_ACC(args->timer)
  }
#endif

  while ((task = task_queue_get_atomic(join_queue))) {
    /* do the actual join. join method differs for different algorithms,
       i.e. bucket chaining, histogram-based, histogram-based with simd &
       prefetching  */
    results += args->join_function(&task->relR, &task->relS, &task->tmpR,
                                   chainedbuf, args->timer);
    args->parts_processed++;
  }
  args->result = results;

#ifdef JOIN_RESULT_MATERIALIZE
  args->threadresult->nresults = results;
  args->threadresult->threadid = my_tid;
  args->threadresult->results = (void *)chainedbuf;
#endif

  /* global finish time */
  SYNC_GLOBAL_STOP(&args->globaltimer->finish_time, my_tid);
  BARRIER_ARRIVE(args->barrier, rv)
#ifndef NO_TIMING
  /* Actually with this setup we're not timing build */
  if (my_tid == 0) {
    END_MEASURE_JOIN_ACC(args->timer)
    END_MEASURE(args->timer)
  }
#endif

#ifndef OVERVIEW // non-overview case
#ifdef PERF_COUNTERS
#ifdef JOIN // everything
  if (my_tid == 0) {
    PCM_stop();
      auto curtime = std::chrono::steady_clock::now();
      string path = EXP_DIR "/results/breakdown/time_end_" + std::to_string(args->exp_id) + ".txt";
      auto fp = fopen(path.c_str(), "w");
      fprintf(fp, "%ld\n", curtime);
    PCM_log("=========== Build+Probe profiling results =========\n");
    PCM_printResults();
    PCM_cleanup();
  }
#endif
  /* Just to make sure we get consistent performance numbers */
  BARRIER_ARRIVE(args->barrier, rv);
#endif
#endif

#ifdef OVERVIEW // overview counters
#ifdef PERF_COUNTERS
    if (my_tid == 0) {
        PCM_stop();
        auto curtime = std::chrono::steady_clock::now();
        string path = EXP_DIR "/results/breakdown/time_end_" + std::to_string(args->exp_id) + ".txt";
        auto fp = fopen(path.c_str(), "w");
        fprintf(fp, "%ld\n", curtime);
        PCM_log("=========== overview profiling results =========\n");
        PCM_printResults();
        PCM_cleanup();
    }
#endif
    /* Just to make sure we get consistent performance numbers */
    BARRIER_ARRIVE(args->barrier, rv);
#endif

  return 0;
}

/**
 * The template function for different joins: Basically each parallel radix join
 * has a initialization step, partitioning step and build-probe steps. All our
 * parallel radix implementations have exactly the same initialization and
 * partitioning steps. Difference is only in the build-probe step. Here are all
 * the parallel radix join implemetations and their Join (build-probe)
 * functions:
 *
 * - PRO,  Parallel Radix Join Optimized --> bucket_chaining_join()
 * - PRH,  Parallel Radix Join Histogram-based --> histogram_join()
 * - PRHO, Parallel Radix Histogram-based Optimized ->
 * histogram_optimized_join()
 */
result_t *join_init_run(relation_t *relR, relation_t *relS, JoinFunction jf,
                        param_t cmd_params) {
  int i, rv;
  pthread_t tid[nthreads];
  pthread_attr_t attr;
  pthread_barrier_t barrier;
  cpu_set_t set;
  arg_t args[nthreads];

  int32_t **histR, **histS;
  tuple_t *tmpRelR, *tmpRelS;
  int32_t numperthr[2];
  int64_t result = 0;

  /* task_queue_t * part_queue, * join_queue; */
  int numnuma = get_num_numa_regions();
  task_queue_t *part_queue[numnuma];
  task_queue_t *join_queue[numnuma];

#ifdef SKEW_HANDLING
  task_queue_t *skew_queue;
  task_t *skewtask = NULL;
  skew_queue = task_queue_init(FANOUT_PASS1);
#endif

  for (i = 0; i < numnuma; i++) {
    part_queue[i] = task_queue_init(FANOUT_PASS1);
    join_queue[i] = task_queue_init((1 << NUM_RADIX_BITS));
  }

  result_t *joinresult = 0;
  joinresult = (result_t *)malloc(sizeof(result_t));

#ifdef JOIN_RESULT_MATERIALIZE
  joinresult->resultlist =
      (threadresult_t *)malloc(sizeof(threadresult_t) * nthreads);
#endif

  /* allocate temporary space for partitioning */
  tmpRelR = (tuple_t *)alloc_aligned(relR->num_tuples * sizeof(tuple_t) +
                                     RELATION_PADDING);
  tmpRelS = (tuple_t *)alloc_aligned(relS->num_tuples * sizeof(tuple_t) +
                                     RELATION_PADDING);
  MALLOC_CHECK((tmpRelR && tmpRelS));
  /** Not an elegant way of passing whether we will numa-localize, but this
      feature is experimental anyway. */
  if (numalocalize) {
    uint64_t numwithpad;

    numwithpad = (relR->num_tuples * sizeof(tuple_t) + RELATION_PADDING) /
                 sizeof(tuple_t);
    numa_localize(tmpRelR, numwithpad, nthreads);

    numwithpad = (relS->num_tuples * sizeof(tuple_t) + RELATION_PADDING) /
                 sizeof(tuple_t);
    numa_localize(tmpRelS, numwithpad, nthreads);
  }

  /* allocate histograms arrays, actual allocation is local to threads */
  histR = (int32_t **)alloc_aligned(nthreads * sizeof(int32_t *));
  histS = (int32_t **)alloc_aligned(nthreads * sizeof(int32_t *));
  MALLOC_CHECK((histR && histS));

  rv = pthread_barrier_init(&barrier, NULL, nthreads);
  if (rv != 0) {
    MSG("[ERROR] Couldn't create the barrier\n");
    exit(EXIT_FAILURE);
  }

  pthread_attr_init(&attr);

#ifdef SYNCSTATS
  /* thread-0 keeps track of synchronization stats */
  args[0].globaltimer = (synctimer_t *)malloc(sizeof(synctimer_t));
#endif

  T_TIMER timer[nthreads]; // every thread has its own timer.

  uint64_t *startTS = new uint64_t();

  /* first assign chunks of relR & relS for each thread */
  numperthr[0] = relR->num_tuples / nthreads;
  numperthr[1] = relS->num_tuples / nthreads;
  for (i = 0; i < nthreads; i++) {
    int cpu_idx = get_cpu_id(i);
#ifdef DEBUG
    DEBUGMSG("Assigning thread-%d to CPU-%d\n", i, cpu_idx);
#endif
    CPU_ZERO(&set);
    CPU_SET(cpu_idx, &set);
    pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &set);
    args[i].timer = &timer[i];

#ifndef NO_TIMING
    args[i].timer->record_gap = cmd_params.gap;
#endif
    args[i].relR = relR->tuples + i * numperthr[0];
    args[i].tmpR = tmpRelR;
    args[i].histR = histR;

    args[i].relS = relS->tuples + i * numperthr[1];
    args[i].tmpS = tmpRelS;
    args[i].histS = histS;

    args[i].numR = (i == (nthreads - 1)) ? (relR->num_tuples - i * numperthr[0])
                                         : numperthr[0];
    args[i].numS = (i == (nthreads - 1)) ? (relS->num_tuples - i * numperthr[1])
                                         : numperthr[1];
    args[i].totalR = relR->num_tuples;
    args[i].totalS = relS->num_tuples;

    args[i].tid = i;
    args[i].part_queue = part_queue;
    args[i].join_queue = join_queue;
#ifdef SKEW_HANDLING
    args[i].skew_queue = skew_queue;
    args[i].skewtask = &skewtask;
#endif
    args[i].barrier = &barrier;
    args[i].join_function = jf;
    args[i].nthreads = nthreads;
    args[i].threadresult = &(joinresult->resultlist[i]);

    args[i].startTS = startTS;

    args[i].exp_id = cmd_params.exp_id;

    rv = pthread_create(&tid[i], &attr, prj_thread, (void *)&args[i]);
    if (rv) {
      MSG("[ERROR] return code from pthread_create() is %d\n", rv);
      exit(-1);
    }
  }
  /* wait for threads to finish */
  for (i = 0; i < nthreads; i++) {
    pthread_join(tid[i], NULL);
  }

  // compute results.
  for (i = 0; i < nthreads; i++) {
    result += args[i].result;
    MSG("Thread%d, produces %ld outputs\n", i, args[i].result);
#ifndef NO_TIMING
    merge(args[i].timer, relR, relS, startTS,
          cmd_params.ts == 0 ? 0 : cmd_params.window_size);
#endif
  }
  joinresult->totalresults = result;
  joinresult->nthreads = nthreads;

  // TODO: add a timer here, how to minus startTimer? Can I use t_timer.h
  int64_t processingTime = curtick() - *startTS;
#ifndef NO_TIMING
  MSG("With timing, Total processing time is: %f\n",
      processingTime / (2.1 * 1E6)); // cycle to ms
#endif

#ifndef NO_TIMING

  std::string name = "PRJ_" + std::to_string(cmd_params.exp_id);
  string path = EXP_DIR "/results/breakdown/" + name.append(".txt");
  auto fp = fopen(path.c_str(), "w");
  breakdown_global((relR->num_tuples + relS->num_tuples), nthreads,
                   args[0].timer,
                   cmd_params.ts == 0 ? 0 : cmd_params.window_size, fp);
  fclose(fp);
  sortRecords("PRJ", cmd_params.exp_id,
              cmd_params.ts == 0 ? 0 : cmd_params.window_size,
              (relR->num_tuples + relS->num_tuples), joinresult->totalresults);
#endif

#ifdef SYNCSTATS
  /* #define ABSDIFF(X,Y) (((X) > (Y)) ? ((X)-(Y)) : ((Y)-(X))) */
  fprintf(stdout, "TID JTASKS T1.1 T1.1-IDLE T1.2 T1.2-IDLE "
                  "T3 T3-IDLE T4 T4-IDLE T5 T5-IDLE\n");
  for (i = 0; i < nthreads; i++) {
    synctimer_t *glob = args[0].globaltimer;
    synctimer_t *local = &args[i].localtimer;
    fprintf(stdout,
            "%d %d %llu %llu %llu %llu %llu %llu %llu %llu "
            "%llu %llu\n",
            (i + 1), args[i].parts_processed, local->sync1[0],
            glob->sync1[0] - local->sync1[0], local->sync1[1] - glob->sync1[0],
            glob->sync1[1] - local->sync1[1], local->sync3 - glob->sync1[1],
            glob->sync3 - local->sync3, local->sync4 - glob->sync3,
            glob->sync4 - local->sync4, local->finish_time - glob->sync4,
            glob->finish_time - local->finish_time);
  }
#endif

  /* clean up */
  for (i = 0; i < nthreads; i++) {
    free(histR[i]);
    free(histS[i]);
  }
  free(histR);
  free(histS);

  for (i = 0; i < numnuma; i++) {
    task_queue_free(part_queue[i]);
    task_queue_free(join_queue[i]);
  }

#ifdef SKEW_HANDLING
  task_queue_free(skew_queue);
#endif
  free(tmpRelR);
  free(tmpRelS);
#ifdef SYNCSTATS
  free(args[0].globaltimer);
#endif

  return joinresult;
}

/** \copydoc PRO */
result_t *PRO(relation_t *relR, relation_t *relS, param_t cmd_params) {
  return join_init_run(relR, relS, bucket_chaining_join, cmd_params);
}

/** \copydoc PRH */
result_t *PRH(relation_t *relR, relation_t *relS, param_t cmd_params) {
  return join_init_run(relR, relS, histogram_join, cmd_params);
}

/** \copydoc PRHO */
result_t *PRHO(relation_t *relR, relation_t *relS, param_t cmd_params) {
  return join_init_run(relR, relS, histogram_optimized_join, cmd_params);
}

/** \copydoc RJ */
result_t *RJ_st(relation_t *relR, relation_t *relS, param_t cmd_params) {
  int64_t result = 0;
  result_t *joinresult;
  uint32_t i;

  relation_t *outRelR, *outRelS;

  outRelR = (relation_t *)malloc(sizeof(relation_t));
  outRelS = (relation_t *)malloc(sizeof(relation_t));

  joinresult = (result_t *)malloc(sizeof(result_t));
#ifdef JOIN_RESULT_MATERIALIZE
  joinresult->resultlist = (threadresult_t *)malloc(sizeof(threadresult_t));
#endif

  /* allocate temporary space for partitioning */
  /* TODO: padding problem */
  size_t sz = relR->num_tuples * sizeof(tuple_t) + RELATION_PADDING;
  outRelR->tuples = (tuple_t *)malloc(sz);
  outRelR->num_tuples = relR->num_tuples;

  sz = relS->num_tuples * sizeof(tuple_t) + RELATION_PADDING;
  outRelS->tuples = (tuple_t *)malloc(sz);
  outRelS->num_tuples = relS->num_tuples;

  /***** do the multi-pass partitioning *****/
#if NUM_PASSES == 1
  /* apply radix-clustering on relation R for pass-1 */
  radix_cluster_nopadding(outRelR, relR, 0, NUM_RADIX_BITS);
  relR = outRelR;

  /* apply radix-clustering on relation S for pass-1 */
  radix_cluster_nopadding(outRelS, relS, 0, NUM_RADIX_BITS);
  relS = outRelS;

#elif NUM_PASSES == 2
  /* apply radix-clustering on relation R for pass-1 */
  radix_cluster_nopadding(outRelR, relR, 0, NUM_RADIX_BITS / NUM_PASSES);

  /* apply radix-clustering on relation S for pass-1 */
  radix_cluster_nopadding(outRelS, relS, 0, NUM_RADIX_BITS / NUM_PASSES);

  /* apply radix-clustering on relation R for pass-2 */
  radix_cluster_nopadding(relR, outRelR, NUM_RADIX_BITS / NUM_PASSES,
                          NUM_RADIX_BITS - (NUM_RADIX_BITS / NUM_PASSES));

  /* apply radix-clustering on relation S for pass-2 */
  radix_cluster_nopadding(relS, outRelS, NUM_RADIX_BITS / NUM_PASSES,
                          NUM_RADIX_BITS - (NUM_RADIX_BITS / NUM_PASSES));

  /* clean up temporary relations */
  free(outRelR->tuples);
  free(outRelS->tuples);
  free(outRelR);
  free(outRelS);

#else
#error Only 1 or 2 pass partitioning is implemented, change NUM_PASSES!
#endif

  int *R_count_per_cluster = (int *)calloc((1 << NUM_RADIX_BITS), sizeof(int));
  int *S_count_per_cluster = (int *)calloc((1 << NUM_RADIX_BITS), sizeof(int));

  /* compute number of tuples per cluster */
  for (i = 0; i < relR->num_tuples; i++) {
    uint32_t idx = (relR->tuples[i].key) & ((1 << NUM_RADIX_BITS) - 1);
    R_count_per_cluster[idx]++;
  }
  for (i = 0; i < relS->num_tuples; i++) {
    uint32_t idx = (relS->tuples[i].key) & ((1 << NUM_RADIX_BITS) - 1);
    S_count_per_cluster[idx]++;
  }

#ifdef JOIN_RESULT_MATERIALIZE
  chainedtuplebuffer_t *chainedbuf = chainedtuplebuffer_init();
#else
  void *chainedbuf = NULL;
#endif

  /* build hashtable on inner */
  int r, s; /* start index of next clusters */
  r = s = 0;
  for (i = 0; i < (1 << NUM_RADIX_BITS); i++) {
    relation_t tmpR, tmpS;

    if (R_count_per_cluster[i] > 0 && S_count_per_cluster[i] > 0) {

      tmpR.num_tuples = R_count_per_cluster[i];
      tmpR.tuples = relR->tuples + r;
      r += R_count_per_cluster[i];

      tmpS.num_tuples = S_count_per_cluster[i];
      tmpS.tuples = relS->tuples + s;
      s += S_count_per_cluster[i];

      result += bucket_chaining_join(&tmpR, &tmpS, NULL, chainedbuf, nullptr);
    } else {
      r += R_count_per_cluster[i];
      s += S_count_per_cluster[i];
    }
  }

#ifdef JOIN_RESULT_MATERIALIZE
  threadresult_t *thrres = &(joinresult->resultlist[0]); /* single-thread */
  thrres->nresults = result;
  thrres->threadid = 0;
  thrres->results = (void *)chainedbuf;
#endif

  /* clean-up temporary buffers */
  free(S_count_per_cluster);
  free(R_count_per_cluster);

#if NUM_PASSES == 1
  /* clean up temporary relations */
  free(outRelR->tuples);
  free(outRelS->tuples);
  free(outRelR);
  free(outRelS);
#endif

  joinresult->totalresults = result;
  joinresult->nthreads = 1;

  return joinresult;
}

/** @} */
