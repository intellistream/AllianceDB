//
// Created by Shuhao Zhang on 17/10/19.
//

#ifndef ALLIANCEDB_T_TIMER_H
#define ALLIANCEDB_T_TIMER_H


#include "../utils/rdtsc.h"              /* startTimer, stopTimer */
#include <sys/time.h>           /* gettimeofday */
#include <stdio.h>              /* printf */
#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>
#include "types.h"


#include <chrono>

using namespace std::chrono;

#define MEASURE
//thread_local structure.
struct T_TIMER {
#ifdef MEASURE
    struct timeval start, end;
    uint64_t overall_timer, partition_timer;
    uint64_t buildtimer_pre = 0, buildtimer = 0;
    uint64_t debuildtimer_pre = 0, debuildtimer = 0;//buildtimer is accumulated.
    uint64_t sorttimer_pre = 0, sorttimer = 0;//accumulate.
    std::vector<std::chrono::milliseconds> recordR;
    std::vector<std::chrono::milliseconds> recordS;
    std::vector<uint64_t> recordRID;
    std::vector<uint64_t> recordSID;
    int record_cnt = 0;
    const int record_gap = 100;
#endif
};

milliseconds now();


/** print out the execution time statistics of the join */
void print_breakdown(int64_t result, T_TIMER *timer, long lastTS, _IO_FILE *pFile);

void merge(T_TIMER *timer, relation_t *relR, relation_t *relS, milliseconds *startTS);

void sortRecords(std::string algo_name, int exp_id, long lastTS);

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

#ifndef BEGIN_MEASURE_SORT_ACC
#define BEGIN_MEASURE_SORT_ACC(timer) \
    startTimer(&timer.sorttimer_pre);
#endif

#ifndef END_MEASURE_SORT_ACC
#define END_MEASURE_SORT_ACC(timer) \
     accTimer(&timer.sorttimer_pre, &timer.sorttimer); /* sort time */
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
    timer.partition_timer = 0; /* no partitioning */
#endif

#ifndef END_MEASURE
#define END_MEASURE(timer) \
    stopTimer(&timer.overall_timer); /* overall */ \
    gettimeofday(&timer.end, NULL);
#endif

#ifndef END_PROGRESSIVE_MEASURE
#define END_PROGRESSIVE_MEASURE(payloadID, timer, IStupleR)     \
        auto ts =now();\
        if(timer.record_cnt % timer.record_gap==0){             \
            if(IStupleR){                                        \
                timer.recordRID.push_back(payloadID);           \
                timer.recordR.push_back(ts);                     \
            }else{                                                \
                timer.recordSID.push_back(payloadID);           \
                timer.recordS.push_back(ts);                    \
                }                                               \
        }                                                       \
        timer.record_cnt++;
#endif


#endif //ALLIANCEDB_T_TIMER_H
