//
// Created by Shuhao Zhang on 17/10/19.
//

#include "t_timer.h"
#include <iostream>
#include <fstream>

using namespace std;

void print_timing(std::vector<uint64_t> vector) {
    std::string homepath = getenv("HOME");
    ofstream outputFile(homepath + "ClionProjects/AllianceDB/hashing/progressive_results.txt");
    for (auto &element : vector) {
        outputFile << element << endl;
    }
    outputFile.close();
}

void print_timing(uint64_t numtuples, int64_t result, T_TIMER *timer) {
    double diff_usec = (((timer->end).tv_sec * 1000000L + (timer->end).tv_usec)
                        - ((timer->start).tv_sec * 1000000L + (timer->start).tv_usec));
    double cyclestuple = timer->overall_timer / numtuples;
    fprintf(stdout, "RUNTIME TOTAL, BUILD, PART (cycles): \n");
    fprintf(stdout, "%llu \t %llu (%.2f%%)  \t %llu (%.2f%%) ",
            timer->overall_timer, timer->buildtimer, (timer->buildtimer * 100 / (double) timer->overall_timer),
            timer->partition_timer, (timer->partition_timer * 100 / (double) timer->overall_timer));
    fprintf(stdout, "\n");
    fprintf(stdout, "TOTAL-TIME-USECS, TOTAL-TUPLES, CYCLES-PER-TUPLE: \n");
    fprintf(stdout, "%.4lf \t %ld \t %.4lf", diff_usec, result, cyclestuple);
    fprintf(stdout, "\n");
    fprintf(stdout, "\n");
    fflush(stdout);
}
