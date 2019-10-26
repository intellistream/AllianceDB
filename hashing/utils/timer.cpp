//
// Created by Shuhao Zhang on 17/10/19.
//

#include "timer.h"

/** print out the execution time statistics of the join */
void print_timing(uint64_t total, uint64_t build, uint64_t part, uint64_t numtuples, int64_t result, timeval *start,
                  timeval *end, uint64_t progressivetimer[]) {
    double diff_usec = (((*end).tv_sec * 1000000L + (*end).tv_usec)
                        - ((*start).tv_sec * 1000000L + (*start).tv_usec));
    double cyclestuple = total;
    cyclestuple /= numtuples;
    fprintf(stdout, "RUNTIME TOTAL, BUILD, PART (cycles): \n");
    fprintf(stdout, "%llu \t %llu (%.2f%%)  \t %llu ",
            total, build, (build * 100 / (double) total), part);
    fprintf(stdout, "\n");
    fprintf(stdout, "TOTAL-TIME-USECS, TOTAL-TUPLES, CYCLES-PER-TUPLE: \n");
    fprintf(stdout, "%.4lf \t %ld \t %.4lf", diff_usec, result, cyclestuple);
    fprintf(stdout, "\n");
    fprintf(stdout, "Time to obtain 25%%, 50%%, 75%% of results: \n");
    fprintf(stdout, "(%.2f%%) \t (%.2f%%) \t (%.2f%%)",
            (100 * progressivetimer[0] / (double) total),
            (100 * progressivetimer[1] / (double) total),
            (100 * progressivetimer[2] / (double) total));
    fprintf(stdout, "\n");
    fflush(stdout);

}