//
// Created by Shuhao Zhang on 17/10/19.
//


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

/**
 * print progressive results.
 * @param vector
 */
void
dump_timing(std::vector<std::chrono::milliseconds> vector, std::vector<int64_t> vector_latency, std::string arg_name,
            int exp_id, long lastTS) {

    //print progressive
    int n = vector.size() - 1;
    int check10 = ceil(n * 0.10);
    int check25 = ceil(n * 0.25);
    int check50 = ceil(n * 0.5);
    int check75 = ceil(n * 0.75);
    std::chrono::milliseconds start = vector.at(0);

    fprintf(stdout, "Time to obtain 10%%, 25%%, 50%%, 75%% of results (MSECS): \n");
    fprintf(stdout, "(%.2lu) \t (%.2lu) \t (%.2lu) \t (%.2lu)",
            vector.at(check10).count() + lastTS - start.count(),
            vector.at(check25).count() + lastTS - start.count(),
            vector.at(check50).count() + lastTS - start.count(),
            vector.at(check75).count() + lastTS - start.count());
    fprintf(stdout, "\n");
    fprintf(stdout, "\n");
    fflush(stdout);

    //dump timestmap.
    std::string name = arg_name + "_" + std::to_string(exp_id);
    string path = "/data1/xtra/results/timestamps/" + name.append(".txt");
    ofstream outputFile(path, std::ios::trunc);
    auto begin = vector.begin().operator*();
    vector.erase(vector.begin());
    for (auto &element : vector) {
        outputFile << (std::to_string(element.count() - begin.count() + lastTS) + "\n");
    }
    outputFile.close();

    //dump latency
    std::string name_latency = arg_name + "_" + std::to_string(exp_id);
    string path_latency = "/data1/xtra/results/latency/" + name_latency.append(".txt");
    ofstream outputFile_latency(path_latency, std::ios::trunc);
    for (auto &element : vector_latency) {
        outputFile_latency << (std::to_string(element + lastTS) + "\n");
    }
    outputFile_latency.close();
}

/**
 *
 * @param result
 * @param timer
 * @param lastTS  in millseconds, lazy algorithms have to wait until very last tuple arrive before proceed.
 */
void dump_breakdown(int64_t result, T_TIMER *timer, long lastTS, _IO_FILE *pFile) {
    if (result != 0) {

        double diff_usec = (((timer->end).tv_sec * 1000000L + (timer->end).tv_usec)
                            - ((timer->start).tv_sec * 1000000L + (timer->start).tv_usec)) + lastTS * 1000L;

        if (lastTS != 0) {//lazy join algorithms.
            timer->wait_timer = lastTS * 2.1 * 1000000;//MYC: 2.1GHz
            timer->overall_timer += timer->wait_timer;
        } else {//eager join algorithms.
            timer->partition_timer -= timer->wait_timer;//exclude waiting time during tuple shuffling.
        }
        double cyclestuple = (timer->overall_timer) / result;

        //for system to read.
        //only take one thread to dump?
        //WAIT, PART, BUILD, SORT, MERGE, JOIN
        fprintf(pFile, "%lu\n%lu\n%lu\n%lu\n%lu\n%lu\n",
                timer->wait_timer,
                timer->partition_timer,
                timer->buildtimer,
                timer->sorttimer,
                timer->mergetimer,
                timer->overall_timer -
                (timer->wait_timer + timer->partition_timer + timer->buildtimer + timer->sorttimer + timer->mergetimer)
        );
        fprintf(pFile, "===\n");

        //for user to read.
        fprintf(pFile, "[Info] RUNTIME TOTAL, WAIT, PART, BUILD, SORT, MERGE (cycles): \n");
        fprintf(pFile, "%llu \t %llu (%.2f%%) \t %llu (%.2f%%) \t %llu (%.2f%%)  \t %llu (%.2f%%) ",
                timer->overall_timer,
                timer->wait_timer, (timer->wait_timer * 100 / (double) timer->overall_timer),
                timer->partition_timer, (timer->partition_timer * 100 / (double) timer->overall_timer),
                timer->buildtimer, (timer->buildtimer * 100 / (double) timer->overall_timer),
                timer->sorttimer, (timer->sorttimer * 100 / (double) timer->overall_timer),
                timer->mergetimer, (timer->mergetimer * 100 / (double) timer->overall_timer)
        );
        fprintf(pFile, "\n");
        fprintf(pFile, "TOTAL-TIME-USECS, NUM-TUPLES, CYCLES-PER-TUPLE: \n");
        fprintf(pFile, "%.4lf \t %ld \t %.4lf", diff_usec, result, cyclestuple);
        fprintf(pFile, "\n");
        fprintf(pFile, "\n");
    } else {
        fprintf(pFile, "[Warning] This thread does not matches any tuple.\n\n");
    }
    fflush(pFile);
}

milliseconds actual_start_timestamp;
std::vector<std::chrono::milliseconds> global_record;
std::vector<int64_t> global_record_latency;

void merge(T_TIMER *timer, relation_t *relR, relation_t *relS, milliseconds *startTS) {
#ifdef MEASURE
    //For progressiveness measurement
    actual_start_timestamp = *startTS;
    for (auto i = 0; i < timer->recordR.size(); i++) {
        global_record.push_back(timer->recordR.at(i));
    }
    for (auto i = 0; i < timer->recordS.size(); i++) {
        global_record.push_back(timer->recordS.at(i));
    }
    //For latency measurement
    int64_t latency = -1;
    for (auto i = 0; i < timer->recordRID.size(); i++) {
        latency =
                timer->recordR.at(i).count() - startTS->count()
                - relR->payload->ts[timer->recordRID.at(i)].count();//latency of one tuple.
        global_record_latency.push_back(latency);
    }
    for (auto i = 0; i < timer->recordSID.size(); i++) {
        latency =
                timer->recordS.at(i).count() - startTS->count() -
                relS->payload->ts[timer->recordSID.at(i)].count();//latency of one tuple.
        global_record_latency.push_back(latency);
    }
#endif
}

/**
 * TODO: check why the second sort causes many memory errors.
 * "Conditional jump or move depends on uninitialised value(s)"
 * @param algo_name
 */
void sortRecords(std::string algo_name, int exp_id, long lastTS) {

    //sort the global record to get to know the actual time when each match success.
    global_record.push_back(actual_start_timestamp);
    sort(global_record.begin(), global_record.end());
    sort(global_record_latency.begin(), global_record_latency.end());
    /* now print the progressive results: */
    dump_timing(global_record, global_record_latency, algo_name, exp_id, lastTS);

}

milliseconds now() {
    milliseconds ms = duration_cast<milliseconds>(
            system_clock::now().time_since_epoch()
    );
    return ms;
}
