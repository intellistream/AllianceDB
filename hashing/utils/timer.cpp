//
// Created by Shuhao Zhang on 17/10/19.
//

#include "timer.h"

/** print out the execution time statistics of the join */
void print_timing(uint64_t total, uint64_t build, uint64_t part,
             uint64_t numtuples, int64_t result,
             struct timeval *start, struct timeval *end) {
    double diff_usec = (((*end).tv_sec * 1000000L + (*end).tv_usec)
                        - ((*start).tv_sec * 1000000L + (*start).tv_usec));
    double cyclestuple = total;
    cyclestuple /= numtuples;
    fprintf(stdout, "RUNTIME TOTAL, BUILD, PART (cycles): \n");
    fprintf(stderr, "%llu \t %llu \t %llu ",
            total, build, part);
    fprintf(stdout, "\n");
    fprintf(stdout, "TOTAL-TIME-USECS, TOTAL-TUPLES, CYCLES-PER-TUPLE: \n");
    fprintf(stdout, "%.4lf \t %llu \t ", diff_usec, result);
    fflush(stdout);
    fprintf(stderr, "%.4lf ", cyclestuple);
    fflush(stderr);
    fprintf(stdout, "\n");

}