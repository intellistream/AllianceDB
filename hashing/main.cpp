/**
 * @file    main.c
 * @author  Cagri Balkesen <cagri.balkesen@inf.ethz.ch>
 * @date    Wed May 16 16:03:10 2012
 * @version $Id: main.c 4546 2013-12-07 13:56:09Z bcagri $
 *
 * @brief  Main entry point for running join implementations with given command
 * line parameters.
 * 
 * (c) 2012, ETH Zurich, Systems Group
 * 
 * @mainpage Main-Memory Hash Joins On Multi-Core CPUs: Tuning to the Underlying Hardware
 *
 * @section intro Introduction
 *
 * This package provides implementations of the main-memory hash join algorithms
 * described and studied in our ICDE 2013 paper. Namely, the implemented
 * algorithms are the following with the abbreviated names:
 * 
 *  - NPO:    No Partitioning Join Optimized (Hardware-oblivious algo. in paper)
 *  - PRO:    Parallel Radix Join Optimized (Hardware-conscious algo. in paper)
 *  - PRH:    Parallel Radix Join Histogram-based
 *  - PRHO:   Parallel Radix Join Histogram-based Optimized
 *  - RJ:     Radix Join (single-threaded)
 *  - NPO_st: No Partitioning Join Optimized (single-threaded)
 *
 * @section compilation Compilation
 *
 * The package includes implementations of the algorithms and also the driver
 * code to run and repeat the experimental studies described in the paper.
 * 
 * The code has been written using standard GNU tools and uses autotools
 * for configuration. Thus, compilation should be as simple as:
 * 
 * @verbatim
       $ ./configure
       $ make
@endverbatim
 * 
 * Besides the usual ./configure options, compilation can be customized with the
 * following options:
 * @verbatim
   --enable-debug         enable debug messages on commandline  [default=no]
   --enable-key8B         use 8B keys and values making tuples 16B  [default=no]
   --enable-perfcounters  enable performance monitoring with Intel PCM  [no]
   --enable-paddedbucket  enable padding of buckets to cache line size in NPO [no]
   --enable-timing        enable execution timing  [default=yes]
   --enable-syncstats     enable synchronization timing stats  [default=no]
   --enable-skewhandling  enable fine-granular task decomposition based skew handling in radix [default=no]
@endverbatim
 * Additionally, the code can be configured to enable further optimizations
 * discussed in the Technical Report version of the paper:
 * @verbatim
   --enable-prefetch-npj   enable prefetching in no partitioning join [default=no]
   --enable-swwc-part      enable software write-combining optimization in
                           partitioning (Experimental, not tested extensively) ? [default=no]
@endverbatim
 * Our code makes use of the Intel Performance Counter Monitor tool which was
 * slightly modified to be integrated in to our implementation. The original
 * code can be downloaded from:
 * 
 * http://software.intel.com/en-us/articles/intel-performance-counter-monitor/
 * 
 * We are providing the copy that we used for our experimental study under
 * <b>`lib/intel-pcm-1.7`</b> directory which comes with its own Makefile. Its
 * build process is actually separate from the autotools build process but with
 * the <tt>--enable-perfcounters</tt> option, make command from the top level
 * directory also builds the shared library <b>`libperf.so'</b> that we link to
 * our code. After compiling with --enable-perfcounters, in order to run the
 * executable add `lib/intel-pcm-1.7/lib' to your
 * <tt>LD_LIBRARY_PATH</tt>. In addition, the code must be run with
 * root privileges to acces model specific registers, probably after
 * issuing the following command: `modprobe msr`. Also note that with
 * --enable-perfcounters the code is compiled with g++ since Intel
 * code is written in C++. 
 * 
 * We have successfully compiled and run our code on different Linux
 * variants; the experiments in the paper were performed on Debian and Ubuntu
 * Linux systems.
 * 
 * @section usage Usage and Invocation
 *
 * The <tt>mchashjoins</tt> binary understands the following command line
 * options: 
 * @verbatim
      Join algorithm selection, algorithms : RJ, PRO, PRH, PRHO, NPO, NPO_st
         -a --algo=<name>    Run the hash join algorithm named <name> [PRO]
 
      Other join configuration options, with default values in [] :
         -n --nthreads=<N>  Number of threads to use <N> [2]
         -r --r-size=<R>    Number of tuples in build relation R <R> [128000000]
         -s --s-size=<S>    Number of tuples in probe relation S <S> [128000000]
         -x --r-seed=<x>    Seed value for generating relation R <x> [12345]    
         -y --s-seed=<y>    Seed value for generating relation S <y> [54321]    
         -z --skew=<z>      Zipf* skew parameter for probe relation S <z> [0.0]
         --non-unique       Use non-unique (duplicated) keys in input relations 
         --full-range       Spread keys in relns. in full 32-bit integer range
         --basic-numa       Numa-localize relations to threads (Experimental)

      Performance profiling options, when compiled with --enable-perfcounters.
         -p --perfconf=<P>  Intel PCM config file with upto 4 counters [none]  
         -o --perfout=<O>   Output file to print performance counters [stdout]
 
      Basic user options
          -h --help         Show this message
          --verbose         Be more verbose -- show misc extra info 
          --version         Show version

@endverbatim
 * The above command line options can be used to instantiate a certain
 * configuration to run various joins and print out the resulting
 * statistics. Following the same methodology of the related work, our joins
 * never materialize their results as this would be a common cost for all
 * joins. We only count the number of matching tuples and report this. In order
 * to materialize results, one needs to copy results to a result buffer in the
 * corresponding locations of the source code.
 *
 * @section config Configuration Parameters
 *
 * @subsection cpumapping Logical to Pyhsical CPU Mapping
 *
 * If running on a machine with multiple CPU sockets and/or SMT feature enabled,
 * then it is necessary to identify correct mappings of CPUs on which threads
 * will execute. For instance one of our experiment machines, Intel Xeon L5520
 * had 2 sockets and each socket had 4 cores and 8 threads. In order to only
 * utilize the first socket, we had to use the following configuration for
 * mapping threads 1 to 8 to correct CPUs:
 * 
 * @verbatim
cpu-mapping.txt
8 0 1 2 3 8 9 10 11
@endverbatim
 * 
 * This file is must be created in the executable directory and used by default 
 * if exists in the directory. It basically says that we will use 8 CPUs listed 
 * and threads spawned 1 to 8 will map to the given list in order. For instance 
 * thread 5 will run CPU 8. This file must be changed according to the system at
 * hand. If it is absent, threads will be assigned round-robin. This CPU mapping
 * utility is also integrated into the Wisconsin implementation (found in 
 * `wisconsin-src') and same settings are also valid there.
 * 
 * @subsection perfmonitoring Performance Monitoring
 *
 * For performance monitoring a config file can be provided on the command line
 * with --perfconf which specifies which hardware counters to monitor. For 
 * detailed list of hardware counters consult to "Intel 64 and IA-32 
 * Architectures Software Developer’s Manual" Appendix A. For an example 
 * configuration file used in the experiments, see <b>`pcm.cfg'</b> file. 
 * Lastly, an output file name with --perfout on commandline can be specified to
 * print out profiling results, otherwise it defaults to stdout.
 *
 * @subsection systemparams System and Implementation Parameters
 *
 * The join implementations need to know about the system at hand to a certain
 * degree. For instance #CACHE_LINE_SIZE is required by both of the
 * implementations. In case of no partitioning join, other implementation
 * parameters such as bucket size or whether to pre-allocate for overflowing
 * buckets are parametrized and can be modified in `npj_params.h'.
 * 
 * On the other hand, radix joins are more sensitive to system parameters and 
 * the optimal setting of parameters should be found from machine to machine to 
 * get the same results as presented in the paper. System parameters needed are
 * #CACHE_LINE_SIZE, #L1_CACHE_SIZE and
 * #L1_ASSOCIATIVITY. Other implementation parameters specific to radix
 * join are also important such as #NUM_RADIX_BITS
 * which determines number of created partitions and #NUM_PASSES which
 * determines number of partitioning passes. Our implementations support between
 * 1 and 2 passes and they can be configured using these parameters to find the
 * ideal performance on a given machine.
 *
 * @section data Generating Data Sets of Our Experiments
 *
 * Here we briefly describe how to generate data sets used in our experiments 
 * with the command line parameters above.
 *
 * @subsection workloadB Workload B 
 * 
 * In this data set, the inner relation R and outer relation S have 128*10^6 
 * tuples each. The tuples are 8 bytes long, consisting of 4-byte (or 32-bit) 
 * integers and a 4-byte payload. As for the data distribution, if not 
 * explicitly specified, we use relations with randomly shuffled unique keys 
 * ranging from 1 to 128*10^6. To generate this data set, append the following 
 * parameters to the executable
 * <tt>mchashjoins</tt>:
 *
 * @verbatim
      $ ./mchashjoins [other options] --r-size=128000000 --s-size=128000000 
@endverbatim 
 *
 * \note Configure must have run without --enable-key8B.
 * 
 * @subsection workloadA Workload A
 * 
 * This data set reflects the case where the join is performed between the 
 * primary key of the inner relation R and the foreign key of the outer relation
 * S. The size of R is fixed at 16*2^20 and size of S is fixed at 256*2^20. 
 * The ratio of the inner relation to the outer relation is 1:16. In this data 
 * set, tuples are represented as (key, payload) pairs of 8 bytes each, summing 
 * up to 16 bytes per tuple. To generate this data set do the following:
 * 
 * @verbatim
     $ ./configure --enable-key8B
     $ make
     $ ./mchashjoins [other options] --r-size=16777216 --s-size=268435456 
@endverbatim 
 * 
 * @subsection skew Introducing Skew in Data Sets
 * 
 * Skew can be introduced to the relation S as in our experiments by appending 
 * the following parameter to the command line, which is basically a Zipf 
 * distribution skewness parameter:
 * 
 * @verbatim
     $ ./mchashjoins [other options] --skew=1.05
@endverbatim
 *
 * @section wisconsin Wisconsin Implementation
 *
 * A slightly modified version of the original implementation provided by 
 * Blanas et al. from University of Wisconsin is provided under `wisconsin-src'
 * directory. The changes we made are documented in the header of the README 
 * file. These implementations provide the algorithms mentioned as 
 * `non-optimized no partitioning join' and `non-optimized radix join' in our 
 * paper. The original source code can be downloaded from 
 * http://pages.cs.wisc.edu/∼sblanas/files/multijoin.tar.bz2 .
 *
 *
 *
 * @author Cagri Balkesen <cagri.balkesen@inf.ethz.ch>
 *
 * (c) 2012, ETH Zurich, Systems Group
 *
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <zconf.h>
#include <algorithm>
#include "benchmark.h"
#include "timer/clock.h"

void
print_version();

void
parse_args(int argc, char **argv, param_t *cmd_params);

param_t defaultParam();


int
main(int argc, char **argv) {

    /* start initially on CPU-0 */
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(0, &set);
    if (sched_setaffinity(0, sizeof(set), &set) < 0) {
        perror("sched_setaffinity");
    }

    param_t cmd_params = defaultParam();
    parse_args(argc, argv, &cmd_params);

//#ifdef PROFILE_MEMORY_CONSUMPTION
//    auto curtime = std::chrono::steady_clock::now();
//    // dump the pid outside, and attach vtune for performance measurement
//    string path = EXP_DIR "/time_start_" + std::to_string(cmd_params.exp_id) + ".txt";
//    auto fp = fopen(path.c_str(), "w");
//    setbuf(fp,NULL);
//    fprintf(fp, "%ld\n", curtime);
//    fflush(fp);
//#endif

    //reset relation size according to our settings.
//    cmd_params.r_size = cmd_params.window_size / cmd_params.interval * cmd_params.step_sizeR;
//    cmd_params.s_size = cmd_params.window_size / cmd_params.interval * cmd_params.step_sizeS;

    if (check_avx() == 0) {
        /* no AVX support, just use scalar variants. */
        fprintf(stdout, "[WARN ] AVX is not supported, using scalar sort & merge.\n");
        cmd_params.scalar_sort = 1;
        cmd_params.scalar_merge = 1;
    }
#ifdef PERF_COUNTERS
    PCM_CONFIG = cmd_params.perfconf;
    PCM_OUT = cmd_params.perfout;
#endif

    benchmark(cmd_params);
    return 0;
}

//*  - NPO:    No Partitioning Join Optimized (Hardware-oblivious algo. in paper)
//*  - PRO:    Parallel Radix Join Optimized (Hardware-conscious algo. in paper)
//*  - PRH:    Parallel Radix Join Histogram-based
//*  - PRHO:   Parallel Radix Join Histogram-based Optimized
//*  - RJ:     Radix Join (single-threaded)
//*  - NPO_st: No Partitioning Join Optimized (single-threaded)

/** all available algorithms */
static struct algo_t algos[] =
        {
/*** Blocking Joins ***/
                {"PRO",         PRO}, // The best performing blocking hash join (radix partition optimization).
                {"RJ_st",       RJ_st}, // Radix Join Single_thread
                {"PRH",         PRH},
                {"PRHO",        PRHO},
                {"NPO",         NPO},
                {"NPO_st",      NPO_st}, /* NPO single threaded */
/*** Symmetric Hash Join ***/
                {"SHJ_st",      SHJ_st}, /* Symmetric hash join single_thread*/
                {"SHJ_JM_P",    SHJ_JM_P}, /* Symmetric hash join JM Model, Partition*/
                {"SHJ_JM_NP",   SHJ_JM_NP}, /* Symmetric hash join JM Model, No-Partition*/
                {"SHJ_JB_NP",   SHJ_JB_NP}, /* Symmetric hash join JB Model, No-Partition*/
                {"SHJ_JBCR_NP", SHJ_JBCR_NP}, /* Symmetric hash join JB CountRound Model, No-Partition*/
                {"SHJ_JBCR_P",  SHJ_JBCR_P}, /* Symmetric hash join JB CountRound Model, No-Partition*/
                {"SHJ_HS_NP",   SHJ_HS_NP}, /* Symmetric hash join HS Model, No-Partition*/
/*** Progressive Merge Join ***/
                {"PMJ_st",      PMJ_st}, /* Progressive Merge Join Single_thread*/
                {"PMJ_JM_NP",   PMJ_JM_NP}, /* Progressive Merge Join JM_NP*/
                {"PMJ_JM_P",    PMJ_JM_P}, /* Progressive Merge Join JM_P*/
                {"PMJ_JB_NP",   PMJ_JB_NP}, /* Progressive Merge Join JB_NP*/
                {"PMJ_JBCR_NP", PMJ_JBCR_NP}, /* Progressive Merge Join JBCR_NP*/
                {"PMJ_JBCR_P",  PMJ_JBCR_P}, /* Progressive Merge Join JBCR_P*/
                {"PMJ_HS_NP",   PMJ_HS_NP}, /* Progressive Merge Join HS_NP*/
/*** Ripple Join (unfinished) ***/
                {"RPJ_st",      RPJ_st}, /* Ripple Join Single_thread*/
                {"RPJ_JM_NP",   RPJ_JM_NP}, /* Ripple Join JM*/
                {"RPJ_JB_NP",   RPJ_JB_NP}, /* Ripple Join JB*/
                {"RPJ_JBCR_NP", RPJ_JBCR_NP}, /* Ripple Join JB*/
                {"RPJ_HS_NP",   RPJ_HS_NP}, /* Ripple Join HS*/
                {{0},           0}
        };

param_t defaultParam() {/* Command line parameters */
    param_t cmd_params;

    /* Default values if not specified on command line */
    /* BLOCKING HASHING: PRO (0), RJ_st, PRH, PRHO, NPO (4), NPO_st (5),
     * ONLINE HAHSING: SHJ_st(6), SHJ_JM_NP, SHJ_JB_NP, SHJ_JBCR_NP, SHJ_HS_NP (10)
     * ONLINE SORTING: PMJ_st(11), PMJ_JM_NP, PMJ_JB_NP, PMJ_JBCR_NP, PMJ_HS_NP (15)
     * RIPPLE JOIN: RPJ_st(16), RPJ_JM_NP,  RPJ_JB_NP, RPJ_JBCR_NP, RPJ_HS_NP
     * */
    cmd_params.algo = &algos[4];
    cmd_params.nthreads = 1;//TODO: in HS mode, thread must be larger than 1. Fix it when nthread=1.
    cmd_params.r_seed = 12345;
    cmd_params.s_seed = 54321;
    cmd_params.skew = 0.0;
    cmd_params.verbose = 0;
    cmd_params.perfconf = NULL;
    cmd_params.perfout = NULL;
    cmd_params.nonunique_keys = 0;
    cmd_params.fullrange_keys = 0;
    cmd_params.basic_numa = 0;
    cmd_params.scalar_sort = 0;
    cmd_params.scalar_merge = 0;
    /* TODO: get L3 buffer size programmatically. */
    cmd_params.mwaymerge_bufsize = MWAY_MERGE_BUFFER_SIZE_DEFAULT;

    cmd_params.loadfileR = NULL;
    cmd_params.loadfileS = NULL;
    cmd_params.rkey = 0;
    cmd_params.skey = 0;

    cmd_params.old_param = 0;
    cmd_params.window_size = 10000;
    cmd_params.step_sizeR = 40;
    cmd_params.step_sizeS = -1;
    cmd_params.interval = 1000;
    cmd_params.ts = 1;
    cmd_params.key_distribution = 0;
    cmd_params.ts_distribution = 0;
    cmd_params.zipf_param = 0.0;
    cmd_params.exp_id = 0;
    cmd_params.gap = 10;
    cmd_params.group_size = 2;
    cmd_params.fixS = 0;

    // SAMPLE
    cmd_params.epsilon_r = 0.1;
    cmd_params.epsilon_s = 0.1;
    cmd_params.data_utilization_r = 0;
    cmd_params.data_utilization_s = 0;
    cmd_params.Universal_p = 0.003;
    cmd_params.Bernoulli_q = 0.003;
    cmd_params.reservior_size = 1000;
    cmd_params.rand_buffer_size = 1000;
    cmd_params.presample_size = 1000;
    cmd_params.grp_id = NULL;

    /* default dataset is Workload B (described in paper) */
    cmd_params.r_size = 500000;
    cmd_params.s_size = 500000;
    assert(cmd_params.r_size <= cmd_params.s_size);

    return cmd_params;
}

/* command line handling functions */
void
print_help(char *progname) {
    MSG("Usage: %s [options]\n", progname);

    MSG(" \
    Join algorithm selection, algorithms : RJ_st, PRO, PRH, PRHO, NPO, NPO_st    \n\
       -a --algo=<name>    Run the hash join algorithm named <name> [PRO]     \n\
                                                                              \n\
    Other join configuration options, with default values in [] :             \n\
       -n --nthreads=<N>  Number of threads to use <N> [2]                    \n\
       -r --r-size=<R>    Number of tuples in build relation R <R> [128000000]\n\
       -s --s-size=<S>    Number of tuples in probe relation S <S> [128000000]\n\
       -x --r-seed=<x>    Seed value for generating relation R <x> [12345]    \n\
       -y --s-seed=<y>    Seed value for generating relation S <y> [54321]    \n\
       -z --skew=<z>      Zipf skew parameter for probe relation S <z> [0.0]  \n\
       -R --r-file=<Rf>   The file to load build relation R from <Rf> [R.tbl] \n\
       -S --s-file=<Sf>   The file to load probe relation S from <Sf> [S.tbl] \n\
       --non-unique       Use non-unique (duplicated) keys in input relations \n\
       --full-range       Spread keys in relns. in full 32-bit integer range  \n\
       --basic-numa       Numa-localize relations to threads (Experimental)   \n\
                                                                              \n\
    Performance profiling options, when compiled with --enable-perfcounters.  \n\
       -p --perfconf=<P>  Intel PCM config file with upto 4 counters [none]   \n\
       -o --perfout=<O>   Output file to print performance counters [stdout]  \n\
                                                                              \n\
    Basic user options                                                        \n\
        -h --help         Show this message                                   \n\
        --verbose         Be more verbose -- show misc extra info             \n\
        --version         Show version                                        \n\
    \n");
}

void
print_version() {
    // MSG("\n%s\n", PACKAGE_STRING);
    // MSG("Copyright (c) 2012, 2013, ETH Zurich, Systems Group.\n");
    // MSG("http://www.systems.ethz.ch/projects/paralleljoins\n\n");
    // MSG("Modified 2019, Shuhao Zhang (Tony) and Yancan Mao, TU Berlin, NUS. \n");
}

static char *
mystrdup(const char *s) {
    char *ss = (char *) malloc(strlen(s) + 1);

    if (ss != NULL)
        memcpy(ss, s, strlen(s) + 1);

    return ss;
}

void
parse_args(int argc, char **argv, param_t *cmd_params) {

    int c, i, found;
    /* Flag set by ‘--verbose’. */
    static int verbose_flag;
    static int nonunique_flag;
    static int fullrange_flag;
    static int basic_numa;
    static int scalarsort = 0;
    static int scalarmerge = 0;
    static int part_fanout = PARTFANOUT_DEFAULT; /* 128 */

    while (1) {
        static struct option long_options[] =
                {
                        /* These options set a flag. */
                        {"verbose",          no_argument,       &verbose_flag,   1},
                        {"brief",            no_argument,       &verbose_flag,   0},
                        {"non-unique",       no_argument,       &nonunique_flag, 1},
                        {"full-range",       no_argument,       &fullrange_flag, 1},
                        {"basic-numa",       no_argument,       &basic_numa,     1},
                        {"help",             no_argument,       0,               'h'},
                        {"version",          no_argument,       0,               'v'},
                        /* These options don't set a flag.
                           We distinguish them by their indices. */
                        {"algo",             required_argument, 0,               'a'},
                        {"nthreads",         required_argument, 0,               'n'},
                        {"perfconf",         required_argument, 0,               'p'},
                        {"r-size",           required_argument, 0,               'r'},
                        {"s-size",           required_argument, 0,               's'},
                        {"perfout",          required_argument, 0,               'o'},
                        {"r-seed",           required_argument, 0,               'x'},
                        {"s-seed",           required_argument, 0,               'y'},
                        {"skew",             required_argument, 0,               'z'},
                        {"r-file",           required_argument, 0,               'R'},
                        {"s-file",           required_argument, 0,               'S'},
                        {"r-key",            required_argument, 0,               'J'},
                        {"s-key",            required_argument, 0,               'K'},
                        {"r-ts",             required_argument, 0,               'L'},
                        {"s-ts",             required_argument, 0,               'M'},
                        /* partitioning fanout, e.g., 2^rdxbits */
                        {"partfanout",       required_argument, 0,               'f'},
                        {"numastrategy",     required_argument, 0,               'N'},
                        {"mwaybufsize",      required_argument, 0,               'm'},

                        {"gen_with_ts",      required_argument, 0,               't'},
                        {"real_data",        required_argument, 0,               'B'},
                        {"window-size",      required_argument, 0,               'w'},
                        {"step-size",        required_argument, 0,               'e'},
                        {"interval",         required_argument, 0,               'l'},
                        {"key_distribution", required_argument, 0,               'd'},
                        {"zipf_param",       required_argument, 0,               'Z'},
                        {"exp_id",           required_argument, 0,               'I'},
                        {"ts_distribution",  required_argument, 0,               'D'},
                        {"duplicate_num",    required_argument, 0,               'P'}, // represent num of Partitions
                        {"epsilon_r",        required_argument, 0,               'E'},
                        {"epsilon_s",        required_argument, 0,               'F'},
                        {"Universal_p",      required_argument, 0,               'U'},
                        {"Bernoulli_q",      required_argument, 0,               'Q'},
                        {"reservior_size",   required_argument, 0,               'O'},
                        {"rand_buffer_size", required_argument, 0,               'b'},
                        {"presample_size",   required_argument, 0,               'A'},
                        {"group_id",         required_argument, 0,               'H'},
                        {"data_utilization_r",   required_argument, 0,               'X'},
                        {"data_utilization_s",         required_argument, 0,               'Y'},
                        {0, 0,                                  0,               0}

                };
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, "J:K:L:M:t:w:e:q:l:I:d:Z:D:g:G:B:W:a:n:p:r:s:o:x:y:z:R:S:H:hv[:]:P:U:Q:E:F:X:Y:O:b:A",
                        long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;
        switch (c) {
            case 0:
                /* If this option set a flag, do nothing else now. */
                if (long_options[option_index].flag != 0)
                    break;
                MSG("option %s", long_options[option_index].name);
                if (optarg) MSG(" with arg %s", optarg);
                MSG("\n");
                break;

            case 'a':
                i = 0;
                found = 0;
                while (algos[i].joinAlgo) {
                    if (strcmp(optarg, algos[i].name) == 0) {
                        cmd_params->algo = &algos[i];
                        found = 1;
                        break;
                    }
                    i++;
                }

                if (found == 0) {
                    MSG("[ERROR] Join algorithm named `%s' does not exist!\n",
                        optarg);
                    print_help(argv[0]);
                    exit(EXIT_FAILURE);
                }
                break;

            case 'h':
            case '?':
                /* getopt_long already printed an error message. */
                print_help(argv[0]);
                exit(EXIT_SUCCESS);
                break;

            case 'v':
                print_version();
                exit(EXIT_SUCCESS);
                break;

            case 'n':
                cmd_params->nthreads = atoi(optarg);
                break;

            case 'p':
                cmd_params->perfconf = mystrdup(optarg);
                break;

            case 'r':
                cmd_params->r_size = atol(optarg);
                break;

            case 's':
                cmd_params->s_size = atol(optarg);
                break;

            case 'o':
                cmd_params->perfout = mystrdup(optarg);
                break;

            case 'x':
                cmd_params->r_seed = atoi(optarg);
                break;

            case 'y':
                cmd_params->s_seed = atoi(optarg);
                break;

            case 'z':
                cmd_params->skew = atof(optarg);
                break;

            case 'f':
                part_fanout = atoi(optarg);
                /* check whether fanout is a power of 2 */
                if ((part_fanout & (part_fanout - 1)) != 0) {
                    fprintf(stdout, "[ERROR] Partitioning fan-out must be a power of 2 (2^#radixbits).\n");

                    exit(0);
                }
                break;
            case 'm':
                cmd_params->mwaymerge_bufsize = atoi(optarg);
                break;

            case 'N':
                if (strcmp(optarg, "NEXT") == 0)
                    cmd_params->numastrategy = NEXT;
                else if (strcmp(optarg, "RANDOM") == 0)
                    cmd_params->numastrategy = RANDOM;
                else if (strcmp(optarg, "RING") == 0)
                    cmd_params->numastrategy = RING;
                else {
                    MSG("Invalid NUMA-shuffle strategy. Options: NEXT, RANDOM, RING\n");
                    MSG("Using RING as default.\n");
                }
                break;

            case 'R':
                cmd_params->loadfileR = mystrdup(optarg);
                break;

            case 'S':
                cmd_params->loadfileS = mystrdup(optarg);
                break;
            case 'J':
                cmd_params->rkey = atoi(mystrdup(optarg));
                break;
            case 'K':
                cmd_params->skey = atoi(mystrdup(optarg));
                break;
            case 'L':
                cmd_params->rts = atoi(mystrdup(optarg));
                break;
            case 'M':
                cmd_params->sts = atoi(mystrdup(optarg));
                break;
            case 't':
                cmd_params->ts = atoi(mystrdup(optarg));
                break;
            case 'w':
                cmd_params->window_size = atoi(mystrdup(optarg));
                break;
            case 'e':
                cmd_params->step_sizeR = atoi(mystrdup(optarg));
                break;
            case 'q':
                cmd_params->step_sizeS = atoi(mystrdup(optarg));
                break;
            case 'l':
                cmd_params->interval = atoi(mystrdup(optarg));
                break;
            case 'd':
                cmd_params->key_distribution = atoi(mystrdup(optarg));
                break;
            case 'D':
                cmd_params->ts_distribution = atoi(mystrdup(optarg));
                break;
            case 'Z':
                cmd_params->zipf_param = atof(optarg);
                break;
            case 'I':
                cmd_params->exp_id = atoi(mystrdup(optarg));
                break;
            case 'G':
                cmd_params->group_size = atoi(mystrdup(optarg));
                break;
            case 'g':
                cmd_params->gap = atoi(mystrdup(optarg));
                break;
            case 'B':
                cmd_params->old_param = atoi(mystrdup(optarg));
                break;
            case 'W':
                cmd_params->fixS = atoi(mystrdup(optarg));
                break;
            case '[':
                cmd_params->progressive_step = atoi(mystrdup(optarg));
                break;
            case ']':
                cmd_params->merge_step = atoi(mystrdup(optarg));
                break;
            case 'P':
                cmd_params->duplicate_num = atoi(mystrdup(optarg));
                break;
            case 'E':
                cmd_params->epsilon_r = atof(mystrdup(optarg));
                break;
            case 'F':
                cmd_params->epsilon_s = atof(mystrdup(optarg));
                break;
            case 'X':
                cmd_params->data_utilization_r = atof(mystrdup(optarg));
                break;
            case 'Y':
                cmd_params->data_utilization_s = atof(mystrdup(optarg));
                break;
            case 'U':
                cmd_params->Universal_p = atof(mystrdup(optarg));
                break;
            case 'Q':
                cmd_params->Bernoulli_q = atof(mystrdup(optarg));
                break;
            case 'O':
                cmd_params->reservior_size = atoi(mystrdup(optarg));
                break;
            case 'b':
                cmd_params->rand_buffer_size = atoi(mystrdup(optarg));
                break;
            case 'A':
                cmd_params->presample_size = atoi(mystrdup(optarg));
                break;
            case 'H':
                cmd_params->grp_id = mystrdup(optarg);
                break;
            default:
                break;
        }
    }

    /* if (verbose_flag) */
    /*     printf ("verbose flag is set \n"); */

    if (cmd_params->step_sizeS == -1) {
        cmd_params->step_sizeS = cmd_params->step_sizeR;
    }
    cmd_params->nonunique_keys = nonunique_flag;
    cmd_params->verbose = verbose_flag;
    cmd_params->fullrange_keys = fullrange_flag;
    cmd_params->basic_numa = basic_numa;

    if (cmd_params->nthreads == 1) {
        cmd_params->group_size = 1;
        MSG("[INFO] reset group size to one\n");
    }

    /* Print any remaining command line arguments (not options). */
    if (optind < argc) {
        MSG("non-option arguments: ");
        while (optind < argc) MSG("%s ", argv[optind++]);
        MSG("\nc");
    }
}
