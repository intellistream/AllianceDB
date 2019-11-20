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
        DEBUGMSG("Assigning thread-%d to CPU-%d\n", i, cpu_idx);
        CPU_ZERO(&set);
        CPU_SET(cpu_idx, &set);
        pthread_attr_setaffinity_np(param.attr, sizeof(cpu_set_t), &set);
        param.args[i].nthreads = nthreads;
        param.args[i].tid = i;
        param.args[i].timer = timer;
        param.args[i].barrier = param.barrier;
        param.args[i].threadresult = &(param.joinresult->resultlist[i]);

        /**
         * Three key components
         */
        param.args[i].fetcher = param.fetcher;
        param.args[i].shuffler = param.shuffler;
        param.args[i].joiner = param.joiner;

        rv = pthread_create(&param.tid[i], param.attr, thread_fun, (void *) &param.args[i]);
        if (rv) {
            printf("ERROR; return code from pthread_create() is %d\n", rv);
            exit(-1);
        }
//        printf("Launch thread[%d] :%lu\n", param.args[i].tid, param.tid[i]);
        fflush(stdout);
    }
}
