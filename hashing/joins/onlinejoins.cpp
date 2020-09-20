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
#include "../utils/barrier.h"
#include "../helper/launcher.h"
#include "../helper/thread_task.h"
#include "common_functions.h"

/** @} */

void initialize(int nthreads, const t_param &param) {
    int rv;
    rv = pthread_barrier_init(param.barrier, NULL, nthreads);
    if (rv != 0) {
        MSG("Couldn't create the barrier\n");
        exit(EXIT_FAILURE);
    }
    pthread_attr_init(param.attr);
}

t_param &finishing(int nthreads, t_param &param, uint64_t *startTS, param_t *cmd_params) {
    int i;
    for (i = 0; i < nthreads; i++) {
        if (param.tid[i] != -1)
            pthread_join(param.tid[i], NULL);
    }

    // TODO: add a timer here, how to minus startTimer? Can I use t_timer.h
    int64_t processingTime = curtick() - *startTS;
#ifndef NO_TIMING
    MSG("With timing, Total processing time is: %f", processingTime / (2.1 * 1E6));//cycle to ms
#endif
#ifdef NO_TIMING
    MSG("No timing, Total processing time is: %ld\n", processingTime / (2.1 * 1E6));
#endif
    for (i = 0; i < nthreads; i++) {
        /* sum up results */
        param.result += *param.args[i].matches;
#ifndef NO_TIMING
        merge(param.args[i].timer, param.args[i].fetcher->relR, param.args[i].fetcher->relS, startTS, 0);
#endif
    }
    param.joinresult->totalresults = param.result;
    param.joinresult->nthreads = nthreads;
#ifndef NO_TIMING
#ifndef JOIN //partition-only.
    std::string name = param.algo_name + "_" + std::to_string(param.exp_id);
    string path = "/data1/xtra/results/breakdown/partition_only/" + name.append(".txt");
    auto fp = fopen(path.c_str(), "w");
    /* now print the timing results: */
    for (i = 0; i < nthreads; i++) {
        dump_partition_cost(param.args[i].timer, fp);//partition_only
    }
#else
#ifndef MERGE //build/sort only
    std::string name = param.algo_name + "_" + std::to_string(param.exp_id);
    string path = "/data1/xtra/results/breakdown/partition_buildsort_only/" + name.append(".txt");
    auto fp = fopen(path.c_str(), "w");
    /* now print the timing results: */
    for (i = 0; i < nthreads; i++) {
        dump_partition_cost(param.args[i].timer, fp);//partition + sort/build
    }
#else
#ifndef MATCH //build/sort + probe/merge only
    std::string name = param.algo_name + "_" + std::to_string(param.exp_id);
    string path = "/data1/xtra/results/breakdown/partition_buildsort_probemerge_only/" + name.append(".txt");
    auto fp = fopen(path.c_str(), "w");
    /* now print the timing results: */
    for (i = 0; i < nthreads; i++) {
        dump_partition_cost(param.args[i].timer, fp);//partition + sort/build + probe/merge
    }
#else
#ifndef WAIT //everything except wait.
    std::string name = param.algo_name + "_" + std::to_string(param.exp_id);
    string path = "/data1/xtra/results/breakdown/partition_buildsort_probemerge_join/" + name.append(".txt");
    auto fp = fopen(path.c_str(), "w");
    /* now print the timing results: */
    for (i = 0; i < nthreads; i++) {
        dump_partition_cost(param.args[i].timer, fp);//partition + sort/build + probe/merge + match (no wait)
    }
#else //everything is defined.
    std::string name = param.algo_name + "_" + std::to_string(param.exp_id).append(".txt");
    /* now print the timing results: */
    string path = "/data1/xtra/results/breakdown/allIncludes/" + name;
    auto fp = fopen(path.c_str(), "w");
    double average_partition_timer = 0.0;
    for (i = 0; i < nthreads; i++) {
        dump_partition_cost(param.args[i].timer, fp);//partition + sort/build
        average_partition_timer += param.args[i].timer->partition_timer;
    }
    average_partition_timer /= nthreads;
    breakdown_global(
            (param.args[0].fetcher->relR->num_tuples + param.args[0].fetcher->relS->num_tuples), nthreads,
            average_partition_timer,
            name, cmd_params->window_size);
    fclose(fp);
    sortRecords(param.algo_name, param.exp_id, 0,
                (param.args[0].fetcher->relR->num_tuples + param.args[0].fetcher->relS->num_tuples),
                param.joinresult->totalresults);
#endif // except wait.
#endif // partition with sort/build, probe/merge
#endif // partition with sort/build only
#endif // partition-only
#endif // no_timing flag
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
SHJ_st(relation_t *relR, relation_t *relS, param_t cmd_params) {

    t_param param(1);

#ifdef JOIN_RESULT_MATERIALIZE
    auto resultlist = (threadresult_t *) malloc(sizeof(threadresult_t));
#endif

#ifdef JOIN_RESULT_MATERIALIZE
    chainedtuplebuffer_t *chainedbuf = chainedtuplebuffer_init();
#else
    void *chainedbuf = NULL;
#endif

    // No distribution nor partition
    // Directly call the local SHJ joiner.
    SHJJoiner joiner = shj(0, relR, relS, chainedbuf);//build and probe at the same time.

#ifdef JOIN_RESULT_MATERIALIZE
    threadresult_t *thrres = &(resultlist[0]);/* single-thread */
    thrres->nresults = joiner.matches;
    thrres->threadid = 0;
    thrres->results = (void *) chainedbuf;
#endif
    param.args[0].timer = joiner.timer;
    param.args[0].matches = &joiner.matches;
    finishing(1, param, NULL, NULL);
    param.joinresult->totalresults = param.result;
    param.joinresult->nthreads = 1;
    return param.joinresult;
}

//1st online algorithm
result_t *
SHJ_JM_NP(relation_t *relR, relation_t *relS, param_t cmd_params) {
    t_param param(nthreads);
    initialize(nthreads, param);
    param.fetcher = type_JM_NP_Fetcher;//new JM_NP_Fetcher(nthreads, relR, relS);
    //no shuffler is required for JM mode.
    param.joiner = type_SHJJoiner;//new SHJJoiner();
    param.algo_name = "SHJ_JM_NP";
    param.exp_id = cmd_params.exp_id;
    param.record_gap = cmd_params.gap;
    auto *startTS = new uint64_t();
    auto joinStart = (uint64_t) 0;
    LAUNCH(nthreads, relR, relS, param, THREAD_TASK_NOSHUFFLE, startTS, &joinStart)
    param = finishing(nthreads, param, startTS, &cmd_params);
    return param.joinresult;
}

//1.5st online algorithm
result_t *
SHJ_JM_P(relation_t *relR, relation_t *relS, param_t cmd_params) {
    t_param param(nthreads);
    initialize(nthreads, param);
    param.fetcher = type_JM_P_Fetcher;//new JM_NP_Fetcher(nthreads, relR, relS);
    //no shuffler is required for JM mode.
    param.joiner = type_SHJJoiner;//new SHJJoiner();
    param.algo_name = "SHJ_JM_P";
    param.exp_id = cmd_params.exp_id;
    param.record_gap = cmd_params.gap;
    auto *startTS = new uint64_t();
    auto joinStart = (uint64_t) 0;
    LAUNCH(nthreads, relR, relS, param, THREAD_TASK_NOSHUFFLE, startTS, &joinStart)
    param = finishing(nthreads, param, startTS, &cmd_params);
    return param.joinresult;
}

//2nd online algorithm
result_t *
SHJ_JB_NP(relation_t *relR, relation_t *relS, param_t cmd_params) {
    t_param param(nthreads);
    initialize(nthreads, param);
    param.fetcher = type_JB_NP_Fetcher;//new JB_NP_Fetcher(nthreads, relR, relS);
    param.shuffler = new HashShuffler(nthreads, relR, relS);
    param.joiner = type_SHJJoiner;//new SHJJoiner();
    param.algo_name = "SHJ_JB_NP";
    param.exp_id = cmd_params.exp_id;
    param.record_gap = cmd_params.gap;
    uint64_t *startTS = new uint64_t();
    auto joinStart = (uint64_t) 0;
    LAUNCH(nthreads, relR, relS, param, THREAD_TASK_SHUFFLE, startTS, &joinStart)
    param = finishing(nthreads, param, startTS, &cmd_params);
    return param.joinresult;
}

//3rd online algorithm
result_t *
SHJ_JBCR_NP(relation_t *relR, relation_t *relS, param_t cmd_params) {
    t_param param(nthreads);
    initialize(nthreads, param);
    param.fetcher = type_JB_NP_Fetcher;//new JB_NP_Fetcher(nthreads, relR, relS);
    param.shuffler = new ContRandShuffler(nthreads, relR, relS, cmd_params.group_size);
    param.joiner = type_SHJJoiner;//new SHJJoiner();
    param.algo_name = "SHJ_JBCR_NP";
    param.exp_id = cmd_params.exp_id;
    param.record_gap = cmd_params.gap;
    uint64_t *startTS = new uint64_t();
    auto joinStart = (uint64_t) 0;
    LAUNCH(nthreads, relR, relS, param, THREAD_TASK_SHUFFLE, startTS, &joinStart)
    param = finishing(nthreads, param, startTS, &cmd_params);
    return param.joinresult;
}

//3.5rd online algorithm
result_t *
SHJ_JBCR_P(relation_t *relR, relation_t *relS, param_t cmd_params) {
    t_param param(nthreads);
    initialize(nthreads, param);
    param.fetcher = type_JB_P_Fetcher;//new JB_NP_Fetcher(nthreads, relR, relS);
    param.shuffler = new ContRandShuffler(nthreads, relR, relS, cmd_params.group_size);
    param.joiner = type_SHJJoiner;//new SHJJoiner();
    param.algo_name = "SHJ_JBCR_P";
    param.exp_id = cmd_params.exp_id;
    param.record_gap = cmd_params.gap;
    uint64_t *startTS = new uint64_t();
    auto joinStart = (uint64_t) 0;
    LAUNCH(nthreads, relR, relS, param, THREAD_TASK_SHUFFLE, startTS, &joinStart)
    param = finishing(nthreads, param, startTS, &cmd_params);
    return param.joinresult;
}

//4th
result_t *
SHJ_HS_NP(relation_t *relR, relation_t *relS, param_t cmd_params) {
    t_param param(nthreads);
    initialize(nthreads, param);
    param.fetcher = type_HS_NP_Fetcher;//new HS_NP_Fetcher(nthreads, relR, relS);
    param.shuffler = new HSShuffler(nthreads, relR, relS);
    param.joiner = type_SHJJoiner;//new SHJJoiner();
    param.algo_name = "SHJ_HS_NP";
    param.exp_id = cmd_params.exp_id;
    param.record_gap = cmd_params.gap;
    uint64_t *startTS = new uint64_t();
    auto joinStart = (uint64_t) 0;
    LAUNCH(nthreads, relR, relS, param, THREAD_TASK_SHUFFLE_HS, startTS, &joinStart)
    param = finishing(nthreads, param, startTS, &cmd_params);
    return param.joinresult;
}

result_t *
PMJ_st(relation_t *relR, relation_t *relS, param_t cmd_params) {

    t_param param(1);

#ifdef JOIN_RESULT_MATERIALIZE
    auto resultlist = (threadresult_t *) malloc(sizeof(threadresult_t));
#endif


#ifdef JOIN_RESULT_MATERIALIZE
    chainedtuplebuffer_t *chainedbuf = chainedtuplebuffer_init();
#else
    void *chainedbuf = NULL;
#endif


    // No distribution nor partition
    // Directly call the local pmj joiner.
    PMJJoiner joiner = pmj(0, relR, relS, chainedbuf);//build and probe at the same time.

#ifdef JOIN_RESULT_MATERIALIZE
    threadresult_t *thrres = &(resultlist[0]);/* single-thread */
    thrres->nresults = joiner.matches;
    thrres->threadid = 0;
    thrres->results = (void *) chainedbuf;
#endif
    param.args[0].timer = joiner.timer;
    param.args[0].matches = &joiner.matches;
    finishing(1, param, nullptr, nullptr);
    param.joinresult->totalresults = param.result;
    param.joinresult->nthreads = 1;

    return param.joinresult;
}

result_t *
RPJ_st(relation_t *relR, relation_t *relS, param_t cmd_params) {

    t_param param(1);
#ifdef JOIN_RESULT_MATERIALIZE
    auto resultlist = (threadresult_t *) malloc(sizeof(threadresult_t));
#endif


#ifdef JOIN_RESULT_MATERIALIZE
    chainedtuplebuffer_t *chainedbuf = chainedtuplebuffer_init();
#else
    void *chainedbuf = NULL;
#endif
    // No distribution nor partition
    // Directly call the local joiner.
    RippleJoiner joiner = rpj(0, relR, relS, chainedbuf);// nested loop version.
//    tParam.result = hrpj(0, relR, relS, chainedbuf, &timer);// hash version.

#ifdef JOIN_RESULT_MATERIALIZE
    threadresult_t *thrres = &(resultlist[0]);/* single-thread */
    thrres->nresults = joiner.matches;
    thrres->threadid = 0;
    thrres->results = (void *) chainedbuf;
#endif
    param.args[0].timer = joiner.timer;
    param.args[0].matches = &joiner.matches;
    finishing(1, param, nullptr, nullptr);
    param.joinresult->totalresults = param.result;
    param.joinresult->nthreads = 1;

    return param.joinresult;
}

//5th
result_t *PMJ_JM_NP(relation_t *relR, relation_t *relS, param_t cmd_params) {
    t_param param(nthreads);
    initialize(nthreads, param);
    param.fetcher = type_JM_NP_Fetcher;//new JM_NP_Fetcher(nthreads, relR, relS);
    //no shuffler is required for JM mode.
    param.joiner = type_PMJJoiner;//new PMJJoiner(relR->num_tuples, relS->num_tuples / nthreads, nthreads);
    param.algo_name = "PMJ_JM_NP";
    param.exp_id = cmd_params.exp_id;
    param.record_gap = cmd_params.gap;

    param.progressive_step = cmd_params.progressive_step;
    param.merge_step = cmd_params.merge_step;
    uint64_t *startTS = new uint64_t();
    auto joinStart = (uint64_t) 0;
    LAUNCH(nthreads, relR, relS, param, THREAD_TASK_NOSHUFFLE, startTS, &joinStart)
    param = finishing(nthreads, param, startTS, &cmd_params);
    return param.joinresult;
}

//5.5th
result_t *PMJ_JM_P(relation_t *relR, relation_t *relS, param_t cmd_params) {
    t_param param(nthreads);
    initialize(nthreads, param);
    param.fetcher = type_JM_P_Fetcher;//new JM_NP_Fetcher(nthreads, relR, relS);
    //no shuffler is required for JM mode.
    param.joiner = type_PMJJoiner;//new PMJJoiner(relR->num_tuples, relS->num_tuples / nthreads, nthreads);
    param.algo_name = "PMJ_JM_P";
    param.exp_id = cmd_params.exp_id;
    param.record_gap = cmd_params.gap;

    param.progressive_step = cmd_params.progressive_step;
    param.merge_step = cmd_params.merge_step;
    uint64_t *startTS = new uint64_t();
    auto joinStart = (uint64_t) 0;
    LAUNCH(nthreads, relR, relS, param, THREAD_TASK_NOSHUFFLE, startTS, &joinStart)
    param = finishing(nthreads, param, startTS, &cmd_params);
    return param.joinresult;
}

//6th
result_t *PMJ_JB_NP(relation_t *relR, relation_t *relS, param_t cmd_params) {
    t_param param(nthreads);
    initialize(nthreads, param);
    param.fetcher = type_JB_NP_Fetcher;//new JM_NP_Fetcher(nthreads, relR, relS);
    param.shuffler = new HashShuffler(nthreads, relR, relS);
    param.joiner = type_PMJJoiner;//new PMJJoiner(relR->num_tuples, relS->num_tuples / nthreads, nthreads);
    param.algo_name = "PMJ_JB_NP";
    param.exp_id = cmd_params.exp_id;
    param.record_gap = cmd_params.gap;

    param.progressive_step = cmd_params.progressive_step;
    param.merge_step = cmd_params.merge_step;

    uint64_t *startTS = new uint64_t();
    auto joinStart = (uint64_t) 0;
    LAUNCH(nthreads, relR, relS, param, THREAD_TASK_SHUFFLE, startTS, &joinStart)
    param = finishing(nthreads, param, startTS, &cmd_params);
    return param.joinresult;
}

//6.5
result_t *PMJ_JB_P(relation_t *relR, relation_t *relS, param_t cmd_params) {
    t_param param(nthreads);
    initialize(nthreads, param);
    param.fetcher = type_JB_P_Fetcher;//new JM_NP_Fetcher(nthreads, relR, relS);
    param.shuffler = new HashShuffler(nthreads, relR, relS);
    param.joiner = type_PMJJoiner;//new PMJJoiner(relR->num_tuples, relS->num_tuples / nthreads, nthreads);
    param.algo_name = "PMJ_JB_P";
    param.exp_id = cmd_params.exp_id;
    param.record_gap = cmd_params.gap;

    param.progressive_step = cmd_params.progressive_step;
    param.merge_step = cmd_params.merge_step;

    uint64_t *startTS = new uint64_t();
    auto joinStart = (uint64_t) 0;
    LAUNCH(nthreads, relR, relS, param, THREAD_TASK_SHUFFLE, startTS, &joinStart)
    param = finishing(nthreads, param, startTS, &cmd_params);
    return param.joinresult;
}

//7th
result_t *PMJ_JBCR_NP(relation_t *relR, relation_t *relS, param_t cmd_params) {
    t_param param(nthreads);
    initialize(nthreads, param);
    param.fetcher = type_JB_NP_Fetcher;//new JM_NP_Fetcher(nthreads, relR, relS);
    param.shuffler = new ContRandShuffler(nthreads, relR, relS, cmd_params.group_size);
    param.joiner = type_PMJJoiner;//new PMJJoiner(relR->num_tuples, relS->num_tuples / nthreads, nthreads);
    param.algo_name = "PMJ_JBCR_NP";
    param.exp_id = cmd_params.exp_id;
    param.record_gap = cmd_params.gap;
    param.progressive_step = cmd_params.progressive_step;
    param.merge_step = cmd_params.merge_step;
    uint64_t *startTS = new uint64_t();
    auto joinStart = (uint64_t) 0;
    LAUNCH(nthreads, relR, relS, param, THREAD_TASK_SHUFFLE, startTS, &joinStart)
    param = finishing(nthreads, param, startTS, &cmd_params);
    return param.joinresult;
}

//7.5th
result_t *PMJ_JBCR_P(relation_t *relR, relation_t *relS, param_t cmd_params) {
    t_param param(nthreads);
    initialize(nthreads, param);
    param.fetcher = type_JB_P_Fetcher;//new JM_NP_Fetcher(nthreads, relR, relS);
    param.shuffler = new ContRandShuffler(nthreads, relR, relS, cmd_params.group_size);
    param.joiner = type_PMJJoiner;//new PMJJoiner(relR->num_tuples, relS->num_tuples / nthreads, nthreads);
    param.algo_name = "PMJ_JBCR_P";
    param.exp_id = cmd_params.exp_id;
    param.record_gap = cmd_params.gap;
    param.progressive_step = cmd_params.progressive_step;
    param.merge_step = cmd_params.merge_step;
    uint64_t *startTS = new uint64_t();
    auto joinStart = (uint64_t) 0;
    LAUNCH(nthreads, relR, relS, param, THREAD_TASK_SHUFFLE, startTS, &joinStart)
    param = finishing(nthreads, param, startTS, &cmd_params);
    return param.joinresult;
}


//8th
result_t *PMJ_HS_NP(relation_t *relR, relation_t *relS, param_t cmd_params) {
    t_param param(nthreads);
    initialize(nthreads, param);
    param.fetcher = type_PMJ_HS_NP_Fetcher;//new JM_NP_Fetcher(nthreads, relR, relS);
    param.shuffler = new HSShuffler(nthreads, relR, relS);
    param.joiner = type_PMJJoiner;//new PMJJoiner(relR->num_tuples, relS->num_tuples / nthreads, nthreads);
    param.algo_name = "PMJ_HS_NP";
    param.exp_id = cmd_params.exp_id;
    param.record_gap = cmd_params.gap;
    uint64_t *startTS = new uint64_t();
    auto joinStart = (uint64_t) 0;
    LAUNCH(nthreads, relR, relS, param, THREAD_TASK_SHUFFLE_PMJHS, startTS, &joinStart)
    param = finishing(nthreads, param, startTS, &cmd_params);
    return param.joinresult;
}


//9th
result_t *
RPJ_JM_NP(relation_t *relR, relation_t *relS, param_t cmd_params) {
    t_param param(nthreads);
    initialize(nthreads, param);
    param.fetcher = type_JM_NP_Fetcher;//new JM_NP_Fetcher(nthreads, relR, relS);
    //no shuffler is required for JM mode.
    param.joiner = type_RippleJoiner;//new RippleJoiner(relR, relS, nthreads);
    param.algo_name = "RPJ_JM_NP";
    param.exp_id = cmd_params.exp_id;
    param.record_gap = cmd_params.gap;
    uint64_t *startTS = new uint64_t();
    auto joinStart = (uint64_t) 0;
    LAUNCH(nthreads, relR, relS, param, THREAD_TASK_NOSHUFFLE, startTS, &joinStart)
    param = finishing(nthreads, param, startTS, &cmd_params);
    return param.joinresult;
}

//10th
result_t *
RPJ_JB_NP(relation_t *relR, relation_t *relS, param_t cmd_params) {
    t_param param(nthreads);
    initialize(nthreads, param);
    param.fetcher = type_JB_NP_Fetcher;//new JB_NP_Fetcher(nthreads, relR, relS);
    param.shuffler = new HashShuffler(nthreads, relR, relS);
    param.joiner = type_RippleJoiner;// new RippleJoiner(relR, relS, nthreads);
    param.algo_name = "RPJ_JB_NP";
    param.exp_id = cmd_params.exp_id;
    param.record_gap = cmd_params.gap;
    uint64_t *startTS = new uint64_t();
    auto joinStart = (uint64_t) 0;
    LAUNCH(nthreads, relR, relS, param, THREAD_TASK_SHUFFLE, startTS, &joinStart)
    param = finishing(nthreads, param, startTS, &cmd_params);
    return param.joinresult;
}

//11th
result_t *
RPJ_JBCR_NP(relation_t *relR, relation_t *relS, param_t cmd_params) {
    t_param param(nthreads);
    initialize(nthreads, param);
    param.fetcher = type_JB_NP_Fetcher;//new JB_NP_Fetcher(nthreads, relR, relS);
    param.shuffler = new ContRandShuffler(nthreads, relR, relS, 0);
    param.joiner = type_RippleJoiner;// new RippleJoiner(relR, relS, nthreads);
    param.algo_name = "RPJ_JBCR_NP";
    param.exp_id = cmd_params.exp_id;
    param.record_gap = cmd_params.gap;
    uint64_t *startTS = new uint64_t();
    auto joinStart = (uint64_t) 0;
    LAUNCH(nthreads, relR, relS, param, THREAD_TASK_SHUFFLE, startTS, &joinStart)
    param = finishing(nthreads, param, startTS, &cmd_params);
    return param.joinresult;
}

//12th
result_t *RPJ_HS_NP(relation_t *relR, relation_t *relS, param_t cmd_params) {
    t_param param(nthreads);
    initialize(nthreads, param);
    param.fetcher = type_HS_NP_Fetcher;//new HS_NP_Fetcher(nthreads, relR, relS);
    param.shuffler = new HSShuffler(nthreads, relR, relS);
    param.joiner = type_RippleJoiner;//new RippleJoiner(relR, relS, nthreads);
    param.algo_name = "RPJ_HS_NP";
    param.exp_id = cmd_params.exp_id;
    param.record_gap = cmd_params.gap;
    uint64_t *startTS = new uint64_t();
    auto joinStart = (uint64_t) 0;
    LAUNCH(nthreads, relR, relS, param, THREAD_TASK_SHUFFLE_HS, startTS, &joinStart)
    param = finishing(nthreads, param, startTS, &cmd_params);
    return param.joinresult;
}

/** @}*/
