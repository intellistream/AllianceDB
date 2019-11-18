/**
 * @file    main.c
 * @author  Cagri Balkesen <cagri.balkesen@inf.ethz.ch>
 * @date    Fri Dec 14 10:39:14 2012
 * @version $Id$
 *
 * @brief  Main entry point for running sort-merge join implementations with
 * given command line parameters.
 *
 * (c) 2012-2014, ETH Zurich, Systems Group
 *
 * @mainpage Multi-Core, Main-Memory Joins: Sort vs. Hash Revisited
 *
 * @section intro Introduction
 *
 * This package provides implementations of the main-memory sort-merge join
 * algorithms described and studied in our VLDB 2013 paper (presentation in
 * VLDB 2014). In addition to the newly introduced sort-merge join algorithms
 * \b m-way and \b m-pass, the code package also provides implementations
 * for various algorithms such as single-threaded AVX sorting routine and
 * multi-way merge as well as various micro-benchmarks. The essential
 * sort-merge algorithms presented in the paper are the following:
 *
 *  - \b m-pass:    Sort-merge join with multi-pass naive merging
 *  - \b m-way:     Sort-merge join with multi-way merging
 *
 * @note For the implementation of the hash join algorithms mentioned in the
 * paper, please obtain the hash joins code package from our webpage.
 *
 * @note The implementations are targeted for the first version of the AVX instruction
 * set. The AVX initially did not provide instructions for integers. Therefore, our
 * code treated 2x32-bit integers as 64-bit doubles and operated on them with the
 * corresponding AVX instructions: <sign:1bit-exponent:11bits-fraction:52bits> = <key:32-bits-payload:32-bits>.
 * While this provided correct results for our study, it might lead to errors in generic
 * cases such as when NaN patterns are found in real data. For our case, this can only occur
 * when the key contains all 1s for bits 20:30 and when either of key[0:19] and payload[0:31]
 * has a non-zero bit. Our generators carefully eliminate this from the inputs. In the latest
 * version of AVX (i.e., AVX2), this problem will be completely eliminated.
 *
 * @warning The code has been tested and experimentally evaluated on the \b Intel
 * \b E5-4640, a four socket Sandy Bridge machine with 2.4 Ghz clock, 512 GiB
 * memory and AVX support. The OS is Debian Linux 7.0, kernel version 3.4.4-U5
 * compiled with transparent huge page support. The OS is configured with 2 MiB
 * VM pages for memory allocations transparently. For reproducing the results
 * from the paper all of these parameters should ideally be matched. Moreover, the
 * NUMA-topology of the underlying machine also plays and important role. If the
 * machine has a different NUMA-topology than the E5-4640, then some changes
 * in the code and/or in cpu-mappings.txt might be required. Moreover, memory usage
 * was not the primary concern of our paper and therefore algorithms are usually
 * implemented in an out-of-place manner which requires extensive memory usage.
 * Lack of sufficient memory (3-4X more than input relations) might result in unexpected
 * performance behavior. Please contact us if you need more information regarding
 * how to best configure the code for a given machine.
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
 * @note For the baseline comparisons, C++ STL sort algorithm is used only if
 * the code is compiled with a C++ compiler. In order to enable this, configure
 * with <b>`./configure CC=g++`</b>.
 *
 * Besides the usual ./configure options, compilation can be customized with the
 * following options:
 * @verbatim
       --enable-key8B  use 8B keys and values making tuples 16B [default=no]
       --enable-perfcounters  enable performance counter monitoring with Intel PCM  [default=no]
       --enable-debug  enable debug messages on commandline  [default=no]
       --enable-timing  enable execution timing  [default=yes]
       --enable-materialize  Materialize join results ?  [default=no]
       --enable-persist  Persist input tables and join result table to files (R.tbl S.tbl Out.tbl) ?  [default=no]

@endverbatim
 * @note If the code is compiled with \b --enable-key8B, then tuples become 16-bytes
 * and the AVX implementation becomes dysfunctional. In this configuration, algorithms
 * must run with \b --scalarsort \b --scalarmerge options on the command line.
 *
 * Our code makes use of the Intel Performance Counter Monitor tool (version 2.35)
 * which was slightly modified to be integrated into our implementation. The original
 * code can be obtained from:
 *
 * http://software.intel.com/en-us/articles/intel-performance-counter-monitor/
 *
 * We are providing the copy that we used for our experimental study under
 * <b>`lib/intel-pcm-2.35`</b> directory which comes with its own Makefile. Its
 * build process is actually separate from the autotools build process but with
 * the <tt>--enable-perfcounters</tt> option, make command from the top level
 * directory also builds the shared library <b>`libperf.so'</b> that we link to
 * our code. After compiling with --enable-perfcounters, in order to run the
 * executable add `lib/intel-pcm-2.35/lib' to your
 * <tt>LD_LIBRARY_PATH</tt>. In addition, the code must be run with
 * root privileges to acces model specific registers, probably after
 * issuing the following command: `modprobe msr`. Also note that with
 * --enable-perfcounters the code is compiled with g++ since Intel
 * code is written in C++.
 *
 * We have successfully compiled and run our code on different Linux
 * variants as well as Mac OS X; the experiments in the paper were performed on
 * Debian and Ubuntu Linux systems.
 *
 * @section usage Usage and Invocation
 *
 * The <tt>sortmergejoin</tt> binary understands the following command line
 * options:
 * @verbatim
$ src/sortmergejoins -h
[INFO ] Initializing logical <-> physical thread/CPU mappings...
[INFO ] Getting the NUMA configuration automatically with libNUMA.
Usage: src/sortmergejoins [options]
Join algorithm selection, algorithms : m-way m-pass
       -a --algo=<name>    Run the join algorithm named <name> [m-pass]

    Other join configuration options, with default values in [] :
       -n --nthreads=<N>   Number of threads to use <N> [2]
       -r --r-size=<R>     Number of tuples in build relation R <R> [128000000]
       -s --s-size=<S>     Number of tuples in probe relation S <S> [128000000]
       -x --r-seed=<x>     Seed value for generating relation R <x> [12345]
       -y --s-seed=<y>     Seed value for generating relation S <y> [54321]
       -z --skew=<z>       Zipf skew parameter for probe relation S <z> [0.0]
       --non-unique        Use non-unique (duplicated) keys in input relations
       --full-range        Spread keys in relns. in full 32-bit integer range
       --no-numa-localize  Do not NUMA-localize relations to threads [yes]
       --scalarsort        Use scalar sorting; sort() -> g++ or qsort() -> gcc
       --scalarmerge       Use scalar merging algorithm instead of AVX-merge
       -f --partfanout=<F> Partitioning fan-out (phase-1, 2^#rdxbits) <F> [128]
       -S --numastrategy   NUMA data shuffle strategy: NEXT,RANDOM,RING [NEXT]
       -m --mwaybufsize    Multi-way merge buffer size in bytes,ideally L3-size

    Performance profiling options, when compiled with --enable-perfcounters.
       -p --perfconf=<P>   Intel PCM config file with upto 4 counters [none]
       -o --perfout=<O>    Output file to print performance counters [stdout]

    Basic user options
        -h --help          Show this message
        --verbose          Be more verbose -- show misc extra info
        --version          Show version

@endverbatim
 * The above command line options can be used to instantiate a certain
 * configuration to run the given join algorithm and print out the resulting
 * statistics. Following the same methodology of the related work, our joins
 * never materialize their results as this would be a common cost for all
 * joins. We only count the number of matching tuples and report this. In order
 * to materialize results, one needs to configure the code with --enable-materialize
 * option. Moreover, to write join input and output tables to file, one needs
 * to configure the code with --enable-persist.
 *
 * @section config Configuration Parameters
 *
 * @subsection cpumapping Logical to Pyhsical CPU Mapping
 *
 * If running on a machine with multiple CPU sockets and/or SMT feature enabled,
 * then it is necessary to identify correct mappings of CPUs on which threads
 * will execute. For instance our experimental machine, Intel E5-4640 has 4
 * sockets and each socket has 8 cores and 16 threads with the following NUMA topology:
 * @verbatim
$ numactl --hardware
available: 4 nodes (0-3)
node 0 cpus: 0 4 8 12 16 20 24 28 32 36 40 44 48 52 56 60
node 0 size: 130994 MB
node 0 free: 127664 MB
node 1 cpus: 1 5 9 13 17 21 25 29 33 37 41 45 49 53 57 61
node 1 size: 131072 MB
node 1 free: 128517 MB
node 2 cpus: 2 6 10 14 18 22 26 30 34 38 42 46 50 54 58 62
node 2 size: 131072 MB
node 2 free: 128685 MB
node 3 cpus: 3 7 11 15 19 23 27 31 35 39 43 47 51 55 59 63
node 3 size: 131072 MB
node 3 free: 128769 MB
node distances:
node   0   1   2   3
  0:  10  20  20  20
  1:  20  10  20  20
  2:  20  20  10  20
  3:  20  20  20  10
@endverbatim
 *
 * Based on the NUMA topology, on our experimental machine logical threads (0..63)
 * are round-robin distributed to NUMA regions. Therefore, on this machine even
 * not using a cpu-mapping.txt file results in a good configuration. On a different
 * machine it might be necessary to create a cpu-mapping.txt file which tells how
 * logical threads from 0 to N are assigned to underlying physical threads. An example
 * for a hypothetical 2-socket 16-thread machine would be the following
 * (#threads mapping-list #sockets):
 *
 * @verbatim
$ cat cpu-mapping.txt
16 0 1 2 3 8 9 10 11 4 5 6 7 12 13 14 15 2
@endverbatim
 *
 * This file is must be created in the execution directory and used by default
 * if exists in the directory. It basically says that we will use 16 CPUs listed
 * and threads spawned 1 to 16 will map to the given list in order. For instance
 * thread 5 will run on CPU 8. This file must be changed according to the system at
 * hand. If it is absent, threads will be assigned in a round-robin manner
 * (which works well on our experimental machine).
 *
 * @warning NUMA configuration is also automatically obtained if libNUMA is available
 * in the system. Otherwise, the system assumes a single-socket machine without NUMA.
 * This can be a problem if not correctly identified. NUMA configuration is used in
 * determining how to allocate L3 cache and sharing of it among threads. Therefore
 * might pose a significant performance drop especially in the \b m-way algorithm
 * if incorrectly done.
 *
 * @subsection perfmonitoring Performance Monitoring
 *
 * For performance monitoring a config file can be provided on the command line
 * with --perfconf which specifies which hardware counters to monitor. For
 * detailed list of hardware counters consult to "Intel 64 and IA-32
 * Architectures Software Developer's Manual" Appendix A. For an example
 * configuration file used in the experiments, see <b>`pcm.cfg'</b> file.
 * Lastly, an output file name with --perfout on commandline can be specified to
 * print out profiling results, otherwise it defaults to stdout.
 *
 * @subsection systemparams System and Implementation Parameters
 *
 * The join implementations need to know about the system at hand to a certain
 * degree. For instance #CACHE_LINE_SIZE is required in various places of the
 * implementations. Most of the parameters are defined in \b params.h . Moreover,
 * some of these parameters can be passed during the compilation as well as
 * during the runtime from the command line.
 *
 * Some of the important parameters are: #CACHE_LINE_SIZE, #L2_CACHE_SIZE and
 * #L3_CACHE_SIZE (used for multi-way merge size and can be given from command line).
 *
 * @section data Generating Data Sets of Our Experiments
 *
 * Here we briefly describe how to generate data sets used in our experiments
 * with the command line parameters above.
 *
 * @subsection workloadA Workload A variants
 *
 * This data set is adapted from Albutiu et al., where large joins with billions
 * of tuples are targeted. To generate a one instance of this data set with equal
 * table sizes do the following:
 *
 * @verbatim
     $ ./configure
     $ make
     $ ./sortmergejoins [other options] --r-size=1600000000 --s-size=1600000000
@endverbatim
 *
 * @subsection workloadB Workload B
 *
 * In this data set, the inner relation R and outer relation S have 128*10^6
 * tuples each. The tuples are 8 bytes long, consisting of 4-byte (or 32-bit)
 * integers and a 4-byte payload. As for the data distribution, if not
 * explicitly specified, we use relations with randomly shuffled unique keys
 * ranging from 1 to 128*10^6. To generate this data set, append the following
 * parameters to the executable
 * <tt>sortmergejoins</tt>:
 *
 * @verbatim
      $ ./sortmergejoins [other options] --r-size=128000000 --s-size=128000000
@endverbatim
 *
 * @note Adjust other parameters such as (partitioning fan-out: -f --partfanout=<F>
 * and multi-way merge buffer size: -m --mwaybufsize) according to your machine specs.
 *
 * @subsection skew Introducing Skew in Data Sets
 *
 * @warning The skew handling mechanisms like heavy-hitter detection and such
 * are not fully integrated into this version of the code package. Full support for
 * the skew handling as described in the paper will be integrated/merged from a
 * different branch of the code in the upcoming releases. Please contact us if you
 * have questions on this.
 *
 * Skew can be introduced to the relation S as in our experiments by appending
 * the following parameter to the command line, which is basically a Zipf
 * distribution skewness parameter:
 *
 * @verbatim
     $ ./sortmergejoins [other options] --skew=1.05
@endverbatim
 *
 * @section testing Testing
 *
 * Our code uses \b Check unit testing framework for C code (version 0.9.12, provided in lib/).
 * Latest version of Check can be obtained from:
 *
 * http://check.sourceforge.net/
 *
 * Various parts of the implementation such as functionality of sorting, merging,
 * multi-way merging and partitioning are automatically tested by unit tests contained
 * in \b tests/ folder. After installing \b Check in your system,
 * the procedure is as simple as issuing the following command from the top directory:
 *
 * @verbatim
    $ make check
@endverbatim
 *
 * Other than that, there are some scripts provided to test features like join execution
 * and materialization results which can also be found in \b tests/ folder.
 *
 * @author Cagri Balkesen <cagri.balkesen@inf.ethz.ch>
 *
 * (c) 2012-2014, ETH Zurich, Systems Group
 *
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>           /* gettimeofday */
#include <stdlib.h>
#include <limits.h>             /* LONG_MAX */
#include <math.h>
#include <string.h>             /* strlen(), memcpy() */
#include <getopt.h>             /* getopt */
#include <cpuid.h>              /* for check_avx() */

/* #include <assert.h> */

//#include "affinity/affinity.h"           /* CPU_SET, setaffinity(), pthread_attr_setaffinity_np */
#include "affinity/cpu_mapping.h"        /* cpu_mapping_cleanup() */
#include "util/types.h"
#include "datagen/generator.h"
#include "affinity/memalloc.h"           /* malloc_aligned() */
#include "util/params.h"             /* macro parameters */
#include "affinity/numa_shuffle.h"       /* numa_shuffle_init() */
#include <sched.h>
/**************** include join algorithm thread implementations ***************/
#include "joins/sortmergejoin_multipass.h"
#include "joins/sortmergejoin_multiway.h"
#include "joins/sortmergejoin_multiway_skewhandling.h"
#include "joins/sortmergejoin_mpsm.h"

#ifdef JOIN_MATERIALIZE
#include "tuple_buffer.h"       /* for materialization */
#endif

#ifdef PERF_COUNTERS
#include "perf_counters.h"      /* PCM_x */
#endif

#include "config.h"          /* autoconf header */

/** Debug msg logging method */
#ifdef DEBUG
#define DEBUGMSG(COND, MSG, ...)                                        \
    if(COND) {                                                          \
        fprintf(stdout,                                                 \
                "[DEBUG @ %s:%d\] "MSG, __FILE__, __LINE__, ## __VA_ARGS__); \
    }
#else
#define DEBUGMSG(COND, MSG, ...)
#endif
#ifndef INT_MAX
#define INT_MAX 2147483647
#endif

/** Print out timing stats for the given start and end timestamps */
extern void
print_timing(uint64_t numtuples, struct timeval * start, struct timeval * end,
             FILE * out);

/******************************************************************************
 *                                                                            *
 *                 Command-line handling & Driver Program                     *
 *                                                                            *
 ******************************************************************************/

#if !defined(__cplusplus)
int getopt(int argc, char * const argv[],
           const char *optstring);
#endif

typedef struct algo_t  algo_t;
typedef struct cmdparam_t cmdparam_t;

struct algo_t {
    char name[128];
    result_t * (*joinalgorithm)(relation_t *, relation_t *, joinconfig_t *);
};

struct cmdparam_t {
    algo_t * algo;
    uint64_t r_size;
    uint64_t s_size;
    uint32_t nthreads;
    uint32_t r_seed;
    uint32_t s_seed;
    double skew;
    int nonunique_keys;  /* non-unique keys allowed? */
    int verbose;
    int fullrange_keys;  /* keys covers full int range? */
    int no_numa;/* alloc input chunks thread local? */
    char * perfconf;
    char * perfout;
    int scalar_sort;
    int scalar_merge;
    /* partitioning fanout, e.g., 2^rdxbits */
    int part_fanout;
    /* multi-way merge buffer size in bytes, corresponds to L3 size */
    int mwaymerge_bufsize;
    /* NUMA data shuffling strategy */
    enum numa_strategy_t numastrategy;
};

extern char * optarg;
extern int    optind, opterr, optopt;

/** All available algorithms */
static struct algo_t algos [] =
        {
                {"m-way", sortmergejoin_multiway},
                {"m-pass", sortmergejoin_multipass},
                {"mpsm", sortmergejoin_mpsm},
                {"m-way+skew", sortmergejoin_multiway_skewhandling},
                {{0}, 0}
        };

/* command line handling functions */
void
print_help();

void
print_version();

void
parse_args(int argc, char ** argv, cmdparam_t * cmd_params);


#define AVXFlag     ((1UL<<28)|(1UL<<27))
int check_avx()
{
    unsigned int eax, ebx, ecx, edx;
    if (!__get_cpuid (1, &eax, &ebx, &ecx, &edx))
        return 1;

    /* Run AVX test only if host has AVX runtime support.  */
    if ((ecx & AVXFlag) != AVXFlag)
        return 0; /* missing feature */

    return 1; /* has AVX support! */
}

int
main(int argc, char *argv[])
{
    struct timeval start, end;
    relation_t relR;
    relation_t relS;

    /* start initially on CPU-0 */
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(0, &set);
    if (sched_setaffinity(0, sizeof(set), &set) <0) {
        perror("sched_setaffinity");
    }
    /* initialize logical<->physical cpu mappings */
    fprintf(stdout, "[INFO ] Initializing logical <-> physical thread/CPU mappings...\n");
    cpu_mapping_init();

    /* Command line parameters */
    cmdparam_t cmd_params;

    /* Default values if not specified on command line */
    cmd_params.algo = &algos[1]; /* m-pass: sort-merge join with multi-pass merge */
    cmd_params.nthreads = 1;
    /* default dataset is Workload B (described in paper) */
    cmd_params.r_size = 12800;
    cmd_params.s_size = 12800;
    cmd_params.r_seed   = 12345;
    cmd_params.s_seed   = 54321;
    cmd_params.skew     = 0.0;
    cmd_params.verbose  = 0;
    cmd_params.perfconf = NULL;
    cmd_params.perfout  = NULL;
    cmd_params.nonunique_keys   = 0;
    cmd_params.fullrange_keys   = 0;
    cmd_params.no_numa   = 0;
    cmd_params.scalar_sort  = 0;
    cmd_params.scalar_merge = 0;
    /* TODO: get L3 buffer size programmatically. */
    cmd_params.mwaymerge_bufsize = MWAY_MERGE_BUFFER_SIZE_DEFAULT;

    parse_args(argc, argv, &cmd_params);

    if(check_avx() == 0){
        /* no AVX support, just use scalar variants. */
        fprintf(stdout, "[WARN ] AVX is not supported, using scalar sort & merge.\n");
        cmd_params.scalar_sort = 1;
        cmd_params.scalar_merge = 1;
    }

#ifdef PERF_COUNTERS
    PCM_CONFIG = cmd_params.perfconf;
    PCM_OUT    = cmd_params.perfout;
#endif

    /***** create relation R *****/
    fprintf(stdout,
            "[INFO ] Creating relation R with size = %.3lf MiB, #tuples = %lld : ",
            (double) sizeof(tuple_t) * cmd_params.r_size/(1024.0*1024.0),
            cmd_params.r_size);
    fflush(stdout);

    seed_generator(cmd_params.r_seed);

    /* whether to localize input relations in a NUMA-aware manner */
    int nonumalocalize = cmd_params.no_numa;

    /** first allocate the memory for relations (+ padding based on numthreads) : */
    /******* Relation R ********/
    relR.num_tuples = cmd_params.r_size;
    size_t relRsz = relR.num_tuples * sizeof(tuple_t)
                    + RELATION_PADDING(cmd_params.nthreads, cmd_params.part_fanout);
    relR.tuples = (tuple_t *) malloc_aligned(relRsz);
    /* if there is a need NUMA-localize the input: */
    if(!nonumalocalize){
        numa_localize(relR.tuples, relR.num_tuples, cmd_params.nthreads);
    }
    /******* Relation S ********/
    relS.num_tuples = cmd_params.s_size;
    size_t relSsz = relS.num_tuples * sizeof(tuple_t)
                    + RELATION_PADDING(cmd_params.nthreads, cmd_params.part_fanout);
    relS.tuples = (tuple_t *) malloc_aligned(relSsz);
    /* NUMA-localize the input: */
    if(!nonumalocalize){
        numa_localize(relS.tuples, relS.num_tuples, cmd_params.nthreads);
    }

    gettimeofday(&start, NULL);
    if(cmd_params.fullrange_keys) {
        create_relation_nonunique(&relR, cmd_params.r_size, INT_MAX);
    }
    else if(cmd_params.nonunique_keys) {
        create_relation_nonunique(&relR, cmd_params.r_size, cmd_params.r_size);
    }
    else {
        parallel_create_relation(&relR, cmd_params.r_size,
                                 cmd_params.nthreads, cmd_params.r_size);
        /* create_relation_pk(&relR, cmd_params.r_size); */
    }
    gettimeofday(&end, NULL);
    fprintf(stdout, "OK \n");
    fprintf(stdout, "[INFO ] Time: ");
    print_timing(cmd_params.r_size, &start, &end, stdout);
    fprintf(stdout, "\n");

    /***** create relation S *****/
    fprintf(stdout,
            "[INFO ] Creating relation S with size = %.3lf MiB, #tuples = %lld : ",
            (double) sizeof(tuple_t) * cmd_params.s_size/1024.0/1024.0,
            cmd_params.s_size);
    fflush(stdout);

    seed_generator(cmd_params.s_seed);

    gettimeofday(&start, NULL);
    if(cmd_params.fullrange_keys) {
        create_relation_fk_from_pk(&relS, &relR, cmd_params.s_size);
    }
    else if(cmd_params.nonunique_keys) {
        /* use size of R as the maxid */
        create_relation_nonunique(&relS, cmd_params.s_size, cmd_params.r_size);
    }
    else {
        /* if r_size == s_size then equal-dataset, else non-equal dataset */

        if(cmd_params.skew > 0){
            /* S is skewed */
            create_relation_zipf(&relS, cmd_params.s_size,
                                 cmd_params.r_size, cmd_params.skew);
        }
        else {
            /* S is uniform foreign key */
            parallel_create_relation(&relS, cmd_params.s_size,
                                     cmd_params.nthreads, cmd_params.r_size);
            /* create_relation_pk(&relS, cmd_params.s_size); */
        }
    }
    gettimeofday(&end, NULL);
    fprintf(stdout, "OK \n");
    fprintf(stdout, "[INFO ] Time: ");
    print_timing(cmd_params.s_size, &start, &end, stdout);
    fprintf(stdout, "\n");

    /* setup join configuration parameters */
    joinconfig_t joincfg;
    joincfg.NTHREADS = cmd_params.nthreads;
    joincfg.PARTFANOUT = cmd_params.part_fanout;
    joincfg.SCALARMERGE = cmd_params.scalar_merge;
    joincfg.SCALARSORT = cmd_params.scalar_sort;
    joincfg.MWAYMERGEBUFFERSIZE = cmd_params.mwaymerge_bufsize;
    joincfg.NUMASTRATEGY = cmd_params.numastrategy;
    numa_shuffle_init(joincfg.NUMASTRATEGY, joincfg.NTHREADS);

    /* Run the selected join algorithm */
    fprintf(stdout, "[INFO ] Running join algorithm %s ...\n", cmd_params.algo->name);

    result_t * result = cmd_params.algo->joinalgorithm(&relR, &relS, &joincfg);

    if(result != NULL){
        fprintf(stdout, "\n[INFO ] Results = %llu. DONE.\n", result->totalresults);
    }

#if (defined(PERSIST_RELATIONS) && defined(JOIN_MATERIALIZE))
    if(result != NULL){
        printf("[INFO ] Persisting the output result to \"Out.tbl\" ...\n");
        write_result_relation(result, "Out.tbl");
    }
#endif


    /* cleanup */
    free(relR.tuples);
    free(relS.tuples);

#ifdef JOIN_MATERIALIZE
    if(result != NULL){
        for(int i = 0; i < cmd_params.nthreads; i++){
            chainedtuplebuffer_free(result->resultlist[i].results);
        }
    }
#endif

    if(result != NULL){
        free(result->resultlist);
        free(result);
    }
    cpu_mapping_cleanup();

    return 0;
}

/* command line handling functions */
void
print_help(char * progname)
{
    printf("Usage: %s [options]\n", progname);

    printf("Join algorithm selection, algorithms : ");
    int i = 0;
    while(algos[i].joinalgorithm) {
        printf("%s ", algos[i].name);
        i++;
    }
    printf("\n");

    printf("\
       -a --algo=<name>    Run the join algorithm named <name> [m-pass]        \n\
                                                                               \n\
    Other join configuration options, with default values in [] :              \n\
       -n --nthreads=<N>   Number of threads to use <N> [2]                    \n\
       -r --r-size=<R>     Number of tuples in build relation R <R> [128000000]\n\
       -s --s-size=<S>     Number of tuples in probe relation S <S> [128000000]\n\
       -x --r-seed=<x>     Seed value for generating relation R <x> [12345]    \n\
       -y --s-seed=<y>     Seed value for generating relation S <y> [54321]    \n\
       -z --skew=<z>       Zipf skew parameter for probe relation S <z> [0.0]  \n\
       --non-unique        Use non-unique (duplicated) keys in input relations \n\
       --full-range        Spread keys in relns. in full 32-bit integer range  \n\
       --no-numa-localize  Do not NUMA-localize relations to threads [yes]     \n\
       --scalarsort        Use scalar sorting; sort() -> g++ or qsort() -> gcc \n\
       --scalarmerge       Use scalar merging algorithm instead of AVX-merge   \n\
       -f --partfanout=<F> Partitioning fan-out (phase-1, 2^#rdxbits) <F> [128]\n\
       -S --numastrategy   NUMA data shuffle strategy: NEXT,RANDOM,RING [NEXT] \n\
       -m --mwaybufsize    Multi-way merge buffer size in bytes,ideally L3-size\n\
                                                                               \n\
    Performance profiling options, when compiled with --enable-perfcounters.   \n\
       -p --perfconf=<P>   Intel PCM config file with upto 4 counters [none]   \n\
       -o --perfout=<O>    Output file to print performance counters [stdout]  \n\
                                                                               \n\
    Basic user options                                                         \n\
        -h --help          Show this message                                   \n\
        --verbose          Be more verbose -- show misc extra info             \n\
        --version          Show version                                        \n\
    \n");
}

void
print_version()
{
    printf("\n%s\n", PACKAGE_STRING);
    printf("Copyright (c) 2012-2014, ETH Zurich, Systems Group.\n");
    printf("http://www.systems.ethz.ch/projects/paralleljoins\n\n");
}

static char *
mystrdup (const char *s)
{
    char *ss = (char*) malloc (strlen (s) + 1);

    if (ss != NULL)
        memcpy (ss, s, strlen(s) + 1);

    return ss;
}

void
parse_args(int argc, char ** argv, cmdparam_t * cmd_params)
{

    int c, i, found;
    /* Flag set by --verbose */
    static int verbose_flag;
    static int nonunique_flag;
    static int fullrange_flag;
    static int no_numa;
    static int scalarsort = 0;
    static int scalarmerge = 0;
    static int part_fanout = PARTFANOUT_DEFAULT; /* 128 */

    /* default data shuffling strategy is NEXT-based shuffling over NUMA */
    cmd_params->numastrategy = NEXT;

    while(1) {
        static struct option long_options[] =
                {
                        /* These options set a flag. */
                        {"verbose",    no_argument,    &verbose_flag,   1},
                        {"brief",      no_argument,    &verbose_flag,   0},
                        {"non-unique", no_argument,    &nonunique_flag, 1},
                        {"full-range", no_argument,    &fullrange_flag, 1},
                        {"no-numa-localize", no_argument,    &no_numa, 1},
                        {"scalarsort", no_argument,    &scalarsort, 1},
                        {"scalarmerge",no_argument,    &scalarmerge,1},
                        {"help",       no_argument,    0, 'h'},
                        {"version",    no_argument,    0, 'v'},
                        /* These options don't set a flag.
                           We distinguish them by their indices. */
                        {"algo",    required_argument, 0, 'a'},
                        {"nthreads",required_argument, 0, 'n'},
                        {"perfconf",required_argument, 0, 'p'},
                        {"r-size",  required_argument, 0, 'r'},
                        {"s-size",  required_argument, 0, 's'},
                        {"perfout", required_argument, 0, 'o'},
                        {"r-seed",  required_argument, 0, 'x'},
                        {"s-seed",  required_argument, 0, 'y'},
                        {"skew",    required_argument, 0, 'z'},
                        /* partitioning fanout, e.g., 2^rdxbits */
                        {"partfanout",  required_argument, 0, 'f'},
                        {"numastrategy",    required_argument, 0, 'S'},
                        {"mwaybufsize", required_argument, 0, 'm'},
                        {0, 0, 0, 0}
                };
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long (argc, argv, "a:n:p:r:s:o:x:y:z:hvf:m:S:",
                         long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;
        switch (c)
        {
            case 0:
                /* If this option set a flag, do nothing else now. */
                if (long_options[option_index].flag != 0)
                    break;
                printf ("option %s", long_options[option_index].name);
                if (optarg)
                    printf (" with arg %s", optarg);
                printf ("\n");
                break;

            case 'a':
                i = 0; found = 0;
                while(algos[i].joinalgorithm) {
                    if(strcmp(optarg, algos[i].name) == 0) {
                        cmd_params->algo = &algos[i];
                        found = 1;
                        break;
                    }
                    i++;
                }

                if(found == 0) {
                    printf("[ERROR] Join algorithm named `%s' does not exist!\n",
                           optarg);
                    print_help(argv[0]);
                    exit(EXIT_SUCCESS);
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
                if((part_fanout & (part_fanout-1)) != 0){
                    fprintf(stdout, "[ERROR] Partitioning fan-out must be a power of 2 (2^#radixbits).\n");

                    exit(0);
                }

                break;

            case 'm':
                cmd_params->mwaymerge_bufsize = atoi(optarg);
                break;

            case 'S':
                if (strcmp(optarg, "NEXT") == 0)
                    cmd_params->numastrategy = NEXT;
                else if (strcmp(optarg, "RANDOM") == 0)
                    cmd_params->numastrategy = RANDOM;
                else if (strcmp(optarg, "RING") == 0)
                    cmd_params->numastrategy = RING;
                else {
                    printf("Invalid NUMA-shuffle strategy. Options: NEXT, RANDOM, RING\n");
                    printf("Using RING as default.\n");
                }
                break;

            default:
                break;
        }
    }

    /* make sure part fanout is greater the num-threads */
    if(part_fanout < cmd_params->nthreads){
        fprintf(stdout, "[ERROR] Partitioning fan-out must be >= num-threads.\n");
        exit(0);
    }

    /* make sure we're running only scalar when KEY_8B */
#ifdef KEY_8B
    if(!scalarsort || !scalarmerge){
        fprintf(stdout, "[ERROR] AVX instructions are not available for tuples larger than 8-byte.\n");
        exit(0);
    }
#endif

    /* give a warning about the size of merge buffer */
    if(cmd_params->algo->joinalgorithm == sortmergejoin_multiway) {
        if(cmd_params->mwaymerge_bufsize == MWAY_MERGE_BUFFER_SIZE_DEFAULT){
            fprintf(stdout, "[WARN ] Using a default L3-cache size of %.3lf MiB for the multi-way merge buffer.\n",
                    (double)(MWAY_MERGE_BUFFER_SIZE_DEFAULT/1024.0));
            fprintf(stdout, "[WARN ] Change it based on your machine using -m/--mwaybufsize cmd-options.\n");
        }
    }

    /* if (verbose_flag) */
    /*     printf ("verbose flag is set \n"); */

    cmd_params->nonunique_keys = nonunique_flag;
    cmd_params->verbose        = verbose_flag;
    cmd_params->fullrange_keys = fullrange_flag;
    cmd_params->no_numa        = no_numa;
    cmd_params->scalar_sort    = scalarsort;
    cmd_params->scalar_merge   = scalarmerge;
    cmd_params->part_fanout    = part_fanout;

    /* Print any remaining command line arguments (not options). */
    if (optind < argc) {
        printf ("non-option arguments: ");
        while (optind < argc)
            printf ("%s ", argv[optind++]);
        printf ("\n");
    }
}