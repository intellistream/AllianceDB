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

t_param &finishing(int nthreads, t_param &param, milliseconds *startTS) {

    int i;
    for (i = 0; i < nthreads; i++) {
        if (param.tid[i] != -1)
            pthread_join(param.tid[i], NULL);
        /* sum up results */
        param.result += *param.args[i].matches;
    }

    for (i = 0; i < nthreads; i++) {
#ifdef MEASURE
        merge(param.args[i].timer, param.args[i].fetcher->relR, param.args[i].fetcher->relS, startTS);
#endif
    }
    param.joinresult->totalresults = param.result;
    param.joinresult->nthreads = nthreads;
#ifdef MEASURE
    sortRecords(param.algo_name, param.exp_id, 0);
    std::string name = param.algo_name + "_" + std::to_string(param.exp_id);
    string path = "/data1/xtra/results/breakdown/" + name.append(".txt");
    auto fp = fopen(path.c_str(), "w");
    /* now print the timing results: */
    for (i = 0; i < nthreads; i++) {
        dump_breakdown(*param.args[i].matches, param.args[i].timer, 0, fp);
    }
    fclose(fp);
#endif
    return param;
}

/**
 * Single thread SHJ
 * @param relR
 * @param relS
 * @param nthreads
 * @return
 */
result_t *
SHJ_st(relation_t *relR, relation_t *relS, int nthreads, int exp_id, int group_size) {

    t_param param(1);

#ifdef JOIN_RESULT_MATERIALIZE
    joinresult->resultlist = (threadresult_t *) malloc(sizeof(threadresult_t));
#endif

#ifdef JOIN_RESULT_MATERIALIZE
    chainedtuplebuffer_t * chainedbuf = chainedtuplebuffer_init();
#else
    void *chainedbuf = NULL;
#endif

    // No distribution nor partition
    // Directly call the local SHJ joiner.
    SHJJoiner joiner = shj(0, relR, relS, chainedbuf);//build and probe at the same time.

#ifdef JOIN_RESULT_MATERIALIZE
    threadresult_t * thrres = &(joinresult->resultlist[0]);/* single-thread */
    thrres->nresults = result;
    thrres->threadid = 0;
    thrres->results  = (void *) chainedbuf;
#endif
    param.args[0].timer = joiner.timer;
    param.args[0].matches = &joiner.matches;
    finishing(1, param, nullptr);
    param.joinresult->totalresults = param.result;
    param.joinresult->nthreads = 1;
    return param.joinresult;
}

//1st online algorithm
result_t *
SHJ_JM_NP(relation_t *relR, relation_t *relS, int nthreads, int exp_id, int group_size) {
    t_param param(nthreads);
    initialize(nthreads, param);
    param.fetcher = type_JM_NP_Fetcher;//new JM_NP_Fetcher(nthreads, relR, relS);
    //no shuffler is required for JM mode.
    param.joiner = type_SHJJoiner;//new SHJJoiner();
    param.algo_name = "SHJ_JM_NP";
    param.exp_id = exp_id;
    auto startTS = now();
    LAUNCH(nthreads, relR, relS, param, THREAD_TASK_NOSHUFFLE, &startTS)
    param = finishing(nthreads, param, &startTS);
    return param.joinresult;
}

//2nd online algorithm
result_t *
SHJ_JB_NP(relation_t *relR, relation_t *relS, int nthreads, int exp_id, int group_size) {
    t_param param(nthreads);
    initialize(nthreads, param);
    param.fetcher = type_JB_NP_Fetcher;//new JB_NP_Fetcher(nthreads, relR, relS);
    param.shuffler = new HashShuffler(nthreads, relR, relS);
    param.joiner = type_SHJJoiner;//new SHJJoiner();
    param.algo_name = "SHJ_JB_NP";
    param.exp_id = exp_id;
    auto startTS = now();
    LAUNCH(nthreads, relR, relS, param, THREAD_TASK_SHUFFLE, &startTS)
    param = finishing(nthreads, param, &startTS);
    return param.joinresult;
}

//3rd online algorithm
result_t *
SHJ_JBCR_NP(relation_t *relR, relation_t *relS, int nthreads, int exp_id, int group_size) {
    t_param param(nthreads);
    initialize(nthreads, param);
    param.fetcher = type_JB_NP_Fetcher;//new JB_NP_Fetcher(nthreads, relR, relS);
    param.shuffler = new ContRandShuffler(nthreads, relR, relS);
    param.joiner = type_SHJJoiner;//new SHJJoiner();
    param.algo_name = "SHJ_JBCR_NP";
    param.exp_id = exp_id;
    auto startTS = now();
    LAUNCH(nthreads, relR, relS, param, THREAD_TASK_SHUFFLE, &startTS)
    param = finishing(nthreads, param, &startTS);
    return param.joinresult;
}

//4th
result_t *
SHJ_HS_NP(relation_t *relR, relation_t *relS, int nthreads, int exp_id, int group_size) {
    t_param param(nthreads);
    initialize(nthreads, param);
    param.fetcher = type_HS_NP_Fetcher;//new HS_NP_Fetcher(nthreads, relR, relS);
    param.shuffler = new HSShuffler(nthreads, relR, relS);
    param.joiner = type_SHJJoiner;//new SHJJoiner();
    param.algo_name = "SHJ_HS_NP";
    param.exp_id = exp_id;
    auto startTS = now();
    LAUNCH(nthreads, relR, relS, param, THREAD_TASK_SHUFFLE_HS, &startTS)
    param = finishing(nthreads, param, &startTS);
    return param.joinresult;
}

result_t *PMJ_st(relation_t *relR, relation_t *relS, int nthreads, int exp_id, int group_size) {

    t_param param(1);

#ifdef JOIN_RESULT_MATERIALIZE
    joinresult->resultlist = (threadresult_t *) malloc(sizeof(threadresult_t));
#endif


#ifdef JOIN_RESULT_MATERIALIZE
    chainedtuplebuffer_t * chainedbuf = chainedtuplebuffer_init();
#else
    void *chainedbuf = NULL;
#endif


    // No distribution nor partition
    // Directly call the local pmj joiner.
    PMJJoiner joiner = pmj(0, relR, relS, chainedbuf);//build and probe at the same time.

#ifdef JOIN_RESULT_MATERIALIZE
    threadresult_t * thrres = &(joinresult->resultlist[0]);/* single-thread */
    thrres->nresults = result;
    thrres->threadid = 0;
    thrres->results  = (void *) chainedbuf;
#endif
    param.args[0].timer = joiner.timer;
    param.args[0].matches = &joiner.matches;
    finishing(1, param, nullptr);
    param.joinresult->totalresults = param.result;
    param.joinresult->nthreads = 1;

    return param.joinresult;
}

result_t *RPJ_st(relation_t *relR, relation_t *relS, int nthreads, int exp_id, int group_size) {

    t_param param(1);
#ifdef JOIN_RESULT_MATERIALIZE
    joinresult->resultlist = (threadresult_t *) malloc(sizeof(threadresult_t));
#endif


#ifdef JOIN_RESULT_MATERIALIZE
    chainedtuplebuffer_t * chainedbuf = chainedtuplebuffer_init();
#else
    void *chainedbuf = NULL;
#endif
    // No distribution nor partition
    // Directly call the local joiner.
    RippleJoiner joiner = rpj(0, relR, relS, chainedbuf);// nested loop version.
//    tParam.result = hrpj(0, relR, relS, chainedbuf, &timer);// hash version.

#ifdef JOIN_RESULT_MATERIALIZE
    threadresult_t * thrres = &(joinresult->resultlist[0]);/* single-thread */
    thrres->nresults = result;
    thrres->threadid = 0;
    thrres->results  = (void *) chainedbuf;
#endif
    param.args[0].timer = joiner.timer;
    param.args[0].matches = &joiner.matches;
    finishing(1, param, nullptr);
    param.joinresult->totalresults = param.result;
    param.joinresult->nthreads = 1;

    return param.joinresult;
}

//5th
result_t *PMJ_JM_NP(relation_t *relR, relation_t *relS, int nthreads, int exp_id, int group_size) {
    t_param param(nthreads);
    initialize(nthreads, param);
    param.fetcher = type_JM_NP_Fetcher;//new JM_NP_Fetcher(nthreads, relR, relS);
    //no shuffler is required for JM mode.
    param.joiner = type_PMJJoiner;//new PMJJoiner(relR->num_tuples, relS->num_tuples / nthreads, nthreads);
    param.algo_name = "PMJ_JM_NP";
    param.exp_id = exp_id;
    auto startTS = now();
    LAUNCH(nthreads, relR, relS, param, THREAD_TASK_NOSHUFFLE, &startTS)
    param = finishing(nthreads, param, &startTS);
    return param.joinresult;
}

//6th
result_t *PMJ_JB_NP(relation_t *relR, relation_t *relS, int nthreads, int exp_id, int group_size) {
    t_param param(nthreads);
    initialize(nthreads, param);
    param.fetcher = type_JB_NP_Fetcher;//new JM_NP_Fetcher(nthreads, relR, relS);
    param.shuffler = new HashShuffler(nthreads, relR, relS);
    param.shuffler->group_size = group_size;
    param.joiner = type_PMJJoiner;//new PMJJoiner(relR->num_tuples, relS->num_tuples / nthreads, nthreads);
    param.algo_name = "PMJ_JB_NP";
    param.exp_id = exp_id;
    auto startTS = now();
    LAUNCH(nthreads, relR, relS, param, THREAD_TASK_SHUFFLE, &startTS)
    param = finishing(nthreads, param, &startTS);
    return param.joinresult;
}

//7th
result_t *PMJ_JBCR_NP(relation_t *relR, relation_t *relS, int nthreads, int exp_id, int group_size) {
    t_param param(nthreads);
    initialize(nthreads, param);
    param.fetcher = type_JB_NP_Fetcher;//new JM_NP_Fetcher(nthreads, relR, relS);
    param.shuffler = new ContRandShuffler(nthreads, relR, relS);
    param.shuffler->group_size = group_size;
    param.joiner = type_PMJJoiner;//new PMJJoiner(relR->num_tuples, relS->num_tuples / nthreads, nthreads);
    param.algo_name = "PMJ_JBCR_NP";
    param.exp_id = exp_id;
    auto startTS = now();
    LAUNCH(nthreads, relR, relS, param, THREAD_TASK_SHUFFLE, &startTS)
    param = finishing(nthreads, param, &startTS);
    return param.joinresult;
}

//8th
result_t *PMJ_HS_NP(relation_t *relR, relation_t *relS, int nthreads, int exp_id, int group_size) {
    t_param param(nthreads);
    initialize(nthreads, param);
    param.fetcher = type_PMJ_HS_NP_Fetcher;//new JM_NP_Fetcher(nthreads, relR, relS);
    param.shuffler = new HSShuffler(nthreads, relR, relS);
    param.joiner = type_PMJJoiner;//new PMJJoiner(relR->num_tuples, relS->num_tuples / nthreads, nthreads);
    param.algo_name = "PMJ_HS_NP";
    param.exp_id = exp_id;
    auto startTS = now();
    LAUNCH(nthreads, relR, relS, param, THREAD_TASK_SHUFFLE_PMJHS, &startTS)
    param = finishing(nthreads, param, &startTS);
    return param.joinresult;
}


//9th
result_t *
RPJ_JM_NP(relation_t *relR, relation_t *relS, int nthreads, int exp_id, int group_size) {
    t_param param(nthreads);
    initialize(nthreads, param);
    param.fetcher = type_JM_NP_Fetcher;//new JM_NP_Fetcher(nthreads, relR, relS);
    //no shuffler is required for JM mode.
    param.joiner = type_RippleJoiner;//new RippleJoiner(relR, relS, nthreads);
    param.algo_name = "RPJ_JM_NP";
    param.exp_id = exp_id;

    auto startTS = now();
    LAUNCH(nthreads, relR, relS, param, THREAD_TASK_NOSHUFFLE, &startTS)
    param = finishing(nthreads, param, &startTS);
    return param.joinresult;
}

//10th
result_t *
RPJ_JB_NP(relation_t *relR, relation_t *relS, int nthreads, int exp_id, int group_size) {
    t_param param(nthreads);
    initialize(nthreads, param);
    param.fetcher = type_JB_NP_Fetcher;//new JB_NP_Fetcher(nthreads, relR, relS);
    param.shuffler = new HashShuffler(nthreads, relR, relS);
    param.joiner = type_RippleJoiner;// new RippleJoiner(relR, relS, nthreads);
    param.algo_name = "RPJ_JB_NP";
    param.exp_id = exp_id;
    auto startTS = now();
    LAUNCH(nthreads, relR, relS, param, THREAD_TASK_SHUFFLE, &startTS)
    param = finishing(nthreads, param, &startTS);
    return param.joinresult;
}

//11th
result_t *
RPJ_JBCR_NP(relation_t *relR, relation_t *relS, int nthreads, int exp_id, int group_size) {
    t_param param(nthreads);
    initialize(nthreads, param);
    param.fetcher = type_JB_NP_Fetcher;//new JB_NP_Fetcher(nthreads, relR, relS);
    param.shuffler = new ContRandShuffler(nthreads, relR, relS);
    param.joiner = type_RippleJoiner;// new RippleJoiner(relR, relS, nthreads);
    param.algo_name = "RPJ_JBCR_NP";
    param.exp_id = exp_id;

    auto startTS = now();
    LAUNCH(nthreads, relR, relS, param, THREAD_TASK_SHUFFLE, &startTS)
    param = finishing(nthreads, param, &startTS);
    return param.joinresult;
}

//12th
result_t *RPJ_HS_NP(relation_t *relR, relation_t *relS, int nthreads, int exp_id, int group_size) {
    t_param param(nthreads);
    initialize(nthreads, param);
    param.fetcher = type_HS_NP_Fetcher;//new HS_NP_Fetcher(nthreads, relR, relS);
    param.shuffler = new HSShuffler(nthreads, relR, relS);
    param.joiner = type_RippleJoiner;//new RippleJoiner(relR, relS, nthreads);
    param.algo_name = "RPJ_HS_NP";
    param.exp_id = exp_id;
    auto startTS = now();
    LAUNCH(nthreads, relR, relS, param, THREAD_TASK_SHUFFLE_HS, &startTS)
    param = finishing(nthreads, param, &startTS);
    return param.joinresult;
}

/** @}*/
