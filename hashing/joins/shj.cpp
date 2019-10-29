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

#include "shj.h"
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
#include "../utils/xxhash64.h"
#include <sched.h>
#include <list>
#include <iostream>
/** @} */
/**
 * \ingroup arguments to the threads
 */

struct list {
    std::list<int> *relR_list;
    std::list<int> *relS_list;
};

struct arg_t {
    int32_t tid;
    hashtable_t *htR;
    hashtable_t *htS;

    //used in JM
    relation_t relR;//start point of reading relation.
    relation_t relS;//start point of reading relation.

    //used in JB
    struct list *list;

    pthread_barrier_t *barrier;
    int64_t num_results;

    /* results of the thread */
    threadresult_t *threadresult;

#ifndef NO_TIMING
    /* stats about the thread */
    uint64_t timer1, timer2, timer3, build_timer = 0;
    struct timeval start, end;
    uint64_t progressivetimer[3];//array of progressive timer.
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
long
_SHJ_JB_st(int32_t tid, hashtable_t *ht_R, hashtable_t *ht_S, relation_t *rel_R, relation_t *rel_S,
           std::list<int> list_r, std::list<int> list_s, void *pVoid,
           uint64_t *pre_timer, uint64_t *acc_timer, uint64_t *progressivetimer) {

    const uint32_t hashmask_R = ht_R->hash_mask;
    const uint32_t skipbits_R = ht_R->skip_bits;

    const uint32_t hashmask_S = ht_S->hash_mask;
    const uint32_t skipbits_S = ht_S->skip_bits;

    int64_t matches = 0;//number of matches.

    std::list<int>::const_iterator iterator;
    for (iterator = list_r.begin(); iterator != list_r.end(); ++iterator) {
//        std::cout << *iterator;

        if (tid == 0) {
            startTimer(pre_timer);
        }
        build_hashtable_single(ht_R, rel_R, *iterator, hashmask_R, skipbits_R);//(1)
        if (tid == 0) {
            accTimer(pre_timer, acc_timer); /* build time */
        }
        if (tid == 0) {
            proble_hashtable_single_measure(ht_S, rel_R, *iterator, hashmask_S, skipbits_S, &matches,
                                            progressivetimer);//(2)
        } else {
            matches += proble_hashtable_single(ht_S, rel_R, *iterator, hashmask_S, skipbits_S);//(4)
        }

    }

    for (iterator = list_s.begin(); iterator != list_s.end(); ++iterator) {
//        std::cout << *iterator;

        if (tid == 0) {
            startTimer(pre_timer);
        }
        build_hashtable_single(ht_S, rel_S, *iterator, hashmask_S, skipbits_S);//(3)
        if (tid == 0) {
            accTimer(pre_timer, acc_timer); /* build time */
        }
        if (tid == 0) {
            proble_hashtable_single_measure(ht_R, rel_S, *iterator, hashmask_R, skipbits_R, &matches,
                                            progressivetimer);//(4)
        } else {
            matches += proble_hashtable_single(ht_R, rel_S, *iterator, hashmask_R, skipbits_R);//(4)
        }
    }

    return matches;
}

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
long
_SHJ_st(int32_t tid, hashtable_t *ht_R, hashtable_t *ht_S, relation_t *rel_R, relation_t *rel_S, void *pVoid,
        uint64_t *pre_timer, uint64_t *acc_timer, uint64_t *progressivetimer) {
    uint32_t index_R = 0;//index of rel_R
    uint32_t index_S = 0;//index of rel_S

    const uint32_t hashmask_R = ht_R->hash_mask;
    const uint32_t skipbits_R = ht_R->skip_bits;

    const uint32_t hashmask_S = ht_S->hash_mask;
    const uint32_t skipbits_S = ht_S->skip_bits;

    int64_t matches = 0;//number of matches.

    do {
        if (index_R < rel_R->num_tuples) {
            if (tid == 0) {
                startTimer(pre_timer);
            }
            build_hashtable_single(ht_R, rel_R, index_R, hashmask_R, skipbits_R);//(1)
            if (tid == 0) {
                accTimer(pre_timer, acc_timer); /* build time */
            }
            if (tid == 0) {
                proble_hashtable_single_measure(ht_S, rel_R, index_R, hashmask_S, skipbits_S, &matches,
                                                progressivetimer);//(2)
            } else {
                matches += proble_hashtable_single(ht_R, rel_S, index_S, hashmask_R, skipbits_R);//(4)
            }
            index_R++;
        }

        if (index_S < rel_S->num_tuples) {
            if (tid == 0) {
                startTimer(pre_timer);
            }
            build_hashtable_single(ht_S, rel_S, index_S, hashmask_S, skipbits_S);//(3)
            if (tid == 0) {
                accTimer(pre_timer, acc_timer); /* build time */
            }
            if (tid == 0) {
                proble_hashtable_single_measure(ht_R, rel_S, index_S, hashmask_R, skipbits_R, &matches,
                                                progressivetimer);//(4)
            } else {
                matches += proble_hashtable_single(ht_R, rel_S, index_S, hashmask_R, skipbits_R);//(4)
            }
            index_S++;
        }
    } while (index_R < rel_R->num_tuples || index_S < rel_S->num_tuples);

    return matches;
}

/**
 * Single thread SHJ
 * @param relR
 * @param relS
 * @param nthreads
 * @return
 */
result_t *
SHJ_st(relation_t *relR, relation_t *relS, int nthreads) {
    hashtable_t *htR;
    hashtable_t *htS;
    int64_t result = 0;
    result_t *joinresult;

#ifndef NO_TIMING
    struct timeval start, end;
    uint64_t timer1, timer2, timer3, buildtimer = 0;//buildtimer is accumulated.
    uint64_t progressivetimer[3];//array of progressive timer.
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
    startTimer(&progressivetimer[0]);
    startTimer(&progressivetimer[1]);
    startTimer(&progressivetimer[2]);
    timer3 = 0; /* no partitioning */
#endif

#ifdef JOIN_RESULT_MATERIALIZE
    chainedtuplebuffer_t * chainedbuf = chainedtuplebuffer_init();
#else
    void *chainedbuf = NULL;
#endif

    result = _SHJ_st(0, htR, htS, relR, relS, chainedbuf, &timer2, &buildtimer,
                     progressivetimer);//build and probe at the same time.

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
    print_timing(timer1, buildtimer, timer3, relS->num_tuples, result, &start, &end, progressivetimer);
#endif

    destroy_hashtable(htR);
    destroy_hashtable(htS);

    joinresult->totalresults = result;
    joinresult->nthreads = 1;

    return joinresult;
}

/**
 * Just a wrapper to call the _shj_st
 *
 * @param param the parameters of the thread, i.e. tid, ht, reln, ...
 *
 * @return
 */
void *
shj_JM_thread_task(void *param) {
    int rv;
    arg_t *args = (arg_t *) param;

//    /* allocate overflow buffer for each thread */
//    bucket_buffer_t *overflowbuf;
//    init_bucket_buffer(&overflowbuf);

#ifdef PERF_COUNTERS
    if(args->tid == 0){
        PCM_initPerformanceMonitor(NULL, NULL);
        PCM_start();
    }
#endif
//
//    /* wait at a barrier until each thread starts and start timer */
//    BARRIER_ARRIVE(args->barrier, rv);

#ifndef NO_TIMING
    /* the first thread checkpoints the start time */
    if (args->tid == 0) {
        args->build_timer = 0;
        gettimeofday(&args->start, NULL);
        startTimer(&args->timer1);
        startTimer(&args->timer2);
        startTimer(&args->progressivetimer[0]);
        startTimer(&args->progressivetimer[1]);
        startTimer(&args->progressivetimer[2]);
        args->timer3 = 0; /* no partitionig phase */
    }
#endif

    //allocate two hashtables.
    uint32_t nbucketsR = (args->relR.num_tuples / BUCKET_SIZE);
    allocate_hashtable(&args->htR, nbucketsR);

    uint32_t nbucketsS = (args->relS.num_tuples / BUCKET_SIZE);
    allocate_hashtable(&args->htS, nbucketsS);

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

#ifdef JOIN_RESULT_MATERIALIZE
    chainedtuplebuffer_t * chainedbuf = chainedtuplebuffer_init();
#else
    void *chainedbuf = NULL;
#endif

    args->num_results = _SHJ_st(
            args->tid, args->htR,
            args->htS, &args->relR, &args->relS,
            chainedbuf, &args->timer2, &args->build_timer,
            args->progressivetimer);//build and probe at the same time.
#ifdef JOIN_RESULT_MATERIALIZE
    args->threadresult->nresults = args->num_results;
    args->threadresult->threadid = args->tid;
    args->threadresult->results  = (void *) chainedbuf;
#endif

#ifndef NO_TIMING
//    /* for a reliable timing we have to wait until all finishes */
//    BARRIER_ARRIVE(args->barrier, rv);

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

    return 0;
}


/**
 * Just a wrapper to call the _shj_st
 *
 * @param param the parameters of the thread, i.e. tid, ht, reln, ...
 *
 * @return
 */
void *
shj_JB_thread_task(void *param) {
    int rv;
    arg_t *args = (arg_t *) param;

#ifdef PERF_COUNTERS
    if(args->tid == 0){
        PCM_initPerformanceMonitor(NULL, NULL);
        PCM_start();
    }
#endif
//
//    /* wait at a barrier until each thread starts and start timer */
//    BARRIER_ARRIVE(args->barrier, rv);

#ifndef NO_TIMING
    /* the first thread checkpoints the start time */
    if (args->tid == 0) {
        args->build_timer = 0;
        gettimeofday(&args->start, NULL);
        startTimer(&args->timer1);
        startTimer(&args->timer2);
        startTimer(&args->progressivetimer[0]);
        startTimer(&args->progressivetimer[1]);
        startTimer(&args->progressivetimer[2]);
        args->timer3 = 0; /* no partitionig phase */
    }
#endif

    //allocate two hashtables.
    uint32_t nbucketsR = (args->relR.num_tuples / BUCKET_SIZE);
    allocate_hashtable(&args->htR, nbucketsR);

    uint32_t nbucketsS = (args->relS.num_tuples / BUCKET_SIZE);
    allocate_hashtable(&args->htS, nbucketsS);

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

#ifdef JOIN_RESULT_MATERIALIZE
    chainedtuplebuffer_t * chainedbuf = chainedtuplebuffer_init();
#else
    void *chainedbuf = NULL;
#endif

    args->num_results = _SHJ_JB_st(
            args->tid, args->htR,
            args->htS, &args->relR, &args->relS, *args->list->relR_list, *args->list->relS_list,
            chainedbuf, &args->timer2, &args->build_timer,
            args->progressivetimer);//build and probe at the same time.
#ifdef JOIN_RESULT_MATERIALIZE
    args->threadresult->nresults = args->num_results;
    args->threadresult->threadid = args->tid;
    args->threadresult->results  = (void *) chainedbuf;
#endif

#ifndef NO_TIMING
//    /* for a reliable timing we have to wait until all finishes */
//    BARRIER_ARRIVE(args->barrier, rv);

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

    return 0;
}

inline bool last_thread(int i, int nthreads) {
    return i == (nthreads - 1);
}


/**
 * Multi-thread data partition, Join-Matrix Model, no-(physical)-partition method.
 * When we know size of input relation,
 * the optimal way is
 * (1) partition long relation
 * and (2) replicate short relation.
 */
void
jm_np_distribute(const relation_t *relR, const relation_t *relS,
                 int nthreads, int i, int rv, cpu_set_t &set,
                 struct arg_t *args, pthread_t *tid,
                 pthread_attr_t &attr,
                 result_t *joinresult) {

    int32_t numR, numS, numRthr, numSthr; /* total and per thread num */
    numR = relR->num_tuples;
    numS = relS->num_tuples;

    numSthr = numS / nthreads;//(1)
    numRthr = numR;//(2)

    for (i = 0; i < nthreads; i++) {
        int cpu_idx = get_cpu_id(i);
        DEBUGMSG(1, "Assigning thread-%d to CPU-%d\n", i, cpu_idx);
        CPU_ZERO(&set);
        CPU_SET(cpu_idx, &set);
        pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &set);
        args[i].tid = i;

        /* replicate relR for next thread */
        args[i].relR.num_tuples = numR;
        args[i].relR.tuples = relR->tuples;//configure pointer of start point.

        /* assign part of the relS for next thread */
        args[i].relS.num_tuples = (last_thread(i, nthreads)) ? numS : numSthr;
        args[i].relS.tuples = relS->tuples + numSthr * i;
        numS -= numSthr;

        args[i].threadresult = &(joinresult->resultlist[i]);

        rv = pthread_create(&tid[i], &attr, shj_JM_thread_task, (void *) &args[i]);
        if (rv) {
            printf("ERROR; return code from pthread_create() is %d\n", rv);
            exit(-1);
        }
    }
}

/**
 * Multi-thread data partition, Join-Biclique Model, no-(physical)-partition method.
 * Let's assume extreme hashing.
 */
void
jb_np_distribute(const relation_t *relR, const relation_t *relS,
                 int nthreads, int i, int rv, cpu_set_t &set,
                 struct arg_t *args, pthread_t *tid,
                 pthread_attr_t &attr,
                 result_t *joinresult) {

    int32_t numR, numS; /* total num */
    numR = relR->num_tuples;
    numS = relS->num_tuples;


    int j;
    /* assign task for next thread by hashing */
    for (j = 0; j < numR; j++) {
        intkey_t idx = relR->tuples[j].key % nthreads;
        args[idx].list->relR_list->push_back(j);
    }

    for (j = 0; j < numS; j++) {
        intkey_t idy = relS->tuples[j].key % nthreads;
        args[idy].list->relS_list->push_back(j);
//        printf("args[%d].relR_list : %zu\n", idy, args[idy].list->relS_list->size());
    }

    for (i = 0; i < nthreads; i++) {
        int cpu_idx = get_cpu_id(i);
        DEBUGMSG(1, "Assigning thread-%d to CPU-%d\n", i, cpu_idx);
        CPU_ZERO(&set);
        CPU_SET(cpu_idx, &set);
        pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &set);
        args[i].tid = i;
        args[i].relR.num_tuples = args[i].list->relR_list->size();
        args[i].relR.tuples = relR->tuples;//configure pointer of start point.
        args[i].relS.num_tuples = args[i].list->relS_list->size();
        args[i].relS.tuples = relS->tuples;//configure pointer of start point.
        args[i].threadresult = &(joinresult->resultlist[i]);
        rv = pthread_create(&tid[i], &attr, shj_JB_thread_task, (void *) &args[i]);
        if (rv) {
            printf("ERROR; return code from pthread_create() is %d\n", rv);
            exit(-1);
        }
    }
}


/**
 * Data partition (JM model) then invovle _SHJ_st.
 * @param relR
 * @param relS
 * @param nthreads
 * @return
 */
result_t *
SHJ_JB_NP(relation_t *relR, relation_t *relS, int nthreads) {
    int64_t result = 0;

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

    rv = pthread_barrier_init(&barrier, NULL, nthreads);
    if (rv != 0) {
        printf("Couldn't create the barrier\n");
        exit(EXIT_FAILURE);
    }

    pthread_attr_init(&attr);

    for (i = 0; i < nthreads; i++) {
        args[i].list = new list();
        args[i].list->relR_list = new std::list<int>();
        args[i].list->relS_list = new std::list<int>();
    }

    jb_np_distribute(relR, relS, nthreads,
                     i, rv, set, args, tid, attr, joinresult);

    for (i = 0; i < nthreads; i++) {
        pthread_join(tid[i], NULL);
        /* sum up results */
        result += args[i].num_results;
    }
    joinresult->totalresults = result;
    joinresult->nthreads = nthreads;


#ifndef NO_TIMING
    /* now print the timing results: */
    print_timing(args[0].timer1, args[0].build_timer, args[0].timer3,
                 relS->num_tuples, result,
                 &args[0].start, &args[0].end, args[0].progressivetimer);
#endif

    return joinresult;
}

/**
 * Data partition (JM model) then invovle _SHJ_st.
 * @param relR
 * @param relS
 * @param nthreads
 * @return
 */
result_t *
SHJ_JM_NP(relation_t *relR, relation_t *relS, int nthreads) {
    int64_t result = 0;

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

    rv = pthread_barrier_init(&barrier, NULL, nthreads);
    if (rv != 0) {
        printf("Couldn't create the barrier\n");
        exit(EXIT_FAILURE);
    }

    pthread_attr_init(&attr);
    jm_np_distribute(relR, relS, nthreads,
                     i, rv, set, args, tid, attr, joinresult);

    for (i = 0; i < nthreads; i++) {
        pthread_join(tid[i], NULL);
        /* sum up results */
        result += args[i].num_results;
    }
    joinresult->totalresults = result;
    joinresult->nthreads = nthreads;


#ifndef NO_TIMING
    /* now print the timing results: */
    print_timing(args[0].timer1, args[0].build_timer, args[0].timer3,
                 relS->num_tuples, result,
                 &args[0].start, &args[0].end, args[0].progressivetimer);
#endif

    return joinresult;
}

/** @}*/
