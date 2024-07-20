//
// Created by Shuhao Zhang on 1/11/19.
//

#include "launcher.h"

void
launch(int nthreads, relation_t *relR, relation_t *relS, t_param param, void *(*thread_fun)(void *),
       uint64_t *startTS, uint64_t *joinStart) {
    int i;
    int rv;
    cpu_set_t set;
#ifdef JOIN_RESULT_MATERIALIZE
    param.joinresult->resultlist = (threadresult_t *) malloc(sizeof(threadresult_t)
                                                             * nthreads);
#endif

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
                //have to allocate max size of hash table in each joiner.
                param.args[i].joiner = new SHJJoiner(relR->num_tuples,
                                                     relS->num_tuples / nthreads);
                break;
            case type_PMJJoiner:
                //have to allocate max size of tmp window in each joiner.
                param.args[i].joiner = new PMJJoiner(relR->num_tuples,
                                                     relS->num_tuples,
                                                     nthreads);
                /** configure parameters for PMJ. */
                ((PMJJoiner *) param.args[i].joiner)->progressive_step_R = (int)
                        (relR->num_tuples / nthreads / 100.0 * param.progressive_step);
                ((PMJJoiner *) param.args[i].joiner)->progressive_step_S =
                        (int) (relS->num_tuples / nthreads / 100.0 * param.progressive_step);
                ((PMJJoiner *) param.args[i].joiner)->merge_step = param.merge_step;

                break;
            case type_RippleJoiner:
                param.args[i].joiner = new RippleJoiner(relR, relS, nthreads);
                break;
        }

#ifndef NO_TIMING
        param.args[i].joiner->timer->record_gap = param.record_gap;
        DEBUGMSG("record_gap:%d\n", param.args[i].joiner->timer->record_gap);
#endif

        param.args[i].nthreads = nthreads;
        param.args[i].tid = i;
        param.args[i].barrier = param.barrier;
        param.args[i].timer = param.args[i].joiner->timer;
        param.args[i].matches = &param.args[i].joiner->matches;
        param.args[i].threadresult = &(param.joinresult->resultlist[i]);
        param.args[i].shuffler = param.shuffler;//shared shuffler.
        param.args[i].startTS = startTS;
        param.args[i].exp_id = param.exp_id;

        // SAMPLE
        static int mod = int(1e9)+7;
        // param.grp_id = .grp_id;

        param.args[i].joiner->epsilon_r = param.epsilon_r;
        param.args[i].joiner->epsilon_s = param.epsilon_s;
        param.args[i].joiner->data_utilization_r = param.data_utilization_r;
        param.args[i].joiner->data_utilization_s = param.data_utilization_s;
        param.args[i].joiner->Epsilon = param.epsilon_r*mod;
        param.args[i].joiner->Bernoulli_q = param.Bernoulli_q*mod;
        param.args[i].joiner->Universal_p = param.Universal_p*mod;
        if (typeid(* param.args[i].joiner) == typeid(SHJJoiner))
        {
            dynamic_cast<SHJJoiner*>(param.args[i].joiner)->hash_key_p = param.Universal_p;
        }
        param.args[i].joiner->reservior_size = param.reservior_size;
        param.args[i].joiner->rand_buffer_size = param.rand_buffer_size;
        param.args[i].joiner->presample_size = param.presample_size;
        // param.args[i].joiner->hash_a = rand();
        // param.args[i].joiner->hash_b = rand();
        param.args[i].joiner->rand_que = NULL;
        param.args[i].joiner->que_head = 0;
        param.args[i].joiner->gm[1][1] = param.args[i].joiner->gm[2][2] = param.args[i].joiner->gm[2][1] = param.args[i].joiner->gm[1][2] = 0;
        // count_pre = pre_smp_thrshld = 1000;
        param.args[i].joiner->count_pre = 0;
        // div_prmtr = 100;
        param.args[i].joiner->hash_a = rand()%mod;
        srand(time(NULL));
        while(param.args[i].joiner->hash_a < 1000) 
            param.args[i].joiner->hash_a = rand()%mod;
        param.args[i].joiner->hash_b = rand()%mod;
        param.args[i].joiner->pre_smp[0] = new std::unordered_map<intkey_t, int>;
        param.args[i].joiner->pre_smp[1] = new std::unordered_map<intkey_t, int>;
        
        switch (param.fetcher) {
            case type_JM_P_Fetcher:
                param.args[i].fetcher = new JM_P_Fetcher(nthreads, relR, relS, i,
                                                          param.args[i].joiner->timer);
                break;
            case type_JM_NP_Fetcher:
                param.args[i].fetcher = new JM_NP_Fetcher(nthreads, relR, relS, i,
                                                          param.args[i].joiner->timer);
                break;
            case type_JB_P_Fetcher:
                param.args[i].fetcher = new JB_P_Fetcher(nthreads, relR, relS, i,
                                                          param.args[i].joiner->timer);
                break;
            case type_JB_NP_Fetcher:
                param.args[i].fetcher = new JB_NP_Fetcher(nthreads, relR, relS, i,
                                                          param.args[i].joiner->timer);
                break;
            case type_HS_NP_Fetcher:
                param.args[i].fetcher = new HS_NP_Fetcher(nthreads, relR, relS, i,
                                                          param.args[i].joiner->timer);
                break;
            case type_PMJ_HS_NP_Fetcher:
                param.args[i].fetcher = new PMJ_HS_NP_Fetcher(nthreads, relR, relS, i,
                                                              param.args[i].joiner->timer);
                break;
        }
        rv = pthread_create(&param.tid[i], param.attr, thread_fun, (void *) &param.args[i]);
        if (rv) {
            MSG("ERROR; return code from pthread_create() is %d\n", rv);
            exit(-1);
        }
//        MSG("Launch thread[%d] :%lu\n", param.args[i].tid, param.tid[i]);
        fflush(stdout);
    }
    // TODO: add a timer here, need to have global view?
    *joinStart = curtick();
}
