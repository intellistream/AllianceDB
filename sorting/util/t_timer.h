//
// Created by Shuhao Zhang on 17/10/19.
//

#ifndef ALLIANCEDB_T_TIMER_H
#define ALLIANCEDB_T_TIMER_H


#include "rdtsc.h"              /* startTimer, stopTimer */
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
    uint64_t overall_timer, partition_timer_pre, partition_timer;
    uint64_t wait_timer_pre=0, wait_timer=0;
    uint64_t buildtimer_pre = 0, buildtimer = 0;
    uint64_t debuildtimer_pre = 0, debuildtimer = 0;//buildtimer is accumulated.
    uint64_t sorttimer_pre = 0, sorttimer = 0;//accumulate.
    uint64_t mergetimer_pre = 0, mergetimer = 0;//accumulate.
    uint64_t join_mergetimer_pre = 0, join_mergetimer = 0;//join during merge.
    uint64_t join_timer_pre = 0, join_timer = 0;//join.
    std::vector<std::chrono::milliseconds> recordR;
    std::vector<std::chrono::milliseconds> recordS;
    std::vector<uint64_t> recordRID;
    std::vector<uint64_t> recordSID;
    int record_cnt = 0;
    int record_gap = 1;
#endif
};

milliseconds now();


/** print out the execution time statistics of the join */
void dump_breakdown(int64_t result, T_TIMER *timer, long lastTS, _IO_FILE *pFile);

void merge(T_TIMER *timer, relation_t *relR, relation_t *relS, milliseconds *startTS);

void sortRecords(std::string algo_name, int exp_id, long lastTS);

#ifndef BEGIN_MEASURE_BUILD
#define BEGIN_MEASURE_BUILD(timer) \
    startTimer(&timer->buildtimer);
#endif

#ifndef END_MEASURE_BUILD
#define END_MEASURE_BUILD(timer) \
    stopTimer(&timer->buildtimer);
#endif

#ifndef BEGIN_MEASURE_BUILD_ACC
#define BEGIN_MEASURE_BUILD_ACC(timer) \
    startTimer(&timer->buildtimer_pre);
#endif

#ifndef END_MEASURE_BUILD_ACC
#define END_MEASURE_BUILD_ACC(timer) \
     accTimer(&timer->buildtimer_pre, &timer->buildtimer); /* build time */
#endif

#ifndef BEGIN_MEASURE_SORT_ACC
#define BEGIN_MEASURE_SORT_ACC(timer) \
    startTimer(&timer->sorttimer_pre);
#endif

#ifndef END_MEASURE_SORT_ACC
#define END_MEASURE_SORT_ACC(timer) \
     accTimer(&timer->sorttimer_pre, &timer->sorttimer); /* sort time */
#endif

#ifndef BEGIN_MEASURE_JOIN_ACC
#define BEGIN_MEASURE_JOIN_ACC(timer) \
    startTimer(&timer->join_timer_pre);
#endif

#ifndef END_MEASURE_JOIN_ACC
#define END_MEASURE_JOIN_ACC(timer) \
     accTimer(&timer->join_timer_pre, &timer->join_timer); /* join time */
#endif

#ifndef BEGIN_MEASURE_JOIN_MERGE_ACC
#define BEGIN_MEASURE_JOIN_MERGE_ACC(timer) \
    startTimer(&timer->join_mergetimer_pre);
#endif

#ifndef END_MEASURE_JOIN_MERGE_ACC
#define END_MEASURE_JOIN_MERGE_ACC(timer) \
     accTimer(&timer->join_mergetimer_pre, &timer->join_mergetimer); /* merge time */
#endif

#ifndef BEGIN_MEASURE_MERGE_ACC
#define BEGIN_MEASURE_MERGE_ACC(timer) \
    startTimer(&timer->mergetimer_pre);
#endif

#ifndef END_MEASURE_MERGE_ACC
#define END_MEASURE_MERGE_ACC(timer) \
     accTimer(&timer->mergetimer_pre, &timer->mergetimer); /* merge time */ \
     timer->mergetimer-=timer->join_mergetimer;/*except the join time happen during merge.*/ \
     timer->join_mergetimer=0;
#endif

#ifndef BEGIN_MEASURE_WAIT_ACC
#define BEGIN_MEASURE_WAIT_ACC(timer) \
     startTimer(&timer->wait_timer_pre); /* wait time */
#endif
#ifndef END_MEASURE_WAIT_ACC
#define END_MEASURE_WAIT_ACC(timer) \
     accTimer(&timer->wait_timer_pre, &timer->wait_timer); /* wait time */
#endif

#ifndef BEGIN_MEASURE_DEBUILD_ACC
#define BEGIN_MEASURE_DEBUILD_ACC(timer) \
     startTimer(&timer->debuildtimer_pre); /* clean time */
#endif
#ifndef END_MEASURE_DEBUILD_ACC
#define END_MEASURE_DEBUILD_ACC(timer) \
     accTimer(&timer->debuildtimer_pre, &timer->debuildtimer); /* clean time */
#endif
#ifndef BEGIN_MEASURE_PARTITION_ACC
#define BEGIN_MEASURE_PARTITION_ACC(timer) \
   startTimer(&timer->partition_timer_pre);
#endif

#ifndef END_MEASURE_PARTITION_ACC
#define END_MEASURE_PARTITION_ACC(timer) \
   accTimer(&timer->partition_timer_pre, &timer->partition_timer); /* partition time */
#endif

#ifndef BEGIN_MEASURE_PARTITION
#define BEGIN_MEASURE_PARTITION(timer) \
   startTimer(&timer->partition_timer);
#endif

#ifndef END_MEASURE_PARTITION
#define END_MEASURE_PARTITION(timer) \
   stopTimer(&timer->partition_timer);
#endif

#ifndef START_MEASURE
#define START_MEASURE(timer) \
    gettimeofday(&timer->start, NULL); \
    startTimer(&timer->overall_timer); \
    timer->partition_timer = 0; /* no partitioning */
#endif

#ifndef END_MEASURE
#define END_MEASURE(timer) \
    stopTimer(&timer->overall_timer); /* overall */ \
    gettimeofday(&timer->end, NULL);
#endif

#ifndef END_PROGRESSIVE_MEASURE
#define END_PROGRESSIVE_MEASURE(payloadID, timer, IStupleR)      \
        if(timer->record_cnt == timer->record_gap){              \
            auto ts =now();                                      \
            if(IStupleR){                                        \
                timer->recordRID.push_back(payloadID);           \
                timer->recordR.push_back(ts);                    \
            }else{                                               \
                auto ts =now();                                  \
                timer->recordSID.push_back(payloadID);           \
                timer->recordS.push_back(ts);                    \
                }                                                \
            timer->record_cnt=0;                                 \
        }                                                        \
        timer->record_cnt++;
#endif


#endif //ALLIANCEDB_T_TIMER_H
