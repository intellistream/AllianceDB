//
// Created by Shuhao Zhang on 17/10/19.
//

#include "t_timer.h" /* startTimer, stopTimer */
#include "../joins/common_functions.h"
#include <algorithm>
#include <pthread.h> /* pthread_* */
#include <sstream>
#include <stdio.h>    /* printf */
#include <stdlib.h>   /* memalign */
#include <string.h>   /* memset */
#include <sys/time.h> /* gettimeofday */
#include <zconf.h>

using namespace std;

std::string GetCurrentWorkingDir(void) {
    char buff[FILENAME_MAX];
    getcwd(buff, FILENAME_MAX);
    std::string current_working_dir(buff);
    return current_working_dir;
}

/**
 * print progressive results.
 * @param vector
 */
void
dump_timing(vector<double> vector, std::vector<double> vector_latency,
            std::vector<double> global_record_gap,
            std::string arg_name,
            int exp_id, long lastTS,unsigned long inputs, int64_t matches) {

    std::string name = arg_name + "_" + std::to_string(exp_id);
    //print progressive
    int n = vector.size() - 1;
    int check01 = floor(n * 0.001);
    int check1 = floor(n * 0.01);
    int check5 = floor(n * 0.05);
    int check50 = floor(n * 0.50);
    int check75 = floor(n * 0.75);
    int check95 = floor(n * 0.95);
    int check99 = floor(n * 0.99);


    string path = EXP_DIR "/results/records/" + name + ".txt";
    ofstream outputFile(path, std::ios::trunc);
    outputFile << (std::to_string(inputs) + "\n");
    outputFile << (std::to_string(matches) + "\n");
    outputFile.close();

    //dump timestmap.

    string path_ts = EXP_DIR "/results/timestamps/" + name + ".txt";
    ofstream outputFile_ts(path_ts, std::ios::trunc);
    auto begin = vector.begin().operator*();
    vector.erase(vector.begin());
    for (auto &element : vector) {
//        printf("timestamp:%f\n", element - begin + lastTS);
        outputFile_ts << (std::to_string(element - begin + lastTS) + "\n");
    }
    outputFile_ts.close();


    fprintf(stdout, "Time to obtain 0.1%%, 1%%, 5%%, 50%%, 75%% of results (MSECS): \n");
    fprintf(stdout, "(%.2f) \t (%.2f) \t (%.2f) \t (%.2f) \t (%.2f)",
            vector.at(check01) + lastTS - begin,
            vector.at(check1) + lastTS - begin,
            vector.at(check5) + lastTS - begin,
            vector.at(check50) + lastTS - begin,
            vector.at(check75) + lastTS - begin
    );
    fprintf(stdout, "\n");
    fprintf(stdout, "\n");
    fflush(stdout);


    //dump latency
    string path_latency = EXP_DIR "/results/latency/" + name + ".txt";
    ofstream outputFile_latency(path_latency, std::ios::trunc);
    for (auto &element : vector_latency) {
        outputFile_latency << (std::to_string(element + lastTS) + "\n");
    }
    outputFile_latency.close();

    fprintf(stdout, "95th latency: (%.2f)\t 99th latency: (%.2f)\n",
            vector_latency.at(check95), vector_latency.at(check99)
    );

    //dump gap
    string path_gap = EXP_DIR "/results/gaps/" + name + ".txt";
    ofstream outputFile_gap(path_gap, std::ios::trunc);
    for (auto &element : global_record_gap) {
        outputFile_gap << (std::to_string(element + lastTS) + "\n");
    }
    outputFile_gap.close();
}

/**
 * Used by lazy joiners.
 * Only need to reference to thread-0 as all steps are synchronized.
 * @param total_results
 * @param nthreads
 * @param timer
 * @param lastTS
 * @param pFile
 */
void breakdown_global(int64_t total_results, int nthreads, T_TIMER *timer, long lastTS, _IO_FILE *pFile) {
    if (lastTS != 0) {//lazy join algorithms.
        SET_WAIT_ACC(timer, lastTS * 2.1 * 1E6)
        timer->overall_timer += timer->wait_timer;
    }
    auto others = (timer->overall_timer -
                   (timer->wait_timer + timer->partition_timer + timer->buildtimer + timer->sorttimer +
                    timer->mergetimer + timer->join_timer));
    printf("%f\n%f\n%f\n%f\n%f\n%f\n%lu\n",
           (double) timer->wait_timer / total_results,
           (double) timer->partition_timer / total_results,
           (double) timer->buildtimer / total_results,
           (double) timer->sorttimer / total_results,
           (double) timer->mergetimer / total_results,
           (double) timer->join_timer / total_results,
           others / total_results
    );

    fprintf(pFile, "%f\n%f\n%f\n%f\n%f\n%f\n%lu\n",
            (double) timer->wait_timer / total_results,
            (double) timer->partition_timer / total_results,
            (double) timer->buildtimer / total_results,
            (double) timer->sorttimer / total_results,
            (double) timer->mergetimer / total_results,
            (double) timer->join_timer / total_results,
            others / total_results
    );
    fflush(pFile);
}

/**
 *
 * @param result
 * @param timer
 * @param lastTS  in millseconds, lazy algorithms have to wait until very last tuple arrive before proceed.
 */
void breakdown_thread(int64_t result, T_TIMER *timer, long lastTS, _IO_FILE *pFile) {
#ifndef NO_TIMING
    if (result != 0) {
        double diff_usec = (((timer->end).tv_sec * 1000000L + (timer->end).tv_usec)
                            - ((timer->start).tv_sec * 1000000L + (timer->start).tv_usec)) + lastTS * 1000L;

        double cyclestuple = (timer->overall_timer) / result;

        //for system to read.
        //only take one thread to dump? YES!
        //WAIT, PART, BUILD, SORT, MERGE, JOIN, OTHERS
        auto others = (timer->overall_timer -
                       (timer->wait_timer + timer->partition_timer + timer->buildtimer + timer->sorttimer +
                        timer->mergetimer + timer->join_timer));
//
//        wait_time += timer->wait_timer / result;
//        partition_time += timer->partition_timer / result;
//        build_time += timer->buildtimer / result;
//        sort_time += timer->sorttimer / result;
//        merge_time += timer->mergetimer / result;
//        join_time += timer->join_timer / result;
//        others_time += others / result;

        //for user to read.
        DEBUGMSG(0, "[Info] RUNTIME TOTAL, WAIT, PART, BUILD, SORT, MERGE, JOIN, others (cycles): \n");
        DEBUGMSG(0, "%llu \t %llu (%.2f%%) \t %llu (%.2f%%) \t %llu (%.2f%%)  "
                    "\t %llu (%.2f%%)  \t %llu (%.2f%%) \t %llu (%.2f%%) \t %llu (%.2f%%)",
                 timer->overall_timer / result,
                 timer->wait_timer / result, (timer->wait_timer * 100 / (double) timer->overall_timer),
                 timer->partition_timer / result, (timer->partition_timer * 100 / (double) timer->overall_timer),
                 timer->buildtimer / result, (timer->buildtimer * 100 / (double) timer->overall_timer),
                 timer->sorttimer / result, (timer->sorttimer * 100 / (double) timer->overall_timer),
                 timer->mergetimer / result, (timer->mergetimer * 100 / (double) timer->overall_timer),
                 timer->join_timer / result, (timer->join_timer * 100 / (double) timer->overall_timer),
                 others / result, (others * 100 / (double) timer->overall_timer)
        );
        DEBUGMSG(1, "\n");
        DEBUGMSG(1, "TOTAL-TIME-USECS, NUM-TUPLES, CYCLES-PER-TUPLE: \n");
        DEBUGMSG(1, "%.4lf \t %ld \t %.4lf", diff_usec, result, cyclestuple);
        DEBUGMSG(1, "\n");
        DEBUGMSG(1, "\n");
    } else {
        DEBUGMSG(1, "[Warning] This thread does not matches any tuple.\n\n");
    }
#endif
}

uint64_t actual_start_timestamp;
std::vector<double> global_record;
vector<double> global_record_latency;
vector<double> global_record_gap;

void merge(T_TIMER *timer, relation_t *relR, relation_t *relS, uint64_t *startTS, long lastTS) {
#ifndef NO_TIMING
    //For progressiveness measurement
    actual_start_timestamp = *startTS;
    for (auto i = 0; i < timer->recordR.size(); i++) {
        global_record.push_back(timer->recordR.at(i) / (2.1 * 1E6));
    }
    for (auto i = 0; i < timer->recordS.size(); i++) {
        global_record.push_back(timer->recordS.at(i) / (2.1 * 1E6));
    }
    //For latency and disorder measurement
    uint64_t latency = -1;
    int32_t gap = 0;
    for (auto i = 0; i < timer->recordRID.size(); i++) {
        latency =
                timer->recordR.at(i) - actual_start_timestamp
                - relR->payload->ts[timer->recordRID.at(i)]
                + (uint64_t) (lastTS * 2.1 * 1E6);//latency of one tuple.
        global_record_latency.push_back(latency / (2.1 * 1E6));//cycle to ms

        gap = (int32_t) timer->recordRID.at(i) - i;//if it's sequentially processed, gap should be zero.
        global_record_gap.push_back(gap);
    }
    for (auto i = 0; i < timer->recordSID.size(); i++) {
        latency =
                timer->recordS.at(i) - actual_start_timestamp
                - relS->payload->ts[timer->recordSID.at(i)]
                + (uint64_t) (lastTS * 2.1 * 1E6);//latency of one tuple.
        global_record_latency.push_back(latency / (2.1 * 1E6));
        gap = (int32_t) timer->recordSID.at(i) - i;//if it's sequentially processed, gap should be zero.
        global_record_gap.push_back(gap);
    }
#endif
}


/**
 * TODO: check why the second sort causes many memory errors.
 * "Conditional jump or move depends on uninitialised value(s)"
 * @param algo_name
 */
void sortRecords(string algo_name, int exp_id, long lastTS, unsigned long inputs, int64_t matches) {

    //sort the global record to get to know the actual time when each match success.
    global_record.push_back(actual_start_timestamp / (2.1 * 1E6));//cycles to ms.
    sort(global_record.begin(), global_record.end());
    sort(global_record_latency.begin(), global_record_latency.end());
    sort(global_record_gap.begin(), global_record_gap.end());
    /* now print the progressive results: */
    dump_timing(global_record, global_record_latency, global_record_gap, algo_name, exp_id, lastTS, inputs, matches);

}
