/**
 * @file    sortmergejoin_multiway.c
 * @author  Cagri Balkesen <cagri.balkesen@inf.ethz.ch>
 * @date    Sat Dec 15 15:37:54 2012
 * @version $Id $
 *
 * @brief   m-way sort-merge-join algorithm with multi-way merging.
 *          It uses AVX-based sorting and merging if scalarsort and scalarmerge
 *          flags are not provided.
 *
 *
 * (c) 2012-2014, ETH Zurich, Systems Group
 *
 * \ingroup Joins
 */

#include <math.h>   /* log2(), ceil() */
#include <stdlib.h> /* malloc() */
#include <string.h> /* memcpy() */

#include "../affinity/cpu_mapping.h" /* cpu_id NUMA related methods */
#include "../utils/barrier.h"        /* pthread_barrier_* */

#include "../affinity/memalloc.h"     /* malloc_aligned() */
#include "../affinity/numa_shuffle.h" /* get_numa_shuffle_strategy() */
#include "avx_multiwaymerge.h"        /* avx_multiway_merge() */
#include "avxsort.h"                  /* avxsort_tuples() */
#include "common_functions.h"
#include "partition.h"            /* partition_relation_optimized() */
#include "scalar_multiwaymerge.h" /* scalar_multiway_merge() */
#include "scalarsort.h"           /* scalarsort_tuples() */
#include "sortmergejoin_multiway.h"
#include "unistd.h"

#ifdef JOIN_MATERIALIZE
#include "../utils/tuple_buffer.h"
#endif
#ifdef PERF_COUNTERS

#include "../utils/perf_counters.h" /* PCM_x */

#endif

/**
 * Main thread of First Sort-Merge Join variant with partitioning and complete
 * sorting of both input relations. The merging step in this algorithm tries to
 * overlap the first merging and transfer of remote chunks. However, in compared
 * to other variants, merge phase still takes a significant amount of time.
 *
 * @param param parameters of the thread, see arg_t for details.
 *
 */
void *sortmergejoin_multiway_thread(void *param);

result_t *sortmergejoin_multiway(relation_t *relR, relation_t *relS,
                                 joinconfig_t *joincfg, int exp_id,
                                 int window_size, int gap) {
    /* check whether nr. of threads is a power of 2 */
    if ((joincfg->NTHREADS & (joincfg->NTHREADS - 1)) != 0) {
        fprintf(stdout,
                "[ERROR] m-way sort-merge join runs with a power of 2 #threads.\n");
        return 0;
    }

    return sortmergejoin_initrun(relR, relS, joincfg,
                                 sortmergejoin_multiway_thread, exp_id,
                                 window_size, gap, "MWAY");
}

/**
 * NUMA-local partitioning of both input relations into PARTFANOUT.
 *
 * @param[out] relRparts partitions of relation R
 * @param[out] relSparts partitions of relation S
 * @param[in] args thread arguments
 */
void partitioning_phase(relation_t ***relRparts, relation_t ***relSparts,
                        arg_t *args);

void partitioning_cleanup(relation_t **relRparts, relation_t **relSparts) {
    free(relRparts[0]);
    free(relRparts);
    free(relSparts);
}

/**
 * NUMA-local sorting of partitions of both relations.
 *
 * @note sorted outputs are written back to the original relation, therefore
 * input relation is not preserved in this implementation.
 *
 * @param[in] relRparts partitions of relation R
 * @param[in] relSparts partitions of relation S
 * @param[in] args thread arguments
 */
void sorting_phase(relation_t **relRparts, relation_t **relSparts, arg_t *args);

/**
 * Multi-way merging of sorted NUMA-runs with in-cache resident buffers.
 *
 * @param[in] numaregionid NUMA region id of the executing thread
 * @param[out] relRparts sorted partitions of relation R for joining when
 * nthreads=1
 * @param[out] relSparts sorted partitions of relation S for joining when
 * nthreads=1
 * @param[in] args thread arguments
 * @param[out] mergedRelR completely merged relation R
 * @param[out] mergedRelS completely merged relation S
 */
void multiwaymerge_phase(int numaregionid, relation_t **relRparts,
                         relation_t **relSparts, arg_t *args,
                         relation_t *mergedRelR, relation_t *mergedRelS);

/**
 * Evaluate the merge-join over NUMA-local sorted runs.
 *
 * @param[in] relRparts sorted parts of NUMA-local relation-R if nthr=1
 * @param[in] relSparts sorted parts of NUMA-local relation-S if nthr=1
 * @param[in] mergedRelR sorted entire NUMA-local relation-R if nthr>1
 * @param[in] mergedRelS sorted entire NUMA-local relation-S if nthr>1
 * @param[in,out] args return values are stored in args
 */
void mergejoin_phase(relation_t **relRparts, relation_t **relSparts,
                     relation_t *mergedRelR, relation_t *mergedRelS,
                     arg_t *args);

/**
 * Main execution thread of "m-way" sort-merge join.
 *
 * @param param
 */
void *sortmergejoin_multiway_thread(void *param) {
    arg_t *args = (arg_t *) param;
    int32_t my_tid = args->tid;
    int rv;

//  MSG("Thread-%d started running ... \n", my_tid);

#ifdef PROFILE_TOPDOWN
#ifdef JOIN_THREAD
    sleep(1);
#else
    return nullptr;
#endif
#endif

#ifdef OVERVIEW // overview counters
#ifdef PERF_COUNTERS
    if (my_tid == 0) {
        PCM_initPerformanceMonitor(NULL, NULL);
        PCM_start();
        auto curtime = std::chrono::steady_clock::now();
        string path = "/data1/xtra/time_start_" + std::to_string(args->exp_id) + ".txt";
        auto fp = fopen(path.c_str(), "w");
        fprintf(fp, "%ld\n", curtime);
        sleep(1);
    }
    BARRIER_ARRIVE(args->barrier, rv);
#endif
#endif

#ifdef PARTITION // partition only
#ifdef PERF_COUNTERS
    if (my_tid == 0) {
      PCM_initPerformanceMonitor(NULL, NULL);
      PCM_start();
    }
    BARRIER_ARRIVE(args->barrier, rv);
#endif
#endif

    BARRIER_ARRIVE(args->barrier, rv);

#ifndef NO_TIMING
    if (my_tid == 0) {
        *args->startTS = curtick(); // assign the start timestamp
        START_MEASURE((args->timer))
        BEGIN_MEASURE_PARTITION(args->timer) /* partitioning start */
    }

#endif

    /*************************************************************************
     *
     *   Phase.1) NUMA-local partitioning.
     *
     *************************************************************************/
    relation_t **partsR = NULL;
    relation_t **partsS = NULL;
    partitioning_phase(&partsR, &partsS, args);

#ifdef PARTITION // partition only
#ifdef PERF_COUNTERS
    BARRIER_ARRIVE(args->barrier, rv);
    if (my_tid == 0) {
      PCM_stop();
      PCM_log("========= 1) Profiling results of Partitioning Phase =========\n");
      PCM_printResults();
      PCM_cleanup();
    }
#endif
#endif
    BARRIER_ARRIVE(args->barrier, rv);

#ifndef NO_TIMING
    if (args->tid == 0)
        END_MEASURE_PARTITION(args->timer) /* sort end */
#endif

    // we don't care sorting as it's minor
    //#ifdef PERF_COUNTERS
    //    if (my_tid == 0) {
    //        PCM_start();
    //    }
    //    BARRIER_ARRIVE(args->barrier, rv);
    //#endif

#ifndef NO_TIMING
    if (args->tid == 0)
        BEGIN_MEASURE_SORT_ACC(args->timer) /* sort start */
#endif

    /*************************************************************************
     *
     *   Phase.2) NUMA-local sorting of cache-sized chunks
     *
     *************************************************************************/
    sorting_phase(partsR, partsS, args);

    /**
     * Allocate shared merge buffer for multi-way merge tree.
     * This buffer is further divided into given number of threads
     * active in the same NUMA-region.
     *
     * @note the first thread in each NUMA region allocates the shared L3 buffer.
     */
    int numaregionid = get_numa_region_id(my_tid);
    if (is_first_thread_in_numa_region(my_tid)) {
        /* TODO: make buffer size runtime parameter */
        tuple_t *sharedmergebuffer =
                (tuple_t *) malloc_aligned(args->joincfg->MWAYMERGEBUFFERSIZE);
        args->sharedmergebuffer[numaregionid] = sharedmergebuffer;

        DEBUGMSG(1,
                 "Thread-%d allocated %.3lf KiB merge buffer in NUMA-region-%d to "
                 "be used by %d active threads.\n",
                 my_tid, (double) (args->joincfg->MWAYMERGEBUFFERSIZE / 1024.0),
                 numaregionid, get_num_active_threads_in_numa(numaregionid));
    }

    //#ifdef PERF_COUNTERS
    //        BARRIER_ARRIVE(args->barrier, rv);
    //        if (my_tid == 0) {
    //            PCM_stop();
    //            PCM_log("========= 2) Profiling results of Sorting Phase
    //            =========\n"); PCM_printResults();
    //        }
    //#endif

    BARRIER_ARRIVE(args->barrier, rv);
#ifndef NO_TIMING
    if (args->tid == 0)
        END_MEASURE_SORT_ACC(args->timer) /* sort end */
#endif

    /* check whether local relations are sorted? */
#if 0
    {
    tuple_t * tmparr = (tuple_t *) malloc(sizeof(tuple_t)*args->numR);
    uint32_t off = 0;
    int i;
    const int PARTFANOUT = args->joincfg->PARTFANOUT;
    for(i = 0; i < PARTFANOUT; i ++) {
        relationpair_t * rels = & args->threadrelchunks[my_tid][i];
        memcpy((void*)(tmparr+off), (void*)(rels->R.tuples), rels->R.num_tuples*sizeof(tuple_t));
        off += rels->R.num_tuples;
    }
    if(is_sorted_helper((int64_t*)tmparr, args->numR))
        printf("[INFO ] %d-thread -> relR is sorted, size = %d\n", my_tid, args->numR);
    else
        printf("[ERROR] %d-thread -> relR is NOT sorted, size = %d, off=%d********\n", my_tid, args->numR, off);
    free(tmparr);
    tmparr = (tuple_t *) malloc(sizeof(tuple_t)*args->numS);
    off = 0;
    for(i = 0; i < PARTFANOUT; i ++) {
        relationpair_t * rels = & args->threadrelchunks[my_tid][i];
        memcpy((void*)(tmparr+off), (void*)(rels->S.tuples), rels->S.num_tuples*sizeof(tuple_t));
        off += rels->S.num_tuples;
    }
    if(is_sorted_helper((int64_t*)tmparr, args->numS))
        printf("[INFO ] %d-thread -> relS is sorted, size = %d\n", my_tid, args->numS);
    else
        printf("[ERROR] %d-thread -> relS is NOT sorted, size = %d\n", my_tid, args->numS);
    }
#endif

    // don't care merge as it's minor
    //#ifdef PERF_COUNTERS
    //    if (my_tid == 0) {
    //        PCM_start();
    //    }
    //    BARRIER_ARRIVE(args->barrier, rv);
    //#endif

#ifndef NO_TIMING
    //    BEGIN_MEASURE_MERGEDELTA(args->timer)/* mergedelta start */
    if (args->tid == 0)
        BEGIN_MEASURE_MERGE_ACC(args->timer) /* merge start */
#endif
    //    BEGIN_MEASURE_JOIN(args->timer)/* join start */
    /*************************************************************************
     *
     *   Phase.3) Apply multi-way merging with in-cache resident buffers.
     *
     *************************************************************************/
    relation_t mergedRelR;
    relation_t mergedRelS;
    multiwaymerge_phase(numaregionid, partsR, partsS, args, &mergedRelR,
                        &mergedRelS);

    BARRIER_ARRIVE(args->barrier, rv);
#ifndef NO_TIMING
    //    END_MEASURE_MERGEDELTA(args->timer)/* mergedeleta end */
#endif

    if (my_tid == 0) {
        //        stopTimer(&args->mergedelta);
        //        args->merge = args->mergedelta; /* since we do merge in single go.
        //        */
        DEBUGMSG(1, "Multi-way merge is complete!\n");
        /* the thread that allocated the merge buffer releases it. */
        if (is_first_thread_in_numa_region(my_tid)) {
            free(args->sharedmergebuffer[numaregionid]);
            // free_threadlocal(args->sharedmergebuffer[numaregionid],
            // MWAY_MERGE_BUFFER_SIZE);
        }
    }

//#ifdef PERF_COUNTERS
//    BARRIER_ARRIVE(args->barrier, rv);
//    if (my_tid == 0) {
//        PCM_stop();
//        PCM_log("========= 3) Profiling results of Multi-Way NUMA-MergePhase =========\n");
//        PCM_printResults();
//    }
//    BARRIER_ARRIVE(args->barrier, rv);
//#endif

#ifndef NO_TIMING
    if (args->tid == 0)
    END_MEASURE_MERGE_ACC(args->timer) /* merge end */
#endif

    /* To check whether sorted? */
    /*
    check_sorted((int64_t *)tmpoutR, (int64_t *)tmpoutS,
                 mergeRtotal, mergeStotal, my_tid);
     */
#ifndef NO_TIMING
    if (args->tid == 0) {
        BEGIN_MEASURE_JOIN_ACC(args->timer)
    }
#endif

#ifdef JOIN
#ifdef PERF_COUNTERS
    MSG("PCM starts..")
    if (my_tid == 0) {
        PCM_initPerformanceMonitor(NULL, NULL);
        PCM_start();
    }
    BARRIER_ARRIVE(args->barrier, rv);
#endif
#endif
    /*************************************************************************
     *
     *   Phase.4) NUMA-local merge-join on local sorted runs.
     *
     *************************************************************************/
    mergejoin_phase(partsR, partsS, &mergedRelR, &mergedRelS, args);

#ifndef NO_TIMING
    BARRIER_ARRIVE(args->barrier, rv);
    if (args->tid == 0) {
        END_MEASURE_JOIN_ACC(args->timer) /* join end */
        END_MEASURE(args->timer)          /* end overall*/
    }
#endif

#ifdef JOIN
#ifdef PERF_COUNTERS
    BARRIER_ARRIVE(args->barrier, rv);
    if (my_tid == 0) {
        PCM_stop();
        PCM_log("========= 4) results of Multi-Way Joining Phase =========\n");
        PCM_printResults();
        PCM_cleanup();
    }
#endif
    BARRIER_ARRIVE(args->barrier, rv);
#endif

#ifdef OVERVIEW
#ifdef PERF_COUNTERS
BARRIER_ARRIVE(args->barrier, rv);
if (my_tid == 0) {
    PCM_stop();
    auto curtime = std::chrono::steady_clock::now();
    string path = "/data1/xtra/time_end_" + std::to_string(args->exp_id) + ".txt";
    auto fp = fopen(path.c_str(), "w");
    fprintf(fp, "%ld\n", curtime);
    PCM_log("========= results of Overview =========\n");
    PCM_printResults();
    PCM_cleanup();
}
#endif
    BARRIER_ARRIVE(args->barrier, rv);
#endif

    /* clean-up */
    partitioning_cleanup(partsR, partsS);

    free(args->threadrelchunks[my_tid]);
    /* clean-up temporary relations */
    if (args->nthreads > 1) {
        free_threadlocal(mergedRelR.tuples,
                         mergedRelR.num_tuples * sizeof(tuple_t));
        free_threadlocal(mergedRelS.tuples,
                         mergedRelS.num_tuples * sizeof(tuple_t));
    }

    return 0;
}

void partitioning_phase(relation_t ***relRparts, relation_t ***relSparts,
                        arg_t *args) {
    const int PARTFANOUT = args->joincfg->PARTFANOUT;
    const int NRADIXBITS = log2(PARTFANOUT);

    relation_t **partsR =
            (relation_t **) malloc_aligned(PARTFANOUT * sizeof(relation_t *));
    relation_t **partsS =
            (relation_t **) malloc_aligned(PARTFANOUT * sizeof(relation_t *));

    /** note: only free prels[0] when releasing memory */
    relation_t *prels = (relation_t *) malloc(2 * PARTFANOUT * sizeof(relation_t));
    for (int i = 0; i < PARTFANOUT; i++) {
        partsR[i] = prels + i;
        partsS[i] = prels + PARTFANOUT + i;
    }

    relation_t relR, relS;
    relation_t tmpR, tmpS;
    relR.tuples = args->relR;
    relR.num_tuples = args->numR;
    relS.tuples = args->relS;
    relS.num_tuples = args->numS;
    tmpR.tuples = args->tmp_partR;
    tmpR.num_tuples = args->numR;
    tmpS.tuples = args->tmp_partS;
    tmpS.num_tuples = args->numS;

    /* a maximum of one cache-line padding between partitions in the output */
    /*
    tuple_t * partoutputR = malloc_aligned(relR.num_tuples * sizeof(tuple_t)
                                            + PARTFANOUT * CACHE_LINE_SIZE);
    tuple_t * partoutputS = malloc_aligned(relS.num_tuples * sizeof(tuple_t)
                                            + PARTFANOUT * CACHE_LINE_SIZE);
    tmpR.tuples = partoutputR;
    tmpS.tuples = partoutputS;
    tmpR.num_tuples = args->numR;
    tmpS.num_tuples = args->numS;
    */
    /* after partitioning tmpR, tmpS holds the partitioned data */
    int bitshift = ceil(log2(relR.num_tuples * args->nthreads)) - 1;
    if (args->nthreads == 1)
        bitshift = bitshift - NRADIXBITS + 1;
    else {
#if SKEW_HANDLING
        /* NOTE: Special to skew handling code, we must set the radix bits in a
           way that the MSB-Radix partitioning results in range partitioning
           assuming keys are dense. */
        bitshift = bitshift - NRADIXBITS + 1;
#else
        bitshift = bitshift - NRADIXBITS - 1;
#endif
    }

    DEBUGMSG(1, "[INFO ] bitshift = %d\n", bitshift);

    partition_relation_optimized(partsR, &relR, &tmpR, NRADIXBITS, bitshift);
    partition_relation_optimized(partsS, &relS, &tmpS, NRADIXBITS, bitshift);

    /** return parts */
    *relRparts = partsR;
    *relSparts = partsS;
}

void sorting_phase(relation_t **relRparts, relation_t **relSparts,
                   arg_t *args) {
    const int PARTFANOUT = args->joincfg->PARTFANOUT;

    int32_t my_tid = args->tid;

    args->threadrelchunks[my_tid] =
            (relationpair_t *) malloc_aligned(PARTFANOUT * sizeof(relationpair_t));

    uint64_t ntuples_per_part;
    uint64_t offset = 0;
    tuple_t *optr = args->tmp_sortR + my_tid * CACHELINEPADDING(PARTFANOUT);

    for (int i = 0; i < PARTFANOUT; i++) {
        tuple_t *inptr = (relRparts[i]->tuples);
        tuple_t *outptr = (optr + offset);
        ntuples_per_part = relRparts[i]->num_tuples;
        offset += ALIGN_NUMTUPLES(ntuples_per_part);

        DEBUGMSG(0, "PART-%d-SIZE: %d", PRIu64 "\n", i, relRparts[i]->num_tuples);

        if (scalarflag)
            scalarsort_tuples(&inptr, &outptr, ntuples_per_part);
        else
            avxsort_tuples(&inptr, &outptr, ntuples_per_part);

#if DEBUG_SORT_CHECK
        if (!is_sorted_helper((int64_t *)outptr, ntuples_per_part)) {
          printf("===> %d-thread -> R is NOT sorted, size = %d\n", my_tid,
                 ntuples_per_part);
        }
#endif

        args->threadrelchunks[my_tid][i].R.tuples = outptr;
        args->threadrelchunks[my_tid][i].R.num_tuples = ntuples_per_part;
    }

    offset = 0;
    optr = args->tmp_sortS + my_tid * CACHELINEPADDING(PARTFANOUT);
    for (int i = 0; i < PARTFANOUT; i++) {
        tuple_t *inptr = (relSparts[i]->tuples);
        tuple_t *outptr = (optr + offset);

        ntuples_per_part = relSparts[i]->num_tuples;
        offset += ALIGN_NUMTUPLES(ntuples_per_part);
        /*
        if(my_tid==0)
             fprintf(stdout, "PART-%d-SIZE: %d\n", i, relSparts[i]->num_tuples);
        */
        if (scalarflag)
            scalarsort_tuples(&inptr, &outptr, ntuples_per_part);
        else
            avxsort_tuples(&inptr, &outptr, ntuples_per_part);

#if DEBUG_SORT_CHECK
        if (!is_sorted_helper((int64_t *)outptr, ntuples_per_part)) {
          printf("===> %d-thread -> S is NOT sorted, size = %d\n", my_tid,
                 ntuples_per_part);
        }
#endif

        args->threadrelchunks[my_tid][i].S.tuples = outptr;
        args->threadrelchunks[my_tid][i].S.num_tuples = ntuples_per_part;
        /* if(my_tid == 0) */
        /* printf("S-MYTID=%d FAN=%d OUT-START=%llu\nS-MYTID=%d FAN=%d
         * OUT-END=%llu\n", */
        /*        my_tid, i, outptr, my_tid, i, (outptr+ntuples_per_part)); */
    }
}

void multiwaymerge_phase(int numaregionid, relation_t **relRparts,
                         relation_t **relSparts, arg_t *args,
                         relation_t *mergedRelR, relation_t *mergedRelS) {
    const int PARTFANOUT = args->joincfg->PARTFANOUT;

    int32_t my_tid = args->tid;
    uint64_t mergeRtotal = 0, mergeStotal = 0;
    tuple_t *tmpoutR = NULL;
    tuple_t *tmpoutS = NULL;

    if (args->nthreads == 1) {
        /* single threaded execution; no multi-way merge. */
        for (int i = 0; i < PARTFANOUT; i++) {
            relationpair_t *rels = &args->threadrelchunks[my_tid][i];
            mergeRtotal += rels->R.num_tuples;
            mergeStotal += rels->S.num_tuples;

            /* evaluate join between each sorted part */
            relRparts[i]->tuples = rels->R.tuples;
            relRparts[i]->num_tuples = rels->R.num_tuples;
            relSparts[i]->tuples = rels->S.tuples;
            relSparts[i]->num_tuples = rels->S.num_tuples;
        }
    } else {
        uint32_t j;
        const uint32_t perthread = PARTFANOUT / args->nthreads;

        /* multi-threaded execution */
        /* merge remote relations and bring to local memory */
        const uint32_t start = my_tid * perthread;
        const uint32_t end = start + perthread;

        relation_t *Rparts[PARTFANOUT];
        relation_t *Sparts[PARTFANOUT];

        /* compute the size of merged relations to be stored locally */
        uint32_t f = 0;
        for (j = start; j < end; j++) {
            for (int i = 0; i < args->nthreads; i++) {
                // uint32_t tid = (my_tid + i) % args->nthreads;
                uint32_t tid = get_numa_shuffle_strategy(my_tid, i, args->nthreads);
                // printf("SHUF %d %d --> %d\n", i, my_tid, tid);
                relationpair_t *rels = &args->threadrelchunks[tid][j];
                // fprintf(stdout, "TID=%d Part-%d-size = %d\n", my_tid, f,
                // rels->S.num_tuples);
                Rparts[f] = &rels->R;
                Sparts[f] = &rels->S;
                f++;

                mergeRtotal += rels->R.num_tuples;
                mergeStotal += rels->S.num_tuples;
            }
        }

        /* allocate memory at local node for temporary merge results */
        tmpoutR =
                (tuple_t *) malloc_aligned_threadlocal(mergeRtotal * sizeof(tuple_t));
        tmpoutS =
                (tuple_t *) malloc_aligned_threadlocal(mergeStotal * sizeof(tuple_t));

        /* determine the L3 cache-size per thread */
        /* int nnuma = get_num_numa_regions(); */

        /* active number of threads in the current NUMA-region: */
        int active_nthreads_in_numa = get_num_active_threads_in_numa(numaregionid);

        /* index of the current thread in its NUMA-region: */
        int numatidx = get_thread_index_in_numa(my_tid);

        /* get the exclusive part of the merge buffer for the current thread */
        int bufsz_thr =
                (args->joincfg->MWAYMERGEBUFFERSIZE / active_nthreads_in_numa) /
                sizeof(tuple_t);
        tuple_t *mergebuf =
                args->sharedmergebuffer[numaregionid] + (numatidx * bufsz_thr);

        /* now do the multi-way merging */
        if (scalarflag) {
            scalar_multiway_merge(tmpoutR, Rparts, PARTFANOUT, mergebuf, bufsz_thr);
            scalar_multiway_merge(tmpoutS, Sparts, PARTFANOUT, mergebuf, bufsz_thr);
        } else {
            avx_multiway_merge(tmpoutR, Rparts, PARTFANOUT, mergebuf, bufsz_thr);
            avx_multiway_merge(tmpoutS, Sparts, PARTFANOUT, mergebuf, bufsz_thr);
        }
    }

    /** returned merged relations, only if nthreads > 1 */
    mergedRelR->tuples = tmpoutR;
    mergedRelR->num_tuples = mergeRtotal;
    mergedRelS->tuples = tmpoutS;
    mergedRelS->num_tuples = mergeStotal;
}

void mergejoin_phase(relation_t **relRparts, relation_t **relSparts,
                     relation_t *mergedRelR, relation_t *mergedRelS,
                     arg_t *args) {

#ifdef JOIN_MATERIALIZE
    chainedtuplebuffer_t *chainedbuf = chainedtuplebuffer_init();
#else
    void *chainedbuf = NULL;
#endif

    const int PARTFANOUT = args->joincfg->PARTFANOUT;
    uint64_t nresults = 0;

    if (args->nthreads > 1) {
        tuple_t *rtuples = (tuple_t *) mergedRelR->tuples;
        tuple_t *stuples = (tuple_t *) mergedRelS->tuples;

        nresults = merge_join(rtuples, stuples, mergedRelR->num_tuples,
                              mergedRelS->num_tuples, chainedbuf, args->timer);

    } else {
        /* single-threaded execution: just join sorted partition-pairs */
        for (int i = 0; i < PARTFANOUT; i++) {
            /* evaluate join between each sorted part */
            nresults += merge_join(relRparts[i]->tuples, relSparts[i]->tuples,
                                   relRparts[i]->num_tuples, relSparts[i]->num_tuples,
                                   chainedbuf, args->timer);
        }
    }
    args->result = nresults;
    //    printf("TID=%d --> #res = %d %d\n", args->my_tid, args->result,
    //    nresults);

#ifdef JOIN_MATERIALIZE
    args->threadresult->nresults = nresults;
    args->threadresult->threadid = args->tid;
    args->threadresult->results = (void *)chainedbuf;
#endif
}
