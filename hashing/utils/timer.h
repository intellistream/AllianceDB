//
// Created by Shuhao Zhang on 17/10/19.
//

#ifndef ALLIANCEDB_TIMER_H
#define ALLIANCEDB_TIMER_H


#include "../utils/rdtsc.h"              /* startTimer, stopTimer */
#include <sys/time.h>           /* gettimeofday */
#include <stdio.h>              /* printf */

void print_timing(uint64_t total, uint64_t build, uint64_t part,
             uint64_t numtuples, int64_t result,
             struct timeval *start, struct timeval *end);

#endif //ALLIANCEDB_TIMER_H
