//
// Created by Shuhao Zhang on 1/11/19.
//

#ifndef ALLIANCEDB_LAUNCHER_H
#define ALLIANCEDB_LAUNCHER_H

#include <iostream>
#include <list>
#include "../utils/xxhash64.h"
#include "../utils/barrier.h"
#include "../joins/common_functions.h"
#include "../utils/t_timer.h"  /* startTimer, stopTimer */
#include "../utils/generator.h"          /* numa_localize() */
#include "../utils/cpu_mapping.h"        /* get_cpu_id */
#include "../utils/lock.h"               /* lock, unlock */
#include "../joins/npj_types.h"          /* bucket_t, hashtable_t, bucket_buffer_t */
#include "../joins/npj_params.h"         /* constant parameters */
#include "../joins/onlinejoins.h"
#include <sys/time.h>           /* gettimeofday */
#include <stdlib.h>             /* memalign */
#include <stdio.h>              /* printf */
#include <string.h>             /* memset */
#include <pthread.h>            /* pthread_* */
#include <sched.h>              /* CPU_ZERO, CPU_SET */
#include "../joins/shj_struct.h"

void
launch(int nthreads, relation_t *relR, relation_t *relS, t_param param, T_TIMER *timer, void *(*thread_fun)(void *));

#ifndef LAUNCH
#define LAUNCH(nthreads, relR, relS, param, timer, thread_fun) \
        launch(nthreads, relR, relS, param,  &timer,thread_fun);
#endif

#endif //ALLIANCEDB_LAUNCHER_H
