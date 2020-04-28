//
// Created by Shuhao Zhang on 17/10/19.
//


#include "../helper/launcher.h"
#include "../utils/barrier.h"
#include "t_timer.h"  /* startTimer, stopTimer */
#include <sys/time.h>           /* gettimeofday */
#include <stdlib.h>             /* memalign */
#include <stdio.h>              /* printf */
#include <string.h>             /* memset */
#include <pthread.h>            /* pthread_* */
#include <sstream>
#include <zconf.h>
#include <regex>


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
            int exp_id, long lastTS, unsigned long inputs, int64_t matches
) {
    std::string name = arg_name + "_" + std::to_string(exp_id);
    //print progressive
    int n = vector.size() - 1;
    int check01 = ceil(n * 0.001);
    int check1 = ceil(n * 0.01);
    int check5 = ceil(n * 0.05);
    int check10 = ceil(n * 0.1);
    int check15 = ceil(n * 0.15);
    int check25 = ceil(n * 0.25);
    int check50 = ceil(n * 0.50);
    int check75 = ceil(n * 0.75);
    int check95 = ceil(n * 0.95);
    int check99 = ceil(n * 0.99) - 1;

    //dump matches and inputs.

    string path = "/data1/xtra/results/records/" + name + ".txt";
    ofstream outputFile(path, std::ios::trunc);
    outputFile << (std::to_string(inputs) + "\n");
    outputFile << (std::to_string(matches) + "\n");
    outputFile.close();

    //dump timestmap.
    string path_ts = "/data1/xtra/results/timestamps/" + name + ".txt";
    ofstream outputFile_ts(path_ts, std::ios::trunc);
    auto begin = vector.begin().operator*();
    vector.erase(vector.begin());
    for (auto &element : vector) {
//        printf("timestamp:%f\n", (element + lastTS - begin));
        outputFile_ts << (std::to_string(element + lastTS - begin) + "\n");
    }
    outputFile_ts.close();
    fprintf(stdout, "check50:%d\n", check50);
    fprintf(stdout, "Time to obtain 1%%, 5%%, 10%%, 25%%, 50%% of results (MSECS): \n");
    fprintf(stdout, "(%.2f) \t (%.2f) \t (%.2f) \t (%.2f) \t (%.2f)",
            vector.at(check1) + lastTS - begin,
            vector.at(check5) + lastTS - begin,
            vector.at(check10) + lastTS - begin,
            vector.at(check25) + lastTS - begin,
            vector.at(check50) + lastTS - begin
    );
    fprintf(stdout, "\n");
    fprintf(stdout, "\n");
    fflush(stdout);

    //dump latency
    string path_latency = "/data1/xtra/results/latency/" + name + ".txt";
    ofstream outputFile_latency(path_latency, std::ios::trunc);
    for (auto &element : vector_latency) {
        outputFile_latency << (std::to_string(element + lastTS) + "\n");
    }
    outputFile_latency.close();

    fprintf(stdout, "95th latency: (%.2f)\t 99th latency: (%.2f)\n",
            vector_latency.at(check95), vector_latency.at(check99)
    );

    //dump gap
    string path_gap = "/data1/xtra/results/gaps/" + name + ".txt";
    ofstream outputFile_gap(path_gap, std::ios::trunc);
    for (auto &element : global_record_gap) {
        outputFile_gap << (std::to_string(element + lastTS) + "\n");
    }
    outputFile_gap.close();
}

int matches_in_sort_total = 0;

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
    printf("matches_in_sort_total: %d\n", matches_in_sort_total);
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

double wait_time = 0;
double partition_time = 0;
double build_time = 0;
double sort_time = 0;
double merge_time = 0;
double join_time = 0;
double others_time = 0;

/**
 * Used by eager joiners.
 * We can't synchrnoze steps in eager join algorithm.
 * We hence measure each and then averaging the results.
 * @param total_results
 * @param nthreads
 * @param average_partition_timer
 * @param txtFile
 */
void
breakdown_global(int64_t total_results, int nthreads, double average_partition_timer, std::string txtFile,
                 int window_size) {
    DEBUGMSG("average_partition_timer:%f\n", average_partition_timer);
    DEBUGMSG("txtFile:%s\n", txtFile.c_str());
    //time measurement correction
    string path;
    std::string line;

    path = "/data1/xtra/results/breakdown/partition_buildsort_probemerge_join/" + txtFile;
    std::ifstream infile0(path);
    auto t0 = 0.0;
    while (std::getline(infile0, line)) {
        std::regex newlines_re("\n+");
        auto resultstr = std::regex_replace(line, newlines_re, "");
        t0 += std::stod(resultstr);
    }

    wait_time = average_partition_timer - t0 / nthreads;//corrects for wait_time.
    if (window_size == 0) {
        //there is no waiting time for this dataset anyway.
        wait_time = 0;
    }

    if (txtFile.find("PMJ") != std::string::npos) {
        auto t1 = 0.0;
        path = "/data1/xtra/results/breakdown/partition_buildsort_probemerge_only/" + txtFile;
        std::ifstream infile(path);
        while (std::getline(infile, line)) {
            std::regex newlines_re("\n+");
            auto resultstr = std::regex_replace(line, newlines_re, "");
            t1 += std::stod(resultstr);
            DEBUGMSG("t1:%f\n", t1 / nthreads);
        }

        join_time = average_partition_timer - wait_time - t1 / nthreads;

        auto t2 = 0.0;
        path = "/data1/xtra/results/breakdown/partition_buildsort_only/" + txtFile;
        std::ifstream infile2(path);
        while (std::getline(infile2, line)) {
            DEBUGMSG("Partition 2:%s\n", line.c_str());
            std::regex newlines_re("\n+");
            auto resultstr = std::regex_replace(line, newlines_re, "");
            t2 += std::stod(resultstr);//corrects for merge for PMJ.
        }
        merge_time = average_partition_timer - wait_time - join_time - t2 / nthreads;//corrects for merge for PMJ.

    } else {
        path = "/data1/xtra/results/breakdown/partition_buildsort_only/" + txtFile;
        std::ifstream infile2(path);
        auto t1 = 0.0;
        while (std::getline(infile2, line)) {
            DEBUGMSG("Partition 2:%s\n", line.c_str());
            std::regex newlines_re("\n+");
            auto resultstr = std::regex_replace(line, newlines_re, "");
            t1 += std::stod(resultstr);//corrects for joiner for SHJ.

        }
        DEBUGMSG("t1:%f\n", t1 / nthreads);
        join_time = average_partition_timer - wait_time - t1 / nthreads;
    }

    path = "/data1/xtra/results/breakdown/partition_only/" + txtFile;
    std::ifstream infile3(path);
    auto t1 = 0.0;
    while (std::getline(infile3, line)) {
        std::regex newlines_re("\n+");
        auto resultstr = std::regex_replace(line, newlines_re, "");
        t1 += std::stod(resultstr);
    }

    if (txtFile.find("SHJ") != std::string::npos) {
        build_time = average_partition_timer - wait_time - join_time - merge_time -
                     t1 / nthreads;//corrects for buildtimer for SHJ.
        DEBUGMSG("build timer: %f\n", build_time);
    } else {
        sort_time = average_partition_timer - wait_time - join_time - merge_time -
                    t1 / nthreads;//corrects for buildtimer for PMJ.
        DEBUGMSG("sort timer: %f\n", sort_time);
    }
    partition_time = t1 / nthreads;//corrects for partition_timer.
    DEBUGMSG("partition timer: %f\n", partition_time);


    path = "/data1/xtra/results/breakdown/" + txtFile;
    auto fp = fopen(path.c_str(), "w");


    printf("%f\n%f\n%f\n%f\n%f\n%f\n%f\n",
           (double) wait_time / total_results,
           (double) partition_time / total_results,
           (double) build_time / total_results,
           (double) sort_time / total_results,
           (double) merge_time / total_results,
           (double) join_time / total_results,
           others_time / total_results
    );
    printf("matches_in_sort_total: %d\n", matches_in_sort_total);
    fprintf(fp, "%f\n%f\n%f\n%f\n%f\n%f\n%f\n",
            (double) wait_time / total_results,
            (double) partition_time / total_results,
            (double) build_time / total_results,
            (double) sort_time / total_results,
            (double) merge_time / total_results,
            (double) join_time / total_results,
            others_time / total_results
    );
    fflush(fp);
}

void dump_partition_cost(T_TIMER *timer, _IO_FILE *pFile) {

    printf("partition cost: %lu\n", timer->partition_timer);
    fprintf(pFile, "%lu\n", timer->partition_timer);
    fflush(pFile);
}

//
//void breakdown_thread(int64_t result, T_TIMER *timer, long tid, string file_name) {
//#ifndef NO_TIMING
//
//    if (result != 0) {
//        //time measurement correction
//        string path;
//        int lineid = 0;
//        std::string line;
//
//        if (file_name.find("PMJ") != std::string::npos) {
//            path = "/data1/xtra/results/breakdown/partition_buildsort_probemerge_only/" + file_name;
//            printf("Reading%s\n", path.c_str());
//            std::ifstream infile(path);
//            while (std::getline(infile, line)) {
//                if (lineid == tid) {
//                    std::regex newlines_re("\n+");
//                    auto resultstr = std::regex_replace(line, newlines_re, "");
//                    if (timer->partition_timer > std::stol(resultstr)) {
//                        timer->join_timer = timer->partition_timer - std::stol(resultstr);
//                    } else {
//                        printf("This is strange: it is faster when join is enabled!\n");
//                    }
//                    DEBUGMSG("joine_timer:%ld\n", timer->join_timer);
//                }
//                lineid++;
//            }
//
//            path = "/data1/xtra/results/breakdown/partition_buildsort_only/" + file_name;
//            std::ifstream infile2(path);
//            lineid = 0;
//            while (std::getline(infile2, line)) {
//                if (lineid == tid) {
//                    DEBUGMSG("Partition 2:%s\n", line.c_str());
//                    std::regex newlines_re("\n+");
//                    auto resultstr = std::regex_replace(line, newlines_re, "");
//                    timer->mergetimer = timer->partition_timer - timer->join_timer -
//                                        std::stol(resultstr);//corrects for merge for PMJ.
//                    DEBUGMSG("merge_timer:%ld\n", timer->mergetimer);
//                }
//                lineid++;
//            }
//        } else {//shj
//            path = "/data1/xtra/results/breakdown/partition_buildsort_only/" + file_name;
//            std::ifstream infile2(path);
//            lineid = 0;
//            while (std::getline(infile2, line)) {
//                if (lineid == tid) {
//                    DEBUGMSG("Partition 2:%s\n", line.c_str());
//                    std::regex newlines_re("\n+");
//                    auto resultstr = std::regex_replace(line, newlines_re, "");
//                    if (timer->partition_timer > std::stol(resultstr)) {
//                        timer->join_timer = timer->partition_timer -
//                                            std::stol(resultstr);//corrects for joiner for SHJ.
//                    } else {
//                        printf("This is strange: it is faster when join is enabled!\n");
//                    }
//                    DEBUGMSG("merge_timer:%ld\n", timer->mergetimer);
//
//                }
//                lineid++;
//            }
//        }
//        path = "/data1/xtra/results/breakdown/partition_only/" + file_name;
//        std::ifstream infile3(path);
//        lineid = 0;
//        while (std::getline(infile3, line)) {
//            if (lineid == tid) {
//                std::regex newlines_re("\n+");
//                auto resultstr = std::regex_replace(line, newlines_re, "");
//                if (file_name.find("SHJ") != std::string::npos) {
//                    timer->buildtimer = timer->partition_timer - timer->join_timer - timer->mergetimer -
//                                        std::stol(resultstr);//corrects for buildtimer for SHJ.
//                    printf("build timer: %ld\n", timer->buildtimer);
//                } else {
//                    timer->sorttimer = timer->partition_timer - timer->join_timer - timer->mergetimer -
//                                       std::stol(resultstr);//corrects for buildtimer for PMJ.
//                    DEBUGMSG("sort timer: %ld\n", timer->sorttimer);
//                }
//                timer->partition_timer = std::stol(resultstr);//corrects for partition_timer.
//                DEBUGMSG("partition timer: %ld\n", timer->partition_timer);
//            }
//            lineid++;
//        }
//
//        //WAIT, PART, BUILD, SORT, MERGE, JOIN, OTHERS
//        auto others = (timer->overall_timer -
//                       (timer->wait_timer + timer->partition_timer + timer->buildtimer + timer->sorttimer +
//                        timer->mergetimer + timer->join_timer));
//
//        matches_in_sort_total += timer->matches_in_sort;
////
//        wait_time += timer->wait_timer;
//        partition_time += timer->partition_timer;
//        build_time += timer->buildtimer;
//        sort_time += timer->sorttimer;
//        merge_time += timer->mergetimer;
//        join_time += timer->join_timer;
//        others_time += others;
//
//        //for user to read.
//        printf("[Info] RUNTIME TOTAL, WAIT, PART, BUILD, SORT, MERGE, JOIN, others (cycles): \n");
//        printf("%llu \t %llu (%.2f%%) \t %llu (%.2f%%) \t %llu (%.2f%%)  "
//               "\t %llu (%.2f%%)  \t %llu (%.2f%%) \t %llu (%.2f%%) \t %llu (%.2f%%)\n",
//               timer->overall_timer,
//               timer->wait_timer, (timer->wait_timer * 100 / (double) timer->overall_timer),
//               timer->partition_timer, (timer->partition_timer * 100 / (double) timer->overall_timer),
//               timer->buildtimer, (timer->buildtimer * 100 / (double) timer->overall_timer),
//               timer->sorttimer, (timer->sorttimer * 100 / (double) timer->overall_timer),
//               timer->mergetimer, (timer->mergetimer * 100 / (double) timer->overall_timer),
//               timer->join_timer, (timer->join_timer * 100 / (double) timer->overall_timer),
//               others, (others * 100 / (double) timer->overall_timer)
//        );
//    } else {
//        DEBUGMSG("[Warning] This thread does not matches any tuple.\n\n");
//    }
//#endif
//}

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
    int64_t latency = -1;
    int32_t gap = 0;
    auto Rrecord_size = timer->recordRID.size();
    for (auto i = 0; i < Rrecord_size; i++) {
        latency =
                timer->recordR.at(i) - actual_start_timestamp
                - relR->payload->ts[timer->recordRID.at(i)]//12537240 ~ 9205048
                + (uint64_t) (lastTS * 2.1 * 1E6);//waiting for the last tuple.
//        if (latency < 0)
//            latency = 0;
        global_record_latency.push_back(latency / (2.1 * 1E6));//cycle to ms

        gap = (int32_t) timer->recordRID.at(i) - i;//if it's sequentially processed, gap should be zero.
        global_record_gap.push_back(gap);
    }

    auto Srecord_size = timer->recordSID.size();
    for (auto i = 0; i < Srecord_size; i++) {
        latency =
                timer->recordS.at(i) - actual_start_timestamp //cycles
                - relS->payload->ts[timer->recordSID.at(i)]//cycles
                + (uint64_t) (lastTS * 2.1 * 1E6);//latency of one tuple.
//        if (latency < 0)
//            latency = 0;
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

