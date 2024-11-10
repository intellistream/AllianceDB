//
// Created by Shuhao Zhang on 1/11/19.
//

#ifndef ALLIANCEDB_THREAD_TASK_H
#define ALLIANCEDB_THREAD_TASK_H

#include <iostream>
#include <list>
#include "../utils/xxhash64.h"
#include "../utils/barrier.h"
#include "../joins/common_functions.h"
#include "../timer/t_timer.h"  /* startTimer, stopTimer */
#include "../utils/generator.h"          /* numa_localize() */
#include "../utils/cpu_mapping.h"        /* get_cpu_id */
#include "../utils/lock.h"               /* lock, unlock */
#include "../joins/npj_types.h"          /* bucket_t, hashtable_t, bucket_buffer_t */
#include "../joins/npj_params.h"         /* constant parameters */
#include "../joins/onlinejoins.h"
#include "../joins/batcher.h"
#include <sys/time.h>           /* gettimeofday */
#include <stdlib.h>             /* memalign */
#include <stdio.h>              /* printf */
#include <string.h>             /* memset */
#include <pthread.h>            /* pthread_* */
#include <sched.h>              /* CPU_ZERO, CPU_SET */


/**
 * Just a wrapper to call the _shj_st
 *
 * @param param the parameters of the thread, i.e. tid, ht, reln, ...
 *
 * @return
 */
void *
THREAD_TASK_NOSHUFFLE(void *param);

void *THREAD_TASK_NOSHUFFLE_BATCHED(void *param);


/**
 * Just a wrapper to call the _shj_st
 *
 * @param param the parameters of the thread, i.e. tid, ht, reln, ...
 *
 * @return
 */
void *
THREAD_TASK_SHUFFLE(void *param);

/**
 * Just a wrapper to call the _shj_st Handshake
 *
 * @param param the parameters of the thread, i.e. tid, ht, reln, ...
 *
 * @return
 */
void
*THREAD_TASK_SHUFFLE_HS(void *param);


/**
 * Special HS designed for PMJ.
 * @param param
 * @return
 */
void
*THREAD_TASK_SHUFFLE_PMJHS(void *param);

/**
 * Just a wrapper to call the _shj_st
 *
 * @param param the parameters of the thread, i.e. tid, ht, reln, ...
 *
 * @return
 */
void *
shj_thread_jb_np(void *param);

#endif //ALLIANCEDB_THREAD_TASK_H
