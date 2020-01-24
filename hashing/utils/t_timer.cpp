//
// Created by Shuhao Zhang on 17/10/19.
//

#include "../helper/thread_task.h"
#include "../helper/launcher.h"
#include "../utils/barrier.h"
#include "../utils/t_timer.h"  /* startTimer, stopTimer */
#include <sys/time.h>           /* gettimeofday */
#include <stdlib.h>             /* memalign */
#include <stdio.h>              /* printf */
#include <string.h>             /* memset */
#include <pthread.h>            /* pthread_* */
#include <sstream>
#include <zconf.h>
#include "t_timer.h"


using namespace std;

std::string GetCurrentWorkingDir(void) {
    char buff[FILENAME_MAX];
    getcwd(buff, FILENAME_MAX);
    std::string current_working_dir(buff);
    return current_working_dir;
}

void print_timing(std::vector<std::chrono::milliseconds> vector, std::vector<int64_t> vector_latency, std::string arg_name) {

    //progressive and throughput.
    std::string name = arg_name;
    string path = "/data1/xtra/results/" + name.append("_timestamps.txt");
    ofstream outputFile(path, std::ios::trunc);
    int n = vector.size() - 1;
    int check25 = ceil(n * 0.25);
    int check50 = ceil(n * 0.5);
    int check75 = ceil(n * 0.75);
    std::chrono::milliseconds start = vector.at(0);
    fprintf(stdout, "Time to obtain 25%%, 50%%, 75%% of results (MSECS): \n");
    fprintf(stdout, "(%.2lu) \t (%.2lu) \t (%.2lu)", vector.at(check25) - start, vector.at(check50) - start,
            vector.at(check75) - start);
    fprintf(stdout, "\n");
    fflush(stdout);

//    outputFile << "\n==Detailed Timestamps==\n";
    auto begin = vector.begin().operator*();
    vector.erase(vector.begin());
    for (auto &element : vector) {
        outputFile << (std::to_string(element.count() - begin.count()) + "\n");
    }
    outputFile.close();

    //latency
    std::string name_latency = arg_name;
    string path_latency = "/data1/xtra/results/" + name_latency.append("_latency.txt");
    ofstream outputFile_latency(path_latency, std::ios::trunc);
    for (auto &element : vector_latency) {
        outputFile_latency << (std::to_string(element) + "\n");
    }
    outputFile_latency.close();
}

void print_timing(int64_t result, T_TIMER *timer) {
    if (result != 0) {
        double diff_usec = (((timer->end).tv_sec * 1000000L + (timer->end).tv_usec)
                            - ((timer->start).tv_sec * 1000000L + (timer->start).tv_usec));
        double cyclestuple = timer->overall_timer / result;
        fprintf(stdout, "[Info] RUNTIME TOTAL, BUILD, SORT, PART (cycles): \n");
        fprintf(stdout, "%llu \t %llu (%.2f%%) \t %llu (%.2f%%)   \t %llu (%.2f%%) ",
                timer->overall_timer,
                timer->buildtimer, (timer->buildtimer * 100 / (double) timer->overall_timer),
                timer->sorttimer, (timer->sorttimer * 100 / (double) timer->overall_timer),
                timer->partition_timer, (timer->partition_timer * 100 / (double) timer->overall_timer));
        fprintf(stdout, "\n");
        fprintf(stdout, "TOTAL-TIME-USECS, NUM-TUPLES, CYCLES-PER-TUPLE: \n");
        fprintf(stdout, "%.4lf \t %ld \t %.4lf", diff_usec, result, cyclestuple);
        fprintf(stdout, "\n");
        fprintf(stdout, "\n");
        fflush(stdout);
    } else {
        fprintf(stdout, "[Warning] This thread does not matches any tuple.\n\n");
    }
}

std::chrono::milliseconds actual_start_timestamp = std::chrono::milliseconds::max();
std::vector<std::chrono::milliseconds> global_record;
std::vector<int64_t > global_record_latency;

void merge(T_TIMER *timer, relation_t *relR, relation_t *relS) {
#ifdef MEASURE
    //For progressiveness measurement
    auto start = timer->record.at(0);
    if (actual_start_timestamp.count() > start.count()) {
        actual_start_timestamp = start;
    }
    for (auto i = 1; i < timer->record.size(); i++) {
        global_record.push_back(timer->record.at(i));
    }

    //For latency measurement
    int64_t latency = -1;
    for (auto i = 0; i < timer->recordRID.size(); i++) {
        latency =
                timer->record.at(i + 1).count() - actual_start_timestamp.count() -
                relR->payload->ts[timer->recordRID.at(i)].count();//latency of one tuple.
        global_record_latency.push_back(latency);
    }
    for (auto i = 0; i < timer->recordSID.size(); i++) {
        latency =
                timer->record.at(i + 1).count() - actual_start_timestamp.count() -
                relS->payload->ts[timer->recordSID.at(i)].count();//latency of one tuple.
        global_record_latency.push_back(latency);
    }
#endif
}

void sort(string algo_name) {
    //sort the global record to get to know the actual time when each match success.
    global_record.push_back(actual_start_timestamp);
    sort(global_record.begin(), global_record.end());
    sort(global_record_latency.begin(), global_record_latency.end());
    /* now print the progressive results: */
    print_timing(global_record, global_record_latency, algo_name);
}

milliseconds now() {
    milliseconds ms = duration_cast<milliseconds>(
            system_clock::now().time_since_epoch()
    );
    return ms;
}
