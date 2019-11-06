//
// Created by Shuhao Zhang on 1/11/19.
//

#include "launcher.h"


void launch(int nthreads, t_param param, T_TIMER *timer, void *(*thread_fun)(void *)) {

    int i;
    int rv;
    cpu_set_t set;

    for (i = 0; i < nthreads; i++) {
        int cpu_idx = get_cpu_id(i);
        DEBUGMSG(1, "Assigning thread-%d to CPU-%d\n", i, cpu_idx);
        CPU_ZERO(&set);
        CPU_SET(cpu_idx, &set);
        pthread_attr_setaffinity_np(param.attr, sizeof(cpu_set_t), &set);
        param.args[i].tid = i;
        param.args[i].timer = timer;
        param.args[i].barrier = param.barrier;
        param.args[i].fetcher = param.fetcher;
        param.args[i].shuffler = param.shuffler;
        param.args[i].threadresult = &(param.joinresult->resultlist[i]);
        rv = pthread_create(&param.tid[i], param.attr, thread_fun, (void *) &param.args[i]);
        if (rv) {
            printf("ERROR; return code from pthread_create() is %d\n", rv);
            exit(-1);
        }
    }
}

/**
 * Multi-thread data partition, Join-Matrix Model, no-(physical)-partition method.
 * When we know size of input relation,
 * the optimal way is
 * (1) partition long relation
 * and (2) replicate short relation.
 */
//void
//launch_jm_np(const relation_t *relR,
//             const relation_t *relS, int nthreads, t_param param, T_TIMER *timer) {
//
//    launch(relR, relS, nthreads, param, timer, THREAD_TASK_NOSHUFFLE);
//}

/**
 * Multi-thread data partition, Join-Biclique Model, no-(physical)-partition method.
 * Let's assume extreme hashing.
 */
//void
//launch_jb_np(const relation_t *relR, const relation_t *relS, int nthreads, t_param param, T_TIMER *timer) {
//    launch(relR, relS, nthreads, param, timer, shj_thread_jb_np);
//    for (i = 0; i < nthreads; i++) {
//        args[i].list = new list();
//        args[i].list->relR_list = new std::list<int>();
//        args[i].list->relS_list = new std::list<int>();
//    }
//    int32_t numR, numS; /* total num */
//    numR = relR->num_tuples;
//    numS = relS->num_tuples;
//
//    int j;
//    /* assign task for next thread by hashing */
//    for (j = 0; j < numR; j++) {
//        intkey_t idx = relR->tuples[j].key % nthreads;
//        args[idx].list->relR_list->push_back(j);
//    }
//
//    for (j = 0; j < numS; j++) {
//        intkey_t idy = relS->tuples[j].key % nthreads;
//        args[idy].list->relS_list->push_back(j);
//       printf("args[%d].relR_list : %zu\n", idy, args[idy].list->relS_list->size());
//    }
//
//    for (i = 0; i < nthreads; i++) {
//        int cpu_idx = get_cpu_id(i);
//        DEBUGMSG(1, "Assigning thread-%d to CPU-%d\n", i, cpu_idx);
//        CPU_ZERO(&set);
//        CPU_SET(cpu_idx, &set);
//        pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &set);
//        args[i].tid = i;
//        args[i].timer = timer;
//        args[i].relR.num_tuples = args[i].list->relR_list->size();
//        args[i].relR.tuples = relR->tuples;//configure pointer of start point.
//        args[i].relS.num_tuples = args[i].list->relS_list->size();
//        args[i].relS.tuples = relS->tuples;//configure pointer of start point.
//        args[i].threadresult = &(joinresult->resultlist[i]);
//        rv = pthread_create(&tid[i], &attr, shj_thread_jb_np, (void *) &args[i]);
//        if (rv) {
//            printf("ERROR; return code from pthread_create() is %d\n", rv);
//            exit(-1);
//        }
//    }
//}