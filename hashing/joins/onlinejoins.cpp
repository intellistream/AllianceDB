/**
 * @file    onlinejoins_st.c
 * @author  Shuhao Zhang <tonyzhang19900609@gmail.com>
 * @date    Sat Oct 26 2019
 * @version
 *
 * @brief  The implementation of online-join algorithms: SHJ,..
 *
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <pthread.h>            /* pthread_* */
#include <string.h>             /* memset */
#include <stdio.h>              /* printf */
#include <stdlib.h>             /* memalign */
#include <sys/time.h>           /* gettimeofday */
#include <zconf.h>
#include "onlinejoins.h"
#include "../utils/t_timer.h"  /* startTimer, stopTimer */
#include "../utils/barrier.h"
#include "../helper/launcher.h"
#include "shj_struct.h"
#include "../helper/localjoiner.h"
#include "../helper/thread_task.h"

#ifdef PERF_COUNTERS
#include "perf_counters.h"      /* PCM_x */
#endif


/** @} */

void initialize(int nthreads, const t_param &param) {
    int rv;
    rv = pthread_barrier_init(param.barrier, NULL, nthreads);
    if (rv != 0) {
        printf("Couldn't create the barrier\n");
        exit(EXIT_FAILURE);
    }
    pthread_attr_init(param.attr);
}

t_param &finishing(int nthreads, t_param &param) {
    int i;

//    sleep(60);
    for (i = 0; i < nthreads; i++) {
        pthread_join(param.tid[i], NULL);

        /* sum up results */
        param.result += param.args[i].results;
    }
    param.joinresult->totalresults = param.result;
    param.joinresult->nthreads = nthreads;
    return param;
}


result_t *
SHJ_JB_NP(relation_t *relR, relation_t *relS, int nthreads) {
    t_param param(nthreads);
#ifndef NO_TIMING
    T_TIMER timer;
#endif
#ifdef JOIN_RESULT_MATERIALIZE
    joinresult->resultlist = (threadresult_t *) malloc(sizeof(threadresult_t)
                                                       * nthreads);
#endif
    initialize(nthreads, param);
    param.fetcher = type_JB_NP_Fetcher;//new JB_NP_Fetcher(nthreads, relR, relS);
    param.shuffler = new HashShuffler(nthreads, relR, relS);
    param.joiner = type_SHJJoiner;//new SHJJoiner();
    LAUNCH(nthreads, relR, relS, param, timer, THREAD_TASK_SHUFFLE)
    param = finishing(nthreads, param);
#ifndef NO_TIMING
    /* now print the timing results: */
    print_timing(relS->num_tuples, param.result, &timer);
#endif
    return param.joinresult;
}

result_t *
SHJ_JBCR_NP(relation_t *relR, relation_t *relS, int nthreads) {
    t_param param(nthreads);
#ifndef NO_TIMING
    T_TIMER timer;
#endif
#ifdef JOIN_RESULT_MATERIALIZE
    joinresult->resultlist = (threadresult_t *) malloc(sizeof(threadresult_t)
                                                       * nthreads);
#endif
    initialize(nthreads, param);
    param.fetcher = type_JB_NP_Fetcher;//new JB_NP_Fetcher(nthreads, relR, relS);
    param.shuffler = new ContRandShuffler(nthreads, relR, relS);
    param.joiner = type_SHJJoiner;//new SHJJoiner();
    LAUNCH(nthreads,relR, relS, param, timer, THREAD_TASK_SHUFFLE)
    param = finishing(nthreads, param);
#ifndef NO_TIMING
    /* now print the timing results: */
    print_timing(relS->num_tuples, param.result, &timer);
#endif
    return param.joinresult;
}


result_t *
SHJ_JM_P(relation_t *relR, relation_t *relS, int nthreads) {
    t_param param(nthreads);
#ifndef NO_TIMING
    T_TIMER timer;
#endif

#ifdef JOIN_RESULT_MATERIALIZE
    joinresult->resultlist = (threadresult_t *) malloc(sizeof(threadresult_t)
                                                       * nthreads);
#endif
    initialize(nthreads, param);
    param.fetcher = type_JM_NP_Fetcher; //new JM_NP_Fetcher(nthreads, relR, relS);
    //no shuffler is required for JM mode.
    param.joiner = type_SHJJoiner;//new SHJJoiner();
    LAUNCH(nthreads,relR, relS, param, timer, THREAD_TASK_NOSHUFFLE)
    param = finishing(nthreads, param);
#ifndef NO_TIMING
    /* now print the timing results: */
    print_timing(relS->num_tuples, param.result, &timer);
#endif
    return param.joinresult;
}

result_t *
SHJ_JM_NP(relation_t *relR, relation_t *relS, int nthreads) {
    t_param param(nthreads);
#ifndef NO_TIMING
    T_TIMER timer;
#endif

#ifdef JOIN_RESULT_MATERIALIZE
    joinresult->resultlist = (threadresult_t *) malloc(sizeof(threadresult_t)
                                                       * nthreads);
#endif
    initialize(nthreads, param);
    param.fetcher = type_JM_NP_Fetcher;//new JM_NP_Fetcher(nthreads, relR, relS);
    //no shuffler is required for JM mode.
    param.joiner = type_SHJJoiner;//new SHJJoiner();
    LAUNCH(nthreads, relR, relS,param, timer, THREAD_TASK_NOSHUFFLE)
    param = finishing(nthreads, param);
#ifndef NO_TIMING
    /* now print the timing results: */
    print_timing(relS->num_tuples, param.result, &timer);
#endif
    return param.joinresult;
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

    t_param tParam(1);

#ifdef JOIN_RESULT_MATERIALIZE
    joinresult->resultlist = (threadresult_t *) malloc(sizeof(threadresult_t));
#endif

#ifndef NO_TIMING
    T_TIMER timer;
    START_MEASURE(timer)
#endif

#ifdef JOIN_RESULT_MATERIALIZE
    chainedtuplebuffer_t * chainedbuf = chainedtuplebuffer_init();
#else
    void *chainedbuf = NULL;
#endif

    // No distribution nor partition
    // Directly call the local SHJ joiner.
    tParam.result = shj(0, relR, relS, chainedbuf, &timer);//build and probe at the same time.

#ifdef JOIN_RESULT_MATERIALIZE
    threadresult_t * thrres = &(joinresult->resultlist[0]);/* single-thread */
    thrres->nresults = result;
    thrres->threadid = 0;
    thrres->results  = (void *) chainedbuf;
#endif

#ifndef NO_TIMING
    END_MEASURE(timer)
    /* now print the timing results: */
    print_timing(relS->num_tuples, tParam.result, &timer);
#endif

    tParam.joinresult->totalresults = tParam.result;
    tParam.joinresult->nthreads = 1;

    return tParam.joinresult;
}

result_t *
SHJ_HS_NP(relation_t *relR, relation_t *relS, int nthreads) {
    t_param param(nthreads);
#ifndef NO_TIMING
    T_TIMER timer;
#endif
#ifdef JOIN_RESULT_MATERIALIZE
    joinresult->resultlist = (threadresult_t *) malloc(sizeof(threadresult_t)
                                                       * nthreads);
#endif
    initialize(nthreads, param);
    param.fetcher = type_HS_NP_Fetcher;//new HS_NP_Fetcher(nthreads, relR, relS);
    param.shuffler = new HSShuffler(nthreads, relR, relS);
    param.joiner = type_SHJJoiner;//new SHJJoiner();
    LAUNCH(nthreads, relR, relS,param, timer, THREAD_TASK_SHUFFLE_HS)
    param = finishing(nthreads, param);
#ifndef NO_TIMING
    /* now print the timing results: */
    print_timing(relS->num_tuples, param.result, &timer);
#endif
    return param.joinresult;
}

result_t *PMJ_st(relation_t *relR, relation_t *relS, int nthreads) {

    t_param tParam(1);

#ifdef JOIN_RESULT_MATERIALIZE
    joinresult->resultlist = (threadresult_t *) malloc(sizeof(threadresult_t));
#endif

#ifndef NO_TIMING
    T_TIMER timer;
    START_MEASURE(timer)
#endif

#ifdef JOIN_RESULT_MATERIALIZE
    chainedtuplebuffer_t * chainedbuf = chainedtuplebuffer_init();
#else
    void *chainedbuf = NULL;
#endif

    // No distribution nor partition
    // Directly call the local pmj joiner.
    tParam.result = pmj(0, relR, relS, chainedbuf, &timer);//build and probe at the same time.

#ifdef JOIN_RESULT_MATERIALIZE
    threadresult_t * thrres = &(joinresult->resultlist[0]);/* single-thread */
    thrres->nresults = result;
    thrres->threadid = 0;
    thrres->results  = (void *) chainedbuf;
#endif

#ifndef NO_TIMING
    END_MEASURE(timer)
    /* now print the timing results: */
    print_timing(relS->num_tuples, tParam.result, &timer);
#endif

    tParam.joinresult->totalresults = tParam.result;
    tParam.joinresult->nthreads = 1;

    return tParam.joinresult;
}


result_t *PMJ_JM_NP(relation_t *relR, relation_t *relS, int nthreads) {

    t_param param(nthreads);

#ifdef JOIN_RESULT_MATERIALIZE
    joinresult->resultlist = (threadresult_t *) malloc(sizeof(threadresult_t));
#endif

#ifndef NO_TIMING
    T_TIMER timer;
    START_MEASURE(timer)
#endif

#ifdef JOIN_RESULT_MATERIALIZE
    chainedtuplebuffer_t * chainedbuf = chainedtuplebuffer_init();
#else
    void *chainedbuf = NULL;
#endif

    initialize(nthreads, param);
    param.fetcher = type_JM_NP_Fetcher;//new JM_NP_Fetcher(nthreads, relR, relS);
    //no shuffler is required for JM mode.
    param.joiner = type_PMJJoiner;//new PMJJoiner(relR->num_tuples, relS->num_tuples / nthreads, nthreads);
    LAUNCH(nthreads, relR, relS,param, timer, THREAD_TASK_NOSHUFFLE)
    param = finishing(nthreads, param);

#ifdef JOIN_RESULT_MATERIALIZE
    threadresult_t * thrres = &(joinresult->resultlist[0]);/* single-thread */
    thrres->nresults = result;
    thrres->threadid = 0;
    thrres->results  = (void *) chainedbuf;
#endif

#ifndef NO_TIMING
    /* now print the timing results: */
    print_timing(relS->num_tuples, param.result, &timer);
#endif
    return param.joinresult;
}

result_t *PMJ_JB_NP(relation_t *relR, relation_t *relS, int nthreads) {

    t_param param(nthreads);

#ifdef JOIN_RESULT_MATERIALIZE
    joinresult->resultlist = (threadresult_t *) malloc(sizeof(threadresult_t));
#endif

#ifndef NO_TIMING
    T_TIMER timer;
    START_MEASURE(timer)
#endif

#ifdef JOIN_RESULT_MATERIALIZE
    chainedtuplebuffer_t * chainedbuf = chainedtuplebuffer_init();
#else
    void *chainedbuf = NULL;
#endif

    initialize(nthreads, param);
    param.fetcher = type_JB_NP_Fetcher;//new JM_NP_Fetcher(nthreads, relR, relS);
    param.shuffler = new HashShuffler(nthreads, relR, relS);
    param.joiner = type_PMJJoiner;//new PMJJoiner(relR->num_tuples, relS->num_tuples / nthreads, nthreads);
    LAUNCH(nthreads, relR, relS,param, timer, THREAD_TASK_SHUFFLE)
    param = finishing(nthreads, param);

#ifdef JOIN_RESULT_MATERIALIZE
    threadresult_t * thrres = &(joinresult->resultlist[0]);/* single-thread */
    thrres->nresults = result;
    thrres->threadid = 0;
    thrres->results  = (void *) chainedbuf;
#endif

#ifndef NO_TIMING
    /* now print the timing results: */
    print_timing(relS->num_tuples, param.result, &timer);
#endif
    return param.joinresult;
}

result_t *PMJ_HS_NP(relation_t *relR, relation_t *relS, int nthreads) {

    t_param param(nthreads);

#ifdef JOIN_RESULT_MATERIALIZE
    joinresult->resultlist = (threadresult_t *) malloc(sizeof(threadresult_t));
#endif

#ifndef NO_TIMING
    T_TIMER timer;
    START_MEASURE(timer)
#endif

#ifdef JOIN_RESULT_MATERIALIZE
    chainedtuplebuffer_t * chainedbuf = chainedtuplebuffer_init();
#else
    void *chainedbuf = NULL;
#endif

    initialize(nthreads, param);
    param.fetcher = type_HS_NP_Fetcher;//new JM_NP_Fetcher(nthreads, relR, relS);
    param.shuffler = new HSShuffler(nthreads, relR, relS);
    param.joiner = type_PMJJoiner;//new PMJJoiner(relR->num_tuples, relS->num_tuples / nthreads, nthreads);
    LAUNCH(nthreads, relR, relS,param, timer, THREAD_TASK_SHUFFLE_HS)
    param = finishing(nthreads, param);

#ifdef JOIN_RESULT_MATERIALIZE
    threadresult_t * thrres = &(joinresult->resultlist[0]);/* single-thread */
    thrres->nresults = result;
    thrres->threadid = 0;
    thrres->results  = (void *) chainedbuf;
#endif

#ifndef NO_TIMING
    /* now print the timing results: */
    print_timing(relS->num_tuples, param.result, &timer);
#endif
    return param.joinresult;
}

result_t *RPJ_st(relation_t *relR, relation_t *relS, int nthreads) {

    t_param tParam(1);

#ifdef JOIN_RESULT_MATERIALIZE
    joinresult->resultlist = (threadresult_t *) malloc(sizeof(threadresult_t));
#endif

#ifndef NO_TIMING
    T_TIMER timer;
    START_MEASURE(timer)
#endif

#ifdef JOIN_RESULT_MATERIALIZE
    chainedtuplebuffer_t * chainedbuf = chainedtuplebuffer_init();
#else
    void *chainedbuf = NULL;
#endif

    // No distribution nor partition
    // Directly call the local joiner.
    tParam.result = rpj(0, relR, relS, chainedbuf, &timer);// nested loop version.
//    tParam.result = hrpj(0, relR, relS, chainedbuf, &timer);// hash version.

#ifdef JOIN_RESULT_MATERIALIZE
    threadresult_t * thrres = &(joinresult->resultlist[0]);/* single-thread */
    thrres->nresults = result;
    thrres->threadid = 0;
    thrres->results  = (void *) chainedbuf;
#endif

#ifndef NO_TIMING
    END_MEASURE(timer)
    /* now print the timing results: */
    print_timing(relS->num_tuples, tParam.result, &timer);
#endif

    tParam.joinresult->totalresults = tParam.result;
    tParam.joinresult->nthreads = 1;

    return tParam.joinresult;
}

result_t *
RPJ_JM_NP(relation_t *relR, relation_t *relS, int nthreads) {
    t_param param(nthreads);
#ifndef NO_TIMING
    T_TIMER timer;
#endif

#ifdef JOIN_RESULT_MATERIALIZE
    joinresult->resultlist = (threadresult_t *) malloc(sizeof(threadresult_t)
                                                       * nthreads);
#endif
    initialize(nthreads, param);
    param.fetcher = type_JM_NP_Fetcher;//new JM_NP_Fetcher(nthreads, relR, relS);
    //no shuffler is required for JM mode.
    param.joiner = type_RippleJoiner;//new RippleJoiner(relR, relS, nthreads);
    LAUNCH(nthreads, relR, relS,param, timer, THREAD_TASK_NOSHUFFLE)
    param = finishing(nthreads, param);
#ifndef NO_TIMING
    /* now print the timing results: */
    print_timing(relS->num_tuples, param.result, &timer);
#endif
    return param.joinresult;
}

result_t *
RPJ_JB_NP(relation_t *relR, relation_t *relS, int nthreads) {
    t_param param(nthreads);
#ifndef NO_TIMING
    T_TIMER timer;
#endif
#ifdef JOIN_RESULT_MATERIALIZE
    joinresult->resultlist = (threadresult_t *) malloc(sizeof(threadresult_t)
                                                       * nthreads);
#endif
    initialize(nthreads, param);
    param.fetcher = type_JB_NP_Fetcher;//new JB_NP_Fetcher(nthreads, relR, relS);
    param.shuffler = new HashShuffler(nthreads, relR, relS);
    param.joiner = type_RippleJoiner;// new RippleJoiner(relR, relS, nthreads);
    LAUNCH(nthreads,relR, relS, param, timer, THREAD_TASK_SHUFFLE)
    param = finishing(nthreads, param);
#ifndef NO_TIMING
    /* now print the timing results: */
    print_timing(relS->num_tuples, param.result, &timer);
#endif
    return param.joinresult;
}


result_t *RPJ_HS_NP(relation_t *relR, relation_t *relS, int nthreads) {
    t_param param(nthreads);
#ifndef NO_TIMING
    T_TIMER timer;
#endif
#ifdef JOIN_RESULT_MATERIALIZE
    joinresult->resultlist = (threadresult_t *) malloc(sizeof(threadresult_t)
                                                       * nthreads);
#endif
    initialize(nthreads, param);
    param.fetcher = type_HS_NP_Fetcher;//new HS_NP_Fetcher(nthreads, relR, relS);
    param.shuffler = new HSShuffler(nthreads, relR, relS);
    param.joiner = type_RippleJoiner;//new RippleJoiner(relR, relS, nthreads);
    LAUNCH(nthreads, relR, relS,param, timer, THREAD_TASK_SHUFFLE_HS)
    param = finishing(nthreads, param);
#ifndef NO_TIMING
    /* now print the timing results: */
    print_timing(relS->num_tuples, param.result, &timer);
#endif
    return param.joinresult;
}

/** @}*/
