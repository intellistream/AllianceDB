/**
 * @file    sortmergejoin_multipass.c
 * @author  Cagri Balkesen <cagri.balkesen@inf.ethz.ch>
 * @date    Sat Dec 15 15:38:34 2012
 * @version $Id $
 *
 * @brief   m-pass sort-merge-join algorithm with multi-pass merging.
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

#include "../utils/barrier.h" /* pthread_barrier_* */

#include "../affinity/memalloc.h" /* malloc_aligned() */
#include "avxsort.h"              /* avxsort_tuples() */
#include "common_functions.h"
#include "merge.h"      /* avx_merge_tuples(), scalar_merge_tuples() */
#include "partition.h"  /* partition_relation_optimized() */
#include "scalarsort.h" /* scalarsort_tuples() */
#include "sortmergejoin_multipass.h"
#include "unistd.h"

#ifdef JOIN_MATERIALIZE
#include "../utils/tuple_buffer.h"
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
void *sortmergejoin_multipass_thread(void *param);

result_t *sortmergejoin_multipass(relation_t *relR, relation_t *relS,
                                  joinconfig_t *joincfg, int exp_id,
                                  int window_size, int gap) {
    /* check whether nr. of threads is a power of 2 */
    if ((joincfg->NTHREADS & (joincfg->NTHREADS - 1)) != 0) {
        fprintf(
                stdout,
                "[ERROR] m-pass sort-merge join runs with a power of 2 #threads.\n");
        return 0;
    }

    return sortmergejoin_initrun(relR, relS, joincfg,
                                 sortmergejoin_multipass_thread, exp_id,
                                 window_size, gap, "MPASS");
}

/**
 * NUMA-local partitioning of both input relations into PARTFANOUT.
 *
 * @param[out] relRparts partitions of relation R
 * @param[out] relSparts partitions of relation S
 * @param[in] args thread arguments
 */
void mpass_partitioning_phase(relation_t ***relRparts, relation_t ***relSparts,
                              arg_t *args);

void mpass_partitioning_cleanup(relation_t **relRparts,
                                relation_t **relSparts) {
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
void mpass_sorting_phase(relation_t **relRparts, relation_t **relSparts,
                         arg_t *args);

/**
 * First merge phase of NUMA-sorted-runs. Brings remote runs to local
 * NUMA-node while merging pair-wise at the same time.
 *
 * @param[in] args thread arguments
 * @param[out] mergedRelR partially merged relation R
 * @param[out] mergedRelS partially merged relation S
 * @param[out] numrunstomerge number of runs that still needs to be further
 * merged
 * @param[out] mergerunsR sorted runs of R to be further merged
 * @param[out] mergerunsS sorted runs of S to be further merged
 */
void mpass_firstnumamerge_phase(arg_t *args, relation_t *mergedRelR,
                                relation_t *mergedRelS, int *numrunstomerge,
                                relation_t **mergerunsR,
                                relation_t **mergerunsS);

/**
 * Full merge of NUMA-local runs with a 2-way multi-pass merge.
 *
 * @param[in] args thread arguments
 * @param[in] numrunstomerge number of runs that needs to be merged
 * @param[in] mergerunsR sorted input runs of R to be further merged
 * @param[in] mergerunsS sorted input runs of S to be further merged
 * @param[in,out] mergedRelR merged relation R
 * @param[in,out] mergedRelS merged relation S
 */
void mpass_fullmultipassmerge_phase(arg_t *args, int numrunstomerge,
                                    relation_t *mergerunsR,
                                    relation_t *mergerunsS,
                                    relation_t *mergedRelR,
                                    relation_t *mergedRelS);

/**
 * Evaluate the merge-join over NUMA-local sorted runs.
 *
 * @param[in] mergedRelR sorted entire NUMA-local relation-R
 * @param[in] mergedRelS sorted entire NUMA-local relation-S
 * @param[in,out] args return values are stored in args
 */
void mpass_mergejoin_phase(relation_t *mergedRelR, relation_t *mergedRelS,
                           arg_t *args);

void *sortmergejoin_multipass_thread(void *param) {
    arg_t *args = (arg_t *) param;
    int32_t my_tid = args->tid;
    int rv;

    DEBUGMSG(1, "Thread-%d started running ... \n", my_tid);

    int lock;
    /* wait at a barrier until each thread started*/
    BARRIER_ARRIVE(args->barrier, lock)
    *args->startTS = curtick();

#ifndef NO_TIMING
    START_MEASURE(args->timer)
#endif

#ifdef PROFILE_TOPDOWN
#ifdef JOIN_THREAD
    if (my_tid == 0) {
        sleep(1);
    }
    BARRIER_ARRIVE(args->barrier, rv);
#else
    return nullptr;
#endif
#endif

#ifdef OVERVIEW // partition only
#ifdef PERF_COUNTERS
    if (my_tid == 0) {
        PCM_initPerformanceMonitor(NULL, NULL);
        PCM_start();
        auto curtime = std::chrono::steady_clock::now();
        string path = EXP_DIR "/results/breakdown/time_start_" + std::to_string(args->exp_id) + ".txt";
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

#ifndef NO_TIMING
    BEGIN_MEASURE_PARTITION(args->timer) /* partitioning start */
#endif
    /*************************************************************************
     *
     *   Phase.1) NUMA-local partitioning.
     *
     *************************************************************************/
    relation_t **partsR = NULL;
    relation_t **partsS = NULL;
    mpass_partitioning_phase(&partsR, &partsS, args);

    BARRIER_ARRIVE(args->barrier, rv);

#ifdef PARTITION // partition only
#ifdef PERF_COUNTERS
    if (my_tid == 0) {
      PCM_stop();
      PCM_log("========= 1) Profiling results of Partitioning Phase =========\n");
      PCM_printResults();
      PCM_cleanup();
    }
#endif
#endif

#ifndef NO_TIMING
    END_MEASURE_PARTITION(args->timer) /* partition end */
#endif

#ifndef NO_TIMING
    BEGIN_MEASURE_SORT_ACC(args->timer) /* sort start */
#endif

#ifdef SORT // profile sort
#ifdef PERF_COUNTERS
    if (my_tid == 0) {
        PCM_start();
    }
    BARRIER_ARRIVE(args->barrier, rv);
#endif
#endif
    /*************************************************************************
     *
     *   Phase.2) NUMA-local sorting of cache-sized chunks
     *
     *************************************************************************/
    mpass_sorting_phase(partsR, partsS, args);

    BARRIER_ARRIVE(args->barrier, rv);

#ifndef NO_TIMING
    END_MEASURE_SORT_ACC(args->timer) /* sort end */
#endif
#ifdef SORT // profile sort
#ifdef PERF_COUNTERS
    if (my_tid == 0) {
        PCM_stop();
        PCM_log("========= 2) Profiling results of Sorting Phase=========\n");
        PCM_printResults();
        PCM_cleanup();
    }
#endif
#endif
        //
        //#ifdef PERF_COUNTERS
        //    BARRIER_ARRIVE(args->barrier, rv);
        //    if(my_tid == 0){
        //        PCM_start();
        //    }
        //#endif

#ifndef NO_TIMING
    BEGIN_MEASURE_MERGE_ACC(args->timer) /* merge start */
#endif
    /*************************************************************************
     *
     *   Phase.3.1) First merge of NUMA-runs, Bringing remote runs to local
     *   NUMA-node while merging pair-wise at the same time.
     *
     *************************************************************************/
    relation_t mergedRelR;
    relation_t mergedRelS;
    int numrunstomerge = 0;
    relation_t *mergerunsR;
    relation_t *mergerunsS;
    mpass_firstnumamerge_phase(args, &mergedRelR, &mergedRelS, &numrunstomerge,
                               &mergerunsR, &mergerunsS);

    BARRIER_ARRIVE(args->barrier, rv);

#ifndef NO_TIMING
//    END_MEASURE_MERGEDELTA(args->timer)/* mergedeleta end */
#endif

    if (my_tid == 0) {
        //        stopTimer(&args->mergedelta);
        /* we don't need the partitioning & sorting temporary spaces any more. */
        if (args->nthreads > 1) {
            free(args->tmp_partR);
            free(args->tmp_partS);
            free(args->tmp_sortR);
            free(args->tmp_sortS);
            args->tmp_partR = 0;
        }
        DEBUGMSG(1, "First level numa-remote merge is complete!\n");
    }
    //#ifdef PERF_COUNTERS
    //    BARRIER_ARRIVE(args->barrier, rv);
    //    if(my_tid == 0) {
    //        PCM_stop();
    //        PCM_log("========== 3) Profiling results of First NUMA-Merge Phase
    //        ==========\n"); PCM_printResults(); PCM_start();
    //    }
    //    BARRIER_ARRIVE(args->barrier, rv);
    //#endif

    /*************************************************************************
     *
     *   Phase.3.2) Full merge of NUMA-local runs with a 2-way multi-pass merge
     *
     *************************************************************************/
    mpass_fullmultipassmerge_phase(args, numrunstomerge, mergerunsR, mergerunsS,
                                   &mergedRelR, &mergedRelS);

    BARRIER_ARRIVE(args->barrier, rv);

#ifndef NO_TIMING
    END_MEASURE_MERGE_ACC(args->timer) /* merge end */
#endif

    //#ifdef PERF_COUNTERS
    //    BARRIER_ARRIVE(args->barrier, rv);
    //    if(my_tid == 0) {
    //        PCM_stop();
    //        PCM_log("========== 3.1) Profiling results of Rest of the Local
    //        Merge Phase ==========\n"); PCM_printResults();
    //    }
    //#endif

    /* To check whether sorted? */

    //    check_sorted((int64_t *) mergerunsR, (int64_t *) mergerunsS,
    //    mergerunsR->num_tuples, mergerunsS->num_tuples,
    //                 my_tid);

#ifndef NO_TIMING
    BEGIN_MEASURE_JOIN_ACC(args->timer)
#endif
#ifdef JOIN
#ifdef PERF_COUNTERS
    BARRIER_ARRIVE(args->barrier, rv);
    if (my_tid == 0) {
        MSG("PCM starts..")
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
    mpass_mergejoin_phase(&mergedRelR, &mergedRelS, args);

#ifdef JOIN
#ifdef PERF_COUNTERS
    BARRIER_ARRIVE(args->barrier, rv);
    if (my_tid == 0) {
        PCM_stop();
        PCM_log("========= 4) results of Multi-pass Joining Phase =========\n");
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
        string path = EXP_DIR "/results/breakdown/time_end_" + std::to_string(args->exp_id) + ".txt";
        auto fp = fopen(path.c_str(), "w");
        fprintf(fp, "%ld\n", curtime);
        PCM_log("========= results of Overview =========\n");
        PCM_printResults();
        PCM_cleanup();

    }
#endif
    BARRIER_ARRIVE(args->barrier, rv);
#endif

#ifndef NO_TIMING
    /* wait at a barrier until each thread completes join phase */
    BARRIER_ARRIVE(args->barrier, rv)
    END_MEASURE_JOIN_ACC(args->timer)
    END_MEASURE(args->timer) /* end overall*/
#endif

    /* clean-up */
    free(mergerunsR);
    free(mergerunsS);
    mpass_partitioning_cleanup(partsR, partsS);
    free(args->threadrelchunks[my_tid]);
    /* clean-up temporary relations */
    free_threadlocal(mergedRelR.tuples, mergedRelR.num_tuples * sizeof(tuple_t));
    free_threadlocal(mergedRelS.tuples, mergedRelS.num_tuples * sizeof(tuple_t));

    return 0;
}

void mpass_partitioning_phase(relation_t ***relRparts, relation_t ***relSparts,
                              arg_t *args) {
    const int PARTFANOUT = args->joincfg->PARTFANOUT;
    const int NRADIXBITS = log2(PARTFANOUT);

    relation_t **partsR =
            (relation_t **) malloc_aligned(PARTFANOUT * sizeof(relation_t *));
    relation_t **partsS =
            (relation_t **) malloc_aligned(PARTFANOUT * sizeof(relation_t *));

    /** note: only free prels[0] when releasing memory */
    relation_t *prels =
            (relation_t *) malloc_aligned(2 * PARTFANOUT * sizeof(relation_t));
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

    /* after partitioning tmpR, tmpS holds the partitioned data */
    int bitshift = ceil(log2(relR.num_tuples * args->nthreads));
    bitshift = bitshift - NRADIXBITS - 1;

    /* printf("[INFO ] bitshift = %d\n", bitshift); */
    partition_relation_optimized(partsR, &relR, &tmpR, NRADIXBITS, bitshift);
    partition_relation_optimized(partsS, &relS, &tmpS, NRADIXBITS, bitshift);

    /** return parts */
    *relRparts = partsR;
    *relSparts = partsS;
}

void mpass_sorting_phase(relation_t **relRparts, relation_t **relSparts,
                         arg_t *args) {
    const int PARTFANOUT =
            args->joincfg->PARTFANOUT; // how many partitions to handle in one thread.

    int32_t my_tid = args->tid;

    args->threadrelchunks[my_tid] =
            (relationpair_t *) malloc_aligned(PARTFANOUT * sizeof(relationpair_t));

    uint64_t ntuples_per_part;
    uint64_t offset = 0;
    tuple_t *optr = args->tmp_sortR + my_tid * CACHELINEPADDING(1);

    for (int i = 0; i < PARTFANOUT; i++) {
        tuple_t *inptr = (relRparts[i]->tuples);
        tuple_t *outptr = (optr + offset);
        ntuples_per_part = relRparts[i]->num_tuples;
        offset += ALIGN_NUMTUPLES(ntuples_per_part);

        DEBUGMSG(1, "PART-%d-SIZE: %" PRIu64 "\n", i, relRparts[i]->num_tuples);

        if (scalarflag)
            scalarsort_tuples(&inptr, &outptr, ntuples_per_part);
        else
            avxsort_tuples(&inptr, &outptr, ntuples_per_part);

        if (!is_sorted_helper((int64_t *) outptr, ntuples_per_part)) {
            printf("===> %d-thread -> R is NOT sorted, size = %lu\n", my_tid,
                   ntuples_per_part);
        }

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

        /*
        if(!is_sorted_helper((int64_t*)outptr, ntuples_per_part)){
            printf("===> %d-thread -> S is NOT sorted, size = %d\n",
            my_tid, ntuples_per_part);
        }
        */

        args->threadrelchunks[my_tid][i].S.tuples = outptr;
        args->threadrelchunks[my_tid][i].S.num_tuples = ntuples_per_part;
        /* if(my_tid == 0) */
        /* printf("S-MYTID=%d FAN=%d OUT-START=%llu\nS-MYTID=%d FAN=%d
         * OUT-END=%llu\n", */
        /*        my_tid, i, outptr, my_tid, i, (outptr+ntuples_per_part)); */
    }
}

void mpass_firstnumamerge_phase(arg_t *args, relation_t *mergedRelR,
                                relation_t *mergedRelS, int *numrunstomerge,
                                relation_t **mergerunsR,
                                relation_t **mergerunsS) {
    const int PARTFANOUT = args->joincfg->PARTFANOUT;

    int32_t my_tid = args->tid;

    int numruns = 0;
    uint64_t mergeRtotal = 0, mergeStotal = 0;
    tuple_t *tmpoutR;
    tuple_t *tmpoutS;

    relation_t *Smergeouts =
            (relation_t *) malloc_aligned(PARTFANOUT * sizeof(relation_t));
    relation_t *Rmergeouts =
            (relation_t *) malloc_aligned(PARTFANOUT * sizeof(relation_t));

    // relation_t tmpS;
    // tmpR.tuples     = args->tmp_partR;
    // tmpR.num_tuples = args->numR;
    // tmpS.tuples     = args->tmp_partS;
    // tmpS.num_tuples = args->numS;

    if (args->nthreads == 1) {
        /* single threaded execution */
        for (int i = 0; i < PARTFANOUT; i++) {

            relationpair_t *rels = &args->threadrelchunks[my_tid][i];

            /* output offset in the first merge step */
            Rmergeouts[i].tuples = rels->R.tuples;
            Rmergeouts[i].num_tuples = rels->R.num_tuples;
            mergeRtotal += rels->R.num_tuples;

            Smergeouts[i].tuples = rels->S.tuples;
            Smergeouts[i].num_tuples = rels->S.num_tuples;
            mergeStotal += rels->S.num_tuples;
        }
        numruns = PARTFANOUT;
        tmpoutR = args->tmp_partR;
        tmpoutS = args->tmp_partS;
    } else {
        /* multi-threaded execution */
        /* merge remote relations and bring to local memory */
        const int perthread = PARTFANOUT / args->nthreads;
        const int start = my_tid * perthread;
        const int end = start + perthread;

        /* compute the size of merged relations to be stored locally */
        for (int j = start; j < end; j++) {
            for (int i = 0; i < args->nthreads; i += 2) {

                relationpair_t *rels1 = &args->threadrelchunks[i][j];
                relationpair_t *rels2 = &args->threadrelchunks[i + 1][j];

                /* if(rels1->R.num_tuples % 8 != 0 || */
                /*    rels2->R.num_tuples % 8 != 0 || */
                /*    rels1->S.num_tuples % 8 != 0 || */
                /*    rels2->S.num_tuples % 8 != 0 */
                /*    ) */
                /* { */
                /*     DEBUGMSG(1,"THERE IS A PROBLEM WITH ALIGNMENT!\n"); */
                /*     exit(0); */
                /* } */

                mergeRtotal += rels1->R.num_tuples + rels2->R.num_tuples;
                mergeStotal += rels1->S.num_tuples + rels2->S.num_tuples;
            }
        }

        /* allocate memory at local node for temporary merge results */
        tmpoutR =
                (tuple_t *) malloc_aligned_threadlocal(mergeRtotal * sizeof(tuple_t));
        tmpoutS =
                (tuple_t *) malloc_aligned_threadlocal(mergeStotal * sizeof(tuple_t));

        uint64_t offsetR = 0, offsetS = 0;
        for (int j = start; j < end; j++) {
            for (int i = 0; i < args->nthreads; i += 2) {

                // int tid1 = (((my_tid + i)/2) % (args->nthreads/2)) * 2;
                // int tid2 = tid1 + 1;
                int tid1 = (my_tid + i) % (args->nthreads);
                int tid2 = (tid1 + 1) % (args->nthreads);

                relationpair_t *rels1 = &args->threadrelchunks[tid1][j];
                relationpair_t *rels2 = &args->threadrelchunks[tid2][j];

                /* output offset in the first merge step */
                Rmergeouts[numruns].num_tuples =
                        rels1->R.num_tuples + rels2->R.num_tuples;
                Rmergeouts[numruns].tuples = tmpoutR + offsetR;
                tuple_t *Rmergeout = Rmergeouts[numruns].tuples;
                /*
            if(!is_sorted_helper((int64_t *) rels1->R.tuples, rels1->R.num_tuples))
            {
                printf("***NOT_SORTED tid=%d, part-j=%d\n", i+1, j);
            }

            if(!is_sorted_helper((int64_t *) rels2->R.tuples, rels2->R.num_tuples))
            {
                printf("***NOT_SORTED tid=%d, part-j=%d\n", i+1, j);
            }
                 */

                if (scalarflag)
                    scalar_merge_tuples(rels1->R.tuples, rels2->R.tuples, Rmergeout,
                                        rels1->R.num_tuples, rels2->R.num_tuples);
                else
                    avx_merge_tuples(rels1->R.tuples, rels2->R.tuples, Rmergeout,
                                     rels1->R.num_tuples, rels2->R.num_tuples);

                /* if(Rmergeouts[cnt].num_tuples > 0 && !is_sorted_helper(Rmergeout,
                 * Rmergeouts[cnt].num_tuples)) { */
                /*     printf("===> %d-thread -> MERGE-R is NOT sorted, size = %d\n",
                 * my_tid, Rmergeouts[cnt].num_tuples); */
                /*     printf("===> lenA=%d, lenB=%d\n", rels1->R.num_tuples,
                 * rels2->R.num_tuples); */

                /* } */

                /* if(!is_sorted_helper(Rmergeout, Rmergeouts[cnt].num_tuples)) { */
                /*     DEBUGMSG(1,"===> %d-thread -> R is NOT sorted, size = %d --
                 * R1=%d, R2=%d\n",  */
                /*              my_tid, Rmergeouts[cnt].num_tuples,  */
                /*              rels1->R.num_tuples, */
                /*              rels2->R.num_tuples); */
                /*     char fn[64]; */
                /*     sprintf(fn,"merge-tid%d.log", my_tid); */
                /*     FILE * log = fopen(fn, "w"); */
                /*     fprintf(log, "==== Input 1 ====\n"); */
                /*     for(int m = 0; m < rels1->R.num_tuples; m++){ */
                /*         fprintf(log, "%llu\n", rels1->R.tuples[m].key); */
                /*     } */
                /*     fprintf(log, "==== Input 2 ====\n"); */
                /*     for(int m = 0; m < rels2->R.num_tuples; m++){ */
                /*         fprintf(log, "%llu\n", rels2->R.tuples[m].key); */
                /*     } */
                /*     fprintf(log, "==== Merge-output ====\n"); */
                /*     for(int m = 0; m < Rmergeouts[cnt].num_tuples; m++){ */
                /*         fprintf(log, "%llu\n", Rmergeout[m]); */
                /*     } */
                /*     fclose(log); */
                /*     exit(0); */
                /* } */

                Smergeouts[numruns].num_tuples =
                        rels1->S.num_tuples + rels2->S.num_tuples;
                Smergeouts[numruns].tuples = tmpoutS + offsetS;
                tuple_t *Smergeout = Smergeouts[numruns].tuples;

                /* if(!is_sorted_helper((int64_t *) rels1->S.tuples,
                 * rels1->S.num_tuples)) */
                /* { */
                /*     DEBUGMSG(1,"***NOT_SORTED tid=%d, part-j=%d\n", i+1, j); */
                /* } */

                /* if(!is_sorted_helper((int64_t *) rels2->S.tuples,
                 * rels2->S.num_tuples)) */
                /* { */
                /*     DEBUGMSG(1,"***NOT_SORTED tid=%d, part-j=%d\n", i+1, j); */
                /* } */

                if (scalarflag)
                    scalar_merge_tuples(rels1->S.tuples, rels2->S.tuples, Smergeout,
                                        rels1->S.num_tuples, rels2->S.num_tuples);
                else
                    avx_merge_tuples(rels1->S.tuples, rels2->S.tuples, Smergeout,
                                     rels1->S.num_tuples, rels2->S.num_tuples);

                /*
            if(!is_sorted_helper(Rmergeout, Rmergeouts[cnt].num_tuples))
                printf("===> %d-thread -> R is NOT sorted, size = %d\n", my_tid,
                       Rmergeouts[cnt].num_tuples);

            if(!is_sorted_helper(Smergeout, Smergeouts[cnt].num_tuples))
                printf("===> %d-thread -> S is NOT sorted, size = %d\n",
                         my_tid, Smergeouts[cnt].num_tuples);
                 */

                offsetR += Rmergeouts[numruns].num_tuples;
                offsetS += Smergeouts[numruns].num_tuples;

                numruns++;
            }
        }
    }

    /** returned merged relations */
    mergedRelR->tuples = tmpoutR;
    mergedRelR->num_tuples = mergeRtotal;
    mergedRelS->tuples = tmpoutS;
    mergedRelS->num_tuples = mergeStotal;

    /** merge runs */
    *numrunstomerge = numruns;
    *mergerunsR = Rmergeouts;
    *mergerunsS = Smergeouts;
}

void mpass_fullmultipassmerge_phase(arg_t *args, int numrunstomerge,
                                    relation_t *mergerunsR,
                                    relation_t *mergerunsS,
                                    relation_t *mergedRelR,
                                    relation_t *mergedRelS) {
    int32_t my_tid = args->tid;

    /* Full-merge of relation R */
    tuple_t *tmpoutR = mergedRelR->tuples;
    tuple_t *tmpoutR2 = (tuple_t *) malloc_aligned_threadlocal(
            mergedRelR->num_tuples * sizeof(tuple_t));
    tuple_t *reclaimR2 = tmpoutR2; /* for releasing */
    int cnt = numrunstomerge;
    for (; cnt > 1; cnt >>= 1) {
        uint64_t offsetR = 0;
        for (int i = 0, j = 0; i < cnt; i += 2) {
            tuple_t *inpA = (mergerunsR[i].tuples);
            tuple_t *inpB = (mergerunsR[i + 1].tuples);
            tuple_t *out = (tmpoutR2 + offsetR);

            uint64_t len1 = mergerunsR[i].num_tuples;
            uint64_t len2 = mergerunsR[i + 1].num_tuples;

            if (scalarflag)
                scalar_merge_tuples(inpA, inpB, out, len1, len2);
            else
                avx_merge_tuples(inpA, inpB, out, len1, len2);

            mergerunsR[j].tuples = out;
            mergerunsR[j].num_tuples = len1 + len2;
            offsetR += mergerunsR[j].num_tuples;
            j++;
        }
        tuple_t *swap = (tuple_t *) tmpoutR;
        tmpoutR = (tuple_t *) tmpoutR2;
        tmpoutR2 = (tuple_t *) swap;
    }
    /* clean-up tempoutR2 */
    if (reclaimR2 == tmpoutR2) {
        free_threadlocal(reclaimR2, mergedRelR->num_tuples * sizeof(tuple_t));
    } else if (my_tid == 0 && args->nthreads > 1) {
        free(tmpoutR2);
    }

    /* Full-merge of relation S */
    tuple_t *tmpoutS = mergedRelS->tuples;
    tuple_t *tmpoutS2 = (tuple_t *) malloc_aligned_threadlocal(
            mergedRelS->num_tuples * sizeof(tuple_t));
    tuple_t *reclaimS2 = tmpoutS2; /* for releasing */
    cnt = numrunstomerge;
    for (; cnt > 1; cnt >>= 1) {
        uint64_t offsetS = 0;
        for (int i = 0, j = 0; i < cnt; i += 2) {
            tuple_t *inpA = (mergerunsS[i].tuples);
            tuple_t *inpB = (mergerunsS[i + 1].tuples);
            tuple_t *out = (tmpoutS2 + offsetS);

            uint64_t len1 = mergerunsS[i].num_tuples;
            uint64_t len2 = mergerunsS[i + 1].num_tuples;

            if (scalarflag)
                scalar_merge_tuples(inpA, inpB, out, len1, len2);
            else
                avx_merge_tuples(inpA, inpB, out, len1, len2);

            mergerunsS[j].tuples = out;
            mergerunsS[j].num_tuples = len1 + len2;
            offsetS += mergerunsS[j].num_tuples;
            j++;
        }
        tuple_t *swap = (tuple_t *) tmpoutS;
        tmpoutS = (tuple_t *) tmpoutS2;
        tmpoutS2 = (tuple_t *) swap;
    }
    /* clean-up tempoutS2 */
    if (reclaimS2 == tmpoutS2) {
        free_threadlocal(reclaimS2, mergedRelS->num_tuples * sizeof(tuple_t));
    } else if (my_tid == 0 && args->nthreads > 1) {
        free(tmpoutS2);
    }

    /** return completely merged relations */
    mergedRelR->tuples = tmpoutR;
    mergedRelS->tuples = tmpoutS;
}

void mpass_mergejoin_phase(relation_t *mergedRelR, relation_t *mergedRelS,
                           arg_t *args) {
    /* Merge Join */
    tuple_t *rtuples = (tuple_t *) mergedRelR->tuples;
    tuple_t *stuples = (tuple_t *) mergedRelS->tuples;

#ifdef JOIN_MATERIALIZE
    chainedtuplebuffer_t *chainedbuf = chainedtuplebuffer_init();
#else
    void *chainedbuf = NULL;
#endif

    uint64_t nresults =
            merge_join(rtuples, stuples, mergedRelR->num_tuples,
                       mergedRelS->num_tuples, chainedbuf, args->timer);

    args->result = nresults;

    /* printf("TID=%d --> #res = %d %d\n", my_tid, args->result, nresults); */

#ifdef JOIN_MATERIALIZE
    args->threadresult->nresults = nresults;
    args->threadresult->threadid = args->tid;
    args->threadresult->results = (void *)chainedbuf;
#endif
}
