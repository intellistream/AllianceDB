//
// Created by root on 11/29/19.
//

#ifndef ALLIANCEDB_BENCHMARK_H
#define ALLIANCEDB_BENCHMARK_H

#include <stdio.h>              /* printf */
#include <sys/time.h>           /* gettimeofday */
#include <getopt.h>             /* getopt */
#include <stdlib.h>             /* exit */
#include <string.h>             /* strcmp */
#include <limits.h>             /* INT_MAX */
#include <sched.h>
#include <cpuid.h>
#include <assert.h>
#include "joins/common_functions.h"

#include "joins/no_partitioning_join.h" /* no partitioning joins: NPO, NPO_st */
#include "joins/parallel_radix_join.h"  /* parallel radix joins: RJ_st, PRO, PRH, PRHO */
#include "joins/onlinejoins.h"  /* single_thread onlinejoins: SHJ_st*/
#include "utils/generator.h"            /* create_relation_xk */
#include "utils/perf_counters.h" /* PCM_x */
//#include "utils/affinity.h"      /* pthread_attr_setaffinity_np & sched_setaffinity */ only for MAC
#include "utils/config.h"     /* autoconf header */
#include "utils/params.h"

#include <chrono>

using namespace std::chrono;

#ifdef JOIN_RESULT_MATERIALIZE

#include "utils/tuple_buffer.h"       /* for materialization */

#endif

#if !defined(__cplusplus)
int getopt(int argc, char * const argv[],
           const char *optstring);
#endif

#ifndef INT_MAX
#define INT_MAX 2147483647
#endif

#define AVXFlag     ((1UL<<28)|(1UL<<27))


typedef struct algo_t algo_t;
typedef struct param_t param_t;

struct algo_t {
    char name[128];

    result_t *(*joinAlgo)(relation_t *, relation_t *, const param_t cmd_params);
};

struct param_t {
    algo_t *algo;
    uint32_t nthreads;
    uint64_t r_size;
    uint64_t s_size;
    uint32_t r_seed;
    uint32_t s_seed;

    int nonunique_keys;  /* non-unique keys allowed? */
    int verbose;
    int fullrange_keys;  /* keys covers full int range? */
    int basic_numa;/* alloc input chunks thread local? */
    char *perfconf;
    char *perfout;

    int scalar_sort;
    int scalar_merge;
    /* partitioning fanout, e.g., 2^rdxbits */
    int part_fanout;
    /* multi-way merge buffer size in bytes, corresponds to L3 size */
    int mwaymerge_bufsize;
    /* NUMA data shuffling strategy */
    enum numa_strategy_t numastrategy;

    /** if the relations are load from file */
    char *loadfileR;
    char *loadfileS;

    int32_t rkey;
    int32_t skey;

    int32_t rts;
    int32_t sts;

    int old_param; /* whether to use old parameters */
    int fixS;
    int ts;
    int key_distribution;
    int ts_distribution;
    double skew;
    double zipf_param;
    int duplicate_num = 1;

    int window_size;
    int step_sizeR;
    int step_sizeS;
    int interval;

    int group_size;
    int exp_id;
    int gap;
    int progressive_step = 1;//percentile of tuples to sort at each iteration.
    int merge_step = 2;

    double epsilon_r;
    double epsilon_s;
    double data_utilization_r;
    double data_utilization_s;
    double Universal_p;
    double Bernoulli_q;
    int reservior_size;
    int rand_buffer_size;
    int presample_size;

    char *grp_id;
};


extern char *optarg;
extern int optind, opterr, optopt;

/** An experimental feature to allocate input relations numa-local */
extern int numalocalize;  /* defined in generator.c */
extern int nthreads;      /* defined in generator.c */

extern double *hash_p_list;
extern long long *thread_res_list;

//void benchmark(const param_t cmd_params, relation_t *relR, relation_t *relS,
//        relation_payload_t *relPlR, relation_payload_t *relPlS, result_t *results);

/***HELPER function***/
int check_avx();

/***Queries***/
void benchmark(const param_t cmd_params);

#endif //ALLIANCEDB_BENCHMARK_H
