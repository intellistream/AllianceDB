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

#include <pthread.h>  /* pthread_* */
#include <sched.h>    /* CPU_ZERO, CPU_SET */
#include <stdio.h>    /* printf */
#include <stdlib.h>   /* memalign */
#include <string.h>   /* memset */
#include <sys/time.h> /* gettimeofday */

#include "../utils/cpu_mapping.h" /* get_cpu_id */
#include "../utils/lock.h"        /* lock, unlock */
#include "no_partitioning_join.h"
#include "npj_params.h" /* constant parameters */
#include "npj_types.h"  /* bucket_t, hashtable_t, bucket_buffer_t */

#include "../utils/barrier.h" /* pthread_barrier_* */
//#include "../../utils/affinity.h"           /* pthread_attr_setaffinity_np */
#include "../timer/t_timer.h"
#include "../utils/generator.h" /* numa_localize() */
#include "../utils/perf_counters.h"
#include "common_functions.h"
#include <sched.h>
#include "unistd.h"

#ifdef JOIN_RESULT_MATERIALIZE

#include "../timer/clock.h"
#include "../utils/tuple_buffer.h" /* for materialization */

#endif

#ifndef BARRIER_ARRIVE
/** barrier wait macro */
#define BARRIER_ARRIVE(B, RV)                                                  \
  RV = pthread_barrier_wait(B);                                                \
  if (RV != 0 && RV != PTHREAD_BARRIER_SERIAL_THREAD) {                        \
    MSG("Couldn't wait on barrier\n");                                         \
    exit(EXIT_FAILURE);                                                        \
  }
#endif

/**
 * \ingroup common arguments to the threads
 */
struct arg_t {

  int32_t tid;
  hashtable_t *ht;

  relation_t relR;
  relation_t relS;
  pthread_barrier_t *barrier;
  int64_t nthreads;

  /* results of the thread */
  threadresult_t *threadresult;

  //#ifndef NO_TIMING
  T_TIMER *timer;
  //#endif
  int64_t result;

  uint64_t *startTS;

  int exp_id; // for perf stat
};

/** @} */

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
void init_bucket_buffer(bucket_buffer_t **ppbuf) {
  bucket_buffer_t *overflowbuf;
  overflowbuf = (bucket_buffer_t *)malloc(sizeof(bucket_buffer_t));
  overflowbuf->count = 0;
  overflowbuf->next = NULL;

  *ppbuf = overflowbuf;
}

/**
 * @defgroup NPO The No Partitioning Optimized Join Implementation
 * @{
 */

/** \copydoc NPO_st */
result_t *NPO_st(relation_t *relR, relation_t *relS, param_t cmd_params) {
  hashtable_t *ht;
  int64_t result = 0;
  result_t *joinresult;

  uint32_t nbuckets = (relR->num_tuples / BUCKET_SIZE);
  allocate_hashtable(&ht, nbuckets);

  joinresult = (result_t *)malloc(sizeof(result_t));
#ifdef JOIN_RESULT_MATERIALIZE
  joinresult->resultlist = (threadresult_t *)malloc(sizeof(threadresult_t));
#endif

  build_hashtable_st(ht, relR);

#ifdef JOIN_RESULT_MATERIALIZE
  chainedtuplebuffer_t *chainedbuf = chainedtuplebuffer_init();
#else
  void *chainedbuf = NULL;
#endif

  result = probe_hashtable(ht, relS, chainedbuf, nullptr);

#ifdef JOIN_RESULT_MATERIALIZE
  threadresult_t *thrres = &(joinresult->resultlist[0]); /* single-thread */
  thrres->nresults = result;
  thrres->threadid = 0;
  thrres->results = (void *)chainedbuf;
#endif

  destroy_hashtable(ht);
  joinresult->totalresults = result;
  joinresult->nthreads = 1;

  return joinresult;
}

/**
 * Just a wrapper to call the build and probe for each thread.
 *
 * @param param the parameters of the thread, i.e. tid, ht, reln, ...
 *
 * @return
 */
void *npo_thread(void *param) {
  int rv;
  arg_t *args = (arg_t *)param;

  /* allocate overflow buffer for each thread */
  bucket_buffer_t *overflowbuf;
  init_bucket_buffer(&overflowbuf);

  /* wait at a barrier until each thread starts and start T_TIMER */
  BARRIER_ARRIVE(args->barrier, rv);
#ifndef NO_TIMING
  if (args->tid == 0) {
    *args->startTS = curtick(); // assign the start timestamp
    START_MEASURE((args->timer))
    BEGIN_MEASURE_BUILD((args->timer)) /* build start */
  }
#endif

  //#ifdef PERF_COUNTERS
  //    if(args->tid == 0){
  //            PCM_initPerformanceMonitor(NULL, NULL);
  //            PCM_start();
  //        }
  //#endif

  /* insert tuples from the assigned part of relR to the ht */
  build_hashtable_mt(args->ht, &args->relR, &overflowbuf);

  // we don't care build phase as it's minor according to our experimental
  // results.
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

    BARRIER_ARRIVE(args->barrier, rv)
#ifndef NO_TIMING
  /* wait at a barrier until each thread completes build phase */
  if (args->tid == 0) {
    END_MEASURE_BUILD((args->timer))
  }
#endif


#ifdef JOIN_RESULT_MATERIALIZE
  chainedtuplebuffer_t *chainedbuf = chainedtuplebuffer_init();
#else

  void *chainedbuf = NULL;
#endif

#ifndef NO_TIMING
  BARRIER_ARRIVE(args->barrier, rv)
  if (args->tid == 0) {
    BEGIN_MEASURE_JOIN_ACC(args->timer)
  }
#endif

#ifdef PERF_COUNTERS
  if (args->tid == 0) {
    PCM_initPerformanceMonitor(NULL, NULL);
    PCM_start();
  }
   BARRIER_ARRIVE(args->barrier, rv);
#endif

  /* probe for matching tuples from the assigned part of relS */
  args->result =
      probe_hashtable(args->ht, &args->relS, chainedbuf, args->timer);

#ifdef JOIN_RESULT_MATERIALIZE
  args->threadresult->nresults = args->result;
  args->threadresult->threadid = args->tid;
  args->threadresult->results = (void *)chainedbuf;
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

#ifndef NO_TIMING
  /* wait at a barrier until each thread completes join phase */
  BARRIER_ARRIVE(args->barrier, rv)
  if (args->tid == 0) {
    END_MEASURE_JOIN_ACC(args->timer)
    END_MEASURE(args->timer)
  }
#endif

  /* clean-up the overflow buffers */
  free_bucket_buffer(overflowbuf);
  return 0;
}

/**
 * Multi-thread data partition.
 * @param relR
 * @param relS
 * @param nthreads
 * @param ht
 * @param numR
 * @param numS
 * @param numRthr
 * @param numSthr
 * @param i
 * @param rv
 * @param set
 * @param args
 * @param tid
 * @param attr
 * @param barrier
 * @param joinresult
 */
void np_distribute(const relation_t *relR, const relation_t *relS, int nthreads,
                   hashtable_t *ht, int32_t numR, int32_t numS, int32_t numRthr,
                   int32_t numSthr, int i, int rv, cpu_set_t &set, arg_t *args,
                   pthread_t *tid, pthread_attr_t &attr, barrier_t &barrier,
                   const result_t *joinresult, T_TIMER *timer, int exp_id,
                   int group_size, uint64_t *startTS, int record_gap) {
  for (i = 0; i < nthreads; i++) {
    int cpu_idx = get_cpu_id(i);

    DEBUGMSG("Assigning thread-%d to CPU-%d\n", i, cpu_idx)
    CPU_ZERO(&set);
    CPU_SET(cpu_idx, &set);
    pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &set);

    args[i].tid = i;
    args[i].timer = &timer[i];
    args[i].ht = ht;
    args[i].barrier = &barrier;
    args[i].startTS = startTS;

    args[i].exp_id = exp_id;

    /* assing part of the relR for next thread */
    args[i].relR.num_tuples = (i == (nthreads - 1)) ? numR : numRthr;
    args[i].relR.tuples = relR->tuples + numRthr * i;
    numR -= numRthr;

#ifndef NO_TIMING
    args[i].timer->record_gap = record_gap;
    DEBUGMSG(" record_gap:%d\n", args[i].timer->record_gap);
#endif

    DEBUGMSG("Assigning #R=%d to thread %d\n", args[i].relR.num_tuples, i)

    /* assing part of the relS for next thread */
    args[i].relS.num_tuples = (i == (nthreads - 1)) ? numS : numSthr;
    args[i].relS.tuples = relS->tuples + numSthr * i;
    numS -= numSthr;

    DEBUGMSG("Assigning #S=%d to thread %d\n", args[i].relS.num_tuples, i)

    args[i].threadresult = &(joinresult->resultlist[i]);

    rv = pthread_create(&tid[i], &attr, npo_thread, (void *)&args[i]);
    if (rv) {
      MSG("ERROR; return code from pthread_create() is %d\n", rv);
      exit(-1);
    }
  }
}

/** \copydoc NPO */
result_t *NPO(relation_t *relR, relation_t *relS, param_t cmd_params) {
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
  joinresult = (result_t *)malloc(sizeof(result_t));

#ifdef JOIN_RESULT_MATERIALIZE
  joinresult->resultlist =
      (threadresult_t *)malloc(sizeof(threadresult_t) * nthreads);
#endif

  //#ifndef NO_TIMING
  T_TIMER timer[nthreads]; // every thread has its own timer.
  //#endif

  uint64_t *startTS = new uint64_t();
  uint32_t nbuckets = (relR->num_tuples / BUCKET_SIZE);
  allocate_hashtable(&ht, nbuckets);

  numR = relR->num_tuples;
  numS = relS->num_tuples;
  numRthr = numR / nthreads;
  numSthr = numS / nthreads;

  rv = pthread_barrier_init(&barrier, NULL, nthreads);
  if (rv != 0) {
    MSG("Couldn't create the barrier\n");
    exit(EXIT_FAILURE);
  }

  pthread_attr_init(&attr);
  np_distribute(relR, relS, nthreads, ht, numR, numS, numRthr, numSthr, i, rv,
                set, args, tid, attr, barrier, joinresult, timer,
                cmd_params.exp_id, cmd_params.group_size, startTS,
                cmd_params.gap);
  /* wait for threads to finish */
  for (i = 0; i < nthreads; i++) {
    pthread_join(tid[i], NULL);
  }

  // compute results.
  for (i = 0; i < nthreads; i++) {
    result += args[i].result;
    MSG("Thread[%d], produces %ld outputs\n", i, args[i].result);
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
  /* now print the timing results: */

  std::string name = "NPJ_" + std::to_string(cmd_params.exp_id);
  string path = "/data1/xtra/results/breakdown/" + name.append(".txt");
  auto fp = fopen(path.c_str(), "w");
  breakdown_global((numR + numS), nthreads, args[0].timer,
                   cmd_params.ts == 0 ? 0 : cmd_params.window_size, fp);
  fclose(fp);
  sortRecords("NPJ", cmd_params.exp_id,
              cmd_params.ts == 0 ? 0 : cmd_params.window_size, (numR + numS),
              joinresult->totalresults);
#endif
  //    for (i = 0; i < nthreads; i++) {
  //        pthread_join(tid[i], NULL);
  //        /* sum up results */
  //        result += args[i].nthreads;
  //    }
  //    joinresult->totalresults = result;
  //    joinresult->nthreads = nthreads;
  //
  //
  //#ifndef NO_TIMING
  //    /* now print the timing results: */
  //    print_timing(result, &timer);
  //#endif
  destroy_hashtable(ht);
  return joinresult;
}

/** @}*/
