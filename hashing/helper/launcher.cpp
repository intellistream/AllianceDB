//
// Created by Shuhao Zhang on 1/11/19.
//

#include "launcher.h"


void
launch(int nthreads, relation_t *relR, relation_t *relS, t_param param, void *(*thread_fun)(void *),
       milliseconds *startTS) {
    int i;
    int rv;
    cpu_set_t set;

    for (i = 0; i < nthreads; i++) {
        int cpu_idx = get_cpu_id(i);
        DEBUGMSG("Assigning thread-%d to CPU-%d\n", i, cpu_idx);
        CPU_ZERO(&set);
        CPU_SET(cpu_idx, &set);
        pthread_attr_setaffinity_np(param.attr, sizeof(cpu_set_t), &set);
        /**
         * Three key components
         */
        switch (param.joiner) {
            case type_SHJJoiner:
                param.args[i].joiner = new SHJJoiner(relR->num_tuples,
                                                     relS->num_tuples);//have to allocate max size of hash table in each joiner.
                break;
            case type_PMJJoiner:
                param.args[i].joiner = new PMJJoiner(relR->num_tuples,
                                                     relS->num_tuples,
                                                     nthreads);//have to allocate max size of tmp window in each joiner.
                break;
            case type_RippleJoiner:
                param.args[i].joiner = new RippleJoiner(relR, relS, nthreads);
                break;
        }

        if (param.exp_id == 39) {//dataset=Rovio
            param.args[i].joiner->timer->record_gap = 1000;
        } else {
            param.args[i].joiner->timer->record_gap = 1;
        }
//        printf("record_gap:%d\n", param.args[i].joiner->timer->record_gap);

        param.args[i].nthreads = nthreads;
        param.args[i].tid = i;
        param.args[i].barrier = param.barrier;
        param.args[i].timer = param.args[i].joiner->timer;
        param.args[i].matches = &param.args[i].joiner->matches;
        param.args[i].threadresult = &(param.joinresult->resultlist[i]);
        param.args[i].shuffler = param.shuffler;//shared shuffler.

        switch (param.fetcher) {
            case type_JM_NP_Fetcher:
                param.args[i].fetcher = new JM_NP_Fetcher(nthreads, relR, relS, i, startTS,
                                                          param.args[i].joiner->timer);
                break;
            case type_JB_NP_Fetcher:
                param.args[i].fetcher = new JB_NP_Fetcher(nthreads, relR, relS, i, startTS,
                                                          param.args[i].joiner->timer);
                break;
            case type_HS_NP_Fetcher:
                param.args[i].fetcher = new HS_NP_Fetcher(nthreads, relR, relS, i, startTS,
                                                          param.args[i].joiner->timer);
                break;
            case type_PMJ_HS_NP_Fetcher:
                param.args[i].fetcher = new PMJ_HS_NP_Fetcher(nthreads, relR, relS, i, startTS,
                                                              param.args[i].joiner->timer);
                break;
        }

        rv = pthread_create(&param.tid[i], param.attr, thread_fun, (void *) &param.args[i]);
        if (rv) {
            printf("ERROR; return code from pthread_create() is %d\n", rv);
            exit(-1);
        }
//        printf("Launch thread[%d] :%lu\n", param.args[i].tid, param.tid[i]);
        fflush(stdout);
    }
}
