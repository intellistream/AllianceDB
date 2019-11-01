//
// Created by Shuhao Zhang on 17/10/19.
//

#ifndef ALLIANCEDB_T_TIMER_H
#define ALLIANCEDB_T_TIMER_H


#include "../utils/rdtsc.h"              /* startTimer, stopTimer */
#include <sys/time.h>           /* gettimeofday */
#include <stdio.h>              /* printf */

struct T_TIMER {
#ifndef NO_TIMING
    struct timeval start, end;
    uint64_t overall_timer, partition_timer, buildtimer_pre = 0, buildtimer = 0;//buildtimer is accumulated.
    uint64_t progressivetimer[3];//array of progressive T_TIMER.
#endif
};

#ifndef BEGIN_MEASURE_BUILD
#define BEGIN_MEASURE_BUILD(timer) \
    startTimer(&timer.buildtimer);
#endif

#ifndef END_MEASURE_BUILD
#define END_MEASURE_BUILD(timer) \
    stopTimer(&timer.buildtimer);
#endif

#ifndef BEGIN_MEASURE_BUILD_ACC
#define BEGIN_MEASURE_BUILD_ACC(timer) \
    startTimer(&timer.buildtimer_pre);
#endif

#ifndef END_MEASURE_BUILD_ACC
#define END_MEASURE_BUILD_ACC(timer) \
     accTimer(&timer.buildtimer_pre, &timer.buildtimer); /* build time */
#endif

#ifndef BEGIN_MEASURE_PARTITION
#define BEGIN_MEASURE_PARTITION(timer) \
   startTimer(&timer.partition_timer);
#endif

#ifndef END_MEASURE_PARTITION
#define END_MEASURE_PARTITION(timer) \
   stopTimer(&timer.partition_timer);
#endif

#ifndef START_MEASURE
#define START_MEASURE(timer) \
    gettimeofday(&timer.start, NULL); \
    startTimer(&timer.overall_timer); \
    startTimer(&timer.progressivetimer[0]); \
    startTimer(&timer.progressivetimer[1]); \
    startTimer(&timer.progressivetimer[2]); \
    timer.partition_timer = 0; /* no partitioning */
#endif

#ifndef END_MEASURE
#define END_MEASURE(timer) \
    stopTimer(&timer.overall_timer); /* overall */ \
    gettimeofday(&timer.end, NULL);
#endif

/** print out the execution time statistics of the join */
void print_timing(uint64_t numtuples, int64_t result, T_TIMER *timer);

#endif //ALLIANCEDB_T_TIMER_H
