//
// Created by Shuhao Zhang on 17/10/19.
//

#ifndef ALLIANCEDB_T_TIMER_H
#define ALLIANCEDB_T_TIMER_H


#include "../utils/rdtsc.h"              /* startTimer, stopTimer */
#include <sys/time.h>           /* gettimeofday */
#include <stdio.h>              /* printf */
#include <vector>

//thread_local structure.
struct T_TIMER {
#ifndef NO_TIMING
    struct timeval start, end;
    uint64_t overall_timer, partition_timer;
    uint64_t progressivetimer = 0;
    uint64_t buildtimer_pre = 0, buildtimer = 0, debuildtimer_pre = 0, debuildtimer = 0;//buildtimer is accumulated.
    std::vector<uint64_t> record;

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
#ifndef BEGIN_MEASURE_DEBUILD_ACC
#define BEGIN_MEASURE_DEBUILD_ACC(timer) \
     startTimer(&timer.debuildtimer_pre); /* clean time */
#endif
#ifndef END_MEASURE_DEBUILD_ACC
#define END_MEASURE_DEBUILD_ACC(timer) \
     accTimer(&timer.debuildtimer_pre, &timer.debuildtimer); /* clean time */
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
    startTimer(&timer.progressivetimer); \
    timer.partition_timer = 0; /* no partitioning */ \
    timer.record.push_back(curtick());//beginning timestamp.
#endif

#ifndef END_MEASURE
#define END_MEASURE(timer) \
    stopTimer(&timer.overall_timer); /* overall */ \
    gettimeofday(&timer.end, NULL);
#endif

#ifndef END_PROGRESSIVE_MEASURE
#define END_PROGRESSIVE_MEASURE(timer) \
    timer.record.push_back(curtick());
#endif

/**
 * print progressive results.
 * @param vector
 */
void print_timing(std::vector<uint64_t> vector);

/** print out the execution time statistics of the join */
void print_timing(uint64_t numtuples, int64_t result, T_TIMER *timer);

#endif //ALLIANCEDB_T_TIMER_H
