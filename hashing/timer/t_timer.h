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
#include "../utils/types.h"
#include <chrono>

using namespace std::chrono;

//thread_local structure.

struct T_TIMER {
#ifndef NO_TIMING
    struct timeval start, end;
    uint64_t overall_timer, partition_timer_pre, partition_timer;
    uint64_t wait_timer_pre = 0, wait_timer = 0;
    uint64_t buildtimer_pre = 0, buildtimer = 0;
    uint64_t debuildtimer_pre = 0, debuildtimer = 0;//buildtimer is accumulated.
    uint64_t sorttimer_pre = 0, sorttimer = 0;//accumulate.
    uint64_t mergetimer_pre = 0, mergetimer = 0;//accumulate.
    uint64_t join_partitiontimer_pre = 0, join_partitiontimer = 0;//join during partition.
    uint64_t join_mergetimer_pre = 0, join_mergetimer = 0;//join during merge.
    uint64_t join_timer_pre = 0, join_timer = 0;//join.
    uint64_t shuffle_timer_pre = 0, shuffle_timer = 0;//shuffle.
    uint64_t garbage_time = 0;//garbage.
    std::vector<uint64_t> recordR;
    std::vector<uint64_t> recordS;
    std::vector<int32_t> recordRID;
    std::vector<int32_t> recordSID;
    int match_cnt = 0;
    int joiner_cnt = 0;
    int record_gap = 10;
    int matches_in_sort = 0;
#endif
};

/** print out the execution time statistics of the join, used by eager ones */
void breakdown_global(int64_t total_results, int nthreads, long lastTS, _IO_FILE *pFile);

/** print out the execution time statistics of the join, used by lazy ones */
void breakdown_global(int64_t result, int nthreads, T_TIMER *timer, long lastTS, _IO_FILE *pFile);

void dump_partition_cost(T_TIMER *timer, _IO_FILE *pFile);

/** print out the execution time statistics of the join */
void breakdown_thread(int64_t result, T_TIMER *timer, long tid, string name);

void merge(T_TIMER *timer, relation_t *relR, relation_t *relS, uint64_t *startTS, long lastTS);

void sortRecords(std::string algo_name, int exp_id, long lastTS);

#ifndef JOIN_COUNT
#define JOIN_COUNT(timer) \
    timer->joiner_cnt++;
#endif



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

#ifndef BEGIN_MEASURE_MERGE_ACC
#define BEGIN_MEASURE_MERGE_ACC(timer) \
    startTimer(&timer->mergetimer_pre);
#endif

#ifndef END_MEASURE_MERGE_ACC
#define END_MEASURE_MERGE_ACC(timer) \
    accTimer(&timer->mergetimer_pre, &timer->mergetimer); /* merge time */
#endif

#ifndef BEGIN_MEASURE_JOIN_MERGE_ACC
#define BEGIN_MEASURE_JOIN_MERGE_ACC(timer) \
    startTimer(&timer->join_mergetimer_pre);
#endif

#ifndef END_MEASURE_JOIN_MERGE_ACC
#define END_MEASURE_JOIN_MERGE_ACC(timer) \
     accTimer(&timer->join_mergetimer_pre, &timer->join_mergetimer); /* merge time */
#endif

#ifndef BEGIN_MEASURE_JOIN_ACC
#define BEGIN_MEASURE_JOIN_ACC(timer) \
     startTimer(&timer->join_timer_pre);
#endif

#ifndef END_MEASURE_JOIN_ACC
#define END_MEASURE_JOIN_ACC(timer) \
    accTimer(&timer->join_timer_pre, &timer->join_timer );  /* join time */
#endif

#ifndef BEGIN_MEASURE_WAIT_ACC
#define BEGIN_MEASURE_WAIT_ACC(timer) \
    startTimer(&timer->wait_timer_pre); /* wait time */
#endif

#ifndef END_MEASURE_WAIT_ACC
#define END_MEASURE_WAIT_ACC(timer) \
    accTimer(&timer->wait_timer_pre, &timer->wait_timer); /* wait time */
#endif

#ifndef SET_WAIT_ACC
#define SET_WAIT_ACC(timer, delay) \
     timer->wait_timer=delay; /* wait time */
#endif

#ifndef /*BEGIN_MEASURE_DEBUILD_ACC*/NO_TIMING
#define BEGIN_MEASURE_DEBUILD_ACC(timer) \
     startTimer(&timer->debuildtimer_pre); /* clean time */
#endif
#ifndef /*END_MEASURE_DEBUILD_ACC*/NO_TIMING
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

#ifndef BEGIN_GARBAGE
#define BEGIN_GARBAGE(timer) \
   startTimer(&timer->garbage_time);
#endif

#ifndef END_GARBAGE
#define END_GARBAGE(timer) \
   stopTimer(&timer->garbage_time);
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
        timer->match_cnt++;                                     \
        if(timer->match_cnt == timer->record_gap){              \
            if(IStupleR){                                        \
                auto ts =curtick();                              \
                timer->recordRID.push_back(payloadID);           \
                timer->recordR.push_back(ts);                    \
            }else{                                               \
                auto ts =curtick();                              \
                timer->recordSID.push_back(payloadID);           \
                timer->recordS.push_back(ts);                    \
                }                                                \
            timer->match_cnt=0;                                 \
        }
#endif

#ifndef DUMMY
#pragma GCC push_options
#pragma GCC optimize ("O0")
#define DUMMY()                             \
    double sum=0;                              \
    for (short d = 0; d < 100; d++) { \
        sum++; \
    }
#endif
#pragma GCC pop_options
#endif //ALLIANCEDB_T_TIMER_H
