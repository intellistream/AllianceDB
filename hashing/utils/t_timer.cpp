//
// Created by Shuhao Zhang on 17/10/19.
//

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

void print_timing(std::vector<uint64_t> vector, std::string arg_name) {
//    ofstream outputFile(GetCurrentWorkingDir().append("results/").append(arg_name).append("_timestamps.txt"));
    int n = vector.size() - 1;
    int check25 = ceil(n * 0.25);
    int check50 = ceil(n * 0.5);
    int check75 = ceil(n * 0.75);
    uint64_t start = vector.at(0);
    fprintf(stdout, "Time to obtain 25%%, 50%%, 75%% of results (USECS): \n");
    fprintf(stdout, "(%.2lu) \t (%.2lu) \t (%.2lu)", vector.at(check25) - start, vector.at(check50) - start,
            vector.at(check75) - start);
    fprintf(stdout, "\n");
    fflush(stdout);

    string s = "\n==Detailed Timestamps==\n";
    for (auto &element : vector) {
        s.append(std::to_string(element) + "\n");
    }
    printf(s.c_str());
}

void print_timing(int64_t result, T_TIMER *timer) {
    if (result != 0) {
        double diff_usec = (((timer->end).tv_sec * 1000000L + (timer->end).tv_usec)
                            - ((timer->start).tv_sec * 1000000L + (timer->start).tv_usec));
        double cyclestuple = timer->overall_timer / result;
        fprintf(stdout, "RUNTIME TOTAL, BUILD, SORT, PART (cycles): \n");
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
