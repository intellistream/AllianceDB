/**
 * @file    sortmergejoin_multiway_skewhandling.c
 * @author  Cagri Balkesen <cagri.balkesen@inf.ethz.ch>
 * @date    Sat Dec 15 15:37:54 2012
 * @version $Id $
 *
 * @brief   m-way+skew sort-merge-join algorithm with multi-way merging + skew handling.
 *          It uses AVX-based sorting and merging if scalarsort and scalarmerge
 *          flags are not provided.
 *
 *
 * (c) 2012-2014, ETH Zurich, Systems Group
 *
 * \ingroup Joins
 */

#include <stdlib.h> /* malloc() */
#include <math.h>   /* log2(), ceil() */

#include "../util/rdtsc.h"              /* startTimer, stopTimer */
#include "../util/barrier.h"            /* pthread_barrier_* */
#include "../affinity/cpu_mapping.h"        /* cpu_id NUMA related methods */

#include "sortmergejoin_multiway.h" /* partitioning&sorting phases are same. */
#include "sortmergejoin_multiway_skewhandling.h"
#include "joincommon.h"
#include "partition.h"  /* partition_relation_optimized() */
#include "avxsort.h"    /* avxsort_tuples() */
#include "scalarsort.h" /* scalarsort_tuples() */
#include "avx_multiwaymerge.h"         /* avx_multiway_merge() */
#include "scalar_multiwaymerge.h"      /* scalar_multiway_merge() */
#include "../affinity/memalloc.h"           /* malloc_aligned() */
#include "../affinity/numa_shuffle.h"       /* get_numa_shuffle_strategy() */

#ifdef JOIN_MATERIALIZE
#include "tuple_buffer.h"
#endif

/** For skew handling code that has C++ STL usage. */
#include <iostream>
#include <map>
#include <algorithm>

#include <string.h> /* memcpy() */

/** This is a struct used for representing merge tasks when skew handling
    mechanism is enabled. Essentially, large merge tasks are decomposed into
    smaller merge tasks and placed into a task queue. */
struct mergetask_t {
    tuple_t * output;
    relation_t ** runstomerge;
    int numruns;
    unsigned int totaltuples;
    /* if heavy-hitter then not merged, directly copied */
    int isheavyhitter;
};
typedef struct mergetask_t mergetask_t;

/**
 * Main thread of First Sort-Merge Join variant with partitioning and complete
 * sorting of both input relations. The merging step in this algorithm tries to
 * overlap the first merging and transfer of remote chunks. However, in compared
 * to other variants, merge phase still takes a significant amount of time.
 *
 * @param param parameters of the thread, see arg_t for details.
 *
 */
void *
sortmergejoin_multiway_skewhandling_thread(void * param);

result_t *
sortmergejoin_multiway_skewhandling(relation_t * relR, relation_t * relS,
                                    joinconfig_t * joincfg)
{
    /* check whether nr. of threads is a power of 2 */
    if((joincfg->NTHREADS & (joincfg->NTHREADS-1)) != 0){
        fprintf(stdout, "[ERROR] m-way sort-merge join runs with "\
                "a power of 2 #threads.\n");
        return 0;
    }

    return sortmergejoin_initrun(relR, relS, joincfg,
                                 sortmergejoin_multiway_skewhandling_thread);
}


/**
 * NUMA-local partitioning of both input relations into PARTFANOUT.
 *
 * @param[out] relRparts partitions of relation R
 * @param[out] relSparts partitions of relation S
 * @param[in] args thread arguments
 */
extern void
partitioning_phase(relation_t *** relRparts, relation_t *** relSparts,
                   arg_t * args);

extern void
partitioning_cleanup(relation_t ** relRparts, relation_t ** relSparts);

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
extern void
sorting_phase(relation_t ** relRparts, relation_t ** relSparts, arg_t * args);


/**
 * Multi-way merging of sorted NUMA-runs with in-cache resident buffers.
 * + Fine-grained skew handling mechanisms for handling skewed outer relations.
 *
 * @param[in] numaregionid NUMA region id of the executing thread
 * @param[out] relRparts sorted partitions of relation R for joining when nthreads=1
 * @param[out] relSparts sorted partitions of relation S for joining when nthreads=1
 * @param[in] args thread arguments
 * @param[out] mergedRelR completely merged relation R
 * @param[out] mergedRelS completely merged relation S
 */
void
multiwaymerge_phase_withskewhandling(int numaregionid,
                                     relation_t ** relRparts, relation_t ** relSparts, arg_t * args,
                                     relation_t * mergedRelR, relation_t * mergedRelS);

/**
 * Evaluate the merge-join over NUMA-local sorted runs.
 *
 * @param[in] relRparts sorted parts of NUMA-local relation-R if nthr=1
 * @param[in] relSparts sorted parts of NUMA-local relation-S if nthr=1
 * @param[in] mergedRelR sorted entire NUMA-local relation-R if nthr>1
 * @param[in] mergedRelS sorted entire NUMA-local relation-S if nthr>1
 * @param[in,out] args return values are stored in args
 */
extern void
mergejoin_phase(relation_t ** relRparts, relation_t ** relSparts,
                relation_t * mergedRelR, relation_t * mergedRelS, arg_t * args);

/**
 * Balanced partition of n sorted lists using equi-depth histograms
 * and combined CDF over them. Returns num X intkey_t,
 * which is a list of partitioning keys for range values of each thread.
 */
void
balanced_sorted_partition(relation_t ** rels, int nrels, int eqhistsamples,
                          intkey_t * partkeys /* [out] */, int nparts,
                          int my_tid, relation_t ** heavyhitters);


/**
 * Create and add merge tasks to the merge task queue. Also, return
 * the largest merge task without adding to the queue. Merge tasks are
 * identified by the splitter keys found by balanced_sorted_partition().
 */
void
create_and_add_merge_tasks(taskqueue_t * mergequeue,
                           mergetask_t ** largest_ret,
                           intkey_t * merge_splitters,
                           relation_t ** relSparts,
                           tuple_t * outputptr,
                           const int ntasks,
                           const int PARTFANOUT);

/**
 * Detects the heavy hitters and creates specialized direct copy tasks for
 * those instead of merging them.
 */
void
detect_heavy_hitters_and_add_tasks(relation_t * heavyhits,
                                   taskqueue_t * mergequeue,
                                   mergetask_t * largest_task,
                                   int my_tid,
                                   const int PARTFANOUT);

/**
 * Find the first offset in sortedrun which is >= skey.
 */
int64_t
binsearch_lower_bound(tuple_t * sortedrun, int64_t ntups, intkey_t skey);

/**
 * Find the maximum offset in sortedrun which is < skey.
 */
int64_t
binsearch_upper_bound(tuple_t * sortedrun, int64_t ntup,  intkey_t skey);

/**
 * Determines and returns at most SKEW_MAX_HEAVY_HITTERS heavy hitter keys
 * from an equi-depth histogram. Heavy hitter is a key that occurs
 * above given threshold (SKEW_HEAVY_HITTER_THR) percent of the buckets in the
 * histogram.
 */
void
find_heavy_hitters(relation_t * equidepthhist, double thrpercent,
                   relation_t * heavyhits);

/**
 * Create and return a new mergetask_t structure with a given # of merge runs.
 */
mergetask_t *
create_mergetask(int nruns);

void
print_mergetask(mergetask_t * tsk);

/**
 * Release memory for the mergetask_t struct.
 */
void
free_mergetask(mergetask_t * mt);

/**
 * This method copies given relation tuples to the output array one by
 * one. Used for copying out heavy hitters. There is an optimization
 * opportunity to use AVX/SSE copy.
 */
void
do_fast_memcpy(mergetask_t * mwtask);

/**
 * Efficient memcpy operation using non-temporal load/stores.
 */
void
nt_memcpy(void * dst, void * src, size_t sz);


/**
 * Main execution thread of "m-way" sort-merge join.
 *
 * @param param
 */
void *
sortmergejoin_multiway_skewhandling_thread(void * param)
{
    arg_t * args   = (arg_t*) param;
    int32_t my_tid = args->my_tid;
    int rv;

    DEBUGMSG(1, "Thread-%d started running ... \n", my_tid);
#ifdef PERF_COUNTERS
    if(my_tid == 0){
        PCM_initPerformanceMonitor(NULL, NULL);
        PCM_start();
    }
    BARRIER_ARRIVE(args->barrier, rv);
#endif

    BARRIER_ARRIVE(args->barrier, rv);
    if(my_tid == 0) {
        gettimeofday(&args->start, NULL);
        startTimer(&args->part);
        startTimer(&args->sort);
        startTimer(&args->mergedelta);
        startTimer(&args->merge);
        startTimer(&args->join);
    }


    /*************************************************************************
     *
     *   Phase.1) NUMA-local partitioning.
     *
     *************************************************************************/
    relation_t ** partsR = NULL;
    relation_t ** partsS = NULL;
    partitioning_phase(&partsR, &partsS, args);


#ifdef PERF_COUNTERS
    BARRIER_ARRIVE(args->barrier, rv);
    if(my_tid == 0) {
        PCM_stop();
        PCM_log("========= 1) Profiling results of Partitioning Phase =========\n");
        PCM_printResults();
    }
#endif

    BARRIER_ARRIVE(args->barrier, rv);
    if(my_tid == 0) {
        stopTimer(&args->part);
    }

#ifdef PERF_COUNTERS
    if(my_tid == 0){
        PCM_start();
    }
    BARRIER_ARRIVE(args->barrier, rv);
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
    if(is_first_thread_in_numa_region(my_tid)) {
        /* TODO: make buffer size runtime parameter */
        tuple_t * sharedmergebuffer = (tuple_t *)
                malloc_aligned(args->joincfg->MWAYMERGEBUFFERSIZE);
        args->sharedmergebuffer[numaregionid] = sharedmergebuffer;

        DEBUGMSG(1, "Thread-%d allocated %.3lf KiB merge buffer in "\
                 "NUMA-region-%d to be used by %d active threads.\n",
                 my_tid, (double)(args->joincfg->MWAYMERGEBUFFERSIZE/1024.0),
                 numaregionid, get_num_active_threads_in_numa(numaregionid));

#ifdef SKEW_HANDLING
        /* initialize skew handling ; mwaytask_t taskqueues */
        args->numa_taskqueues[numaregionid] = taskqueue_init(32);
#endif
    }

#ifdef PERF_COUNTERS
        BARRIER_ARRIVE(args->barrier, rv);
    if(my_tid == 0) {
        PCM_stop();
        PCM_log("========= 2) Profiling results of Sorting Phase =========\n");
        PCM_printResults();
    }
#endif

    BARRIER_ARRIVE(args->barrier, rv);
    if(my_tid == 0) {
        stopTimer(&args->sort);
    }
    /* check whether local relations are sorted? */
#if 0
    {
#include <string.h> /* memcpy() */
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

#ifdef PERF_COUNTERS
    if(my_tid == 0){
        PCM_start();
    }
    BARRIER_ARRIVE(args->barrier, rv);
#endif


    /*************************************************************************
     *
     *   Phase.3) Apply multi-way merging with in-cache resident buffers +
     *   Detect skew & handle skew in a fine-grained manner for a robust perf.
     *
     *************************************************************************/
    relation_t mergedRelR;
    relation_t mergedRelS;
    multiwaymerge_phase_withskewhandling(numaregionid, partsR, partsS, args,
                                         &mergedRelR, &mergedRelS);


    BARRIER_ARRIVE(args->barrier, rv);
    if(my_tid == 0) {
        stopTimer(&args->mergedelta);
        args->merge = args->mergedelta; /* since we do merge in single go. */
        DEBUGMSG(1, "Multi-way merge is complete!\n");
    }
    /* the thread that allocated the merge buffer releases it. */
    if(is_first_thread_in_numa_region(my_tid)) {
        free(args->sharedmergebuffer[numaregionid]);
        //free_threadlocal(args->sharedmergebuffer[numaregionid],
        //MWAY_MERGE_BUFFER_SIZE);
#ifdef SKEW_HANDLING
        /* initialize skew handling ; mwaytask_t taskqueues */
        taskqueue_free(args->numa_taskqueues[numaregionid]);
#endif
    }

#ifdef PERF_COUNTERS
    BARRIER_ARRIVE(args->barrier, rv);
    if(my_tid == 0) {
        PCM_stop();
        PCM_log("========= 3) Profiling results of Multi-Way NUMA-Merge Phase =========\n");
        PCM_printResults();
        PCM_cleanup();
    }
    BARRIER_ARRIVE(args->barrier, rv);
#endif


    /* To check whether sorted? */
    /*
    check_sorted((int64_t *)tmpoutR, (int64_t *)tmpoutS,
                 mergeRtotal, mergeStotal, my_tid);
     */

    /*************************************************************************
     *
     *   Phase.4) NUMA-local merge-join on local sorted runs.
     *
     *************************************************************************/
    mergejoin_phase(partsR, partsS, &mergedRelR, &mergedRelS, args);

    /* for proper timing */
    BARRIER_ARRIVE(args->barrier, rv);
    if(my_tid == 0) {
        stopTimer(&args->join);
        gettimeofday(&args->end, NULL);
    }

    /* clean-up */
    partitioning_cleanup(partsR, partsS);

    free(args->threadrelchunks[my_tid]);
    /* clean-up temporary relations */
    if(args->nthreads > 1){
        free_threadlocal(mergedRelR.tuples, mergedRelR.num_tuples * sizeof(tuple_t));
        free_threadlocal(mergedRelS.tuples, mergedRelS.num_tuples * sizeof(tuple_t));
    }


    return 0;
}


void
multiwaymerge_phase_withskewhandling(int numaregionid,
                                     relation_t ** relRparts, relation_t ** relSparts, arg_t * args,
                                     relation_t * mergedRelR, relation_t * mergedRelS)
{
    const int PARTFANOUT = args->joincfg->PARTFANOUT;
    const int scalarmergeflag = args->joincfg->SCALARMERGE;

    int32_t my_tid = args->my_tid;
    uint64_t mergeRtotal = 0, mergeStotal = 0;
    tuple_t * tmpoutR = NULL;
    tuple_t * tmpoutS = NULL;

    if(args->nthreads == 1) {
        /* single threaded execution; no multi-way merge. */
        for(int i = 0; i < PARTFANOUT; i ++) {
            relationpair_t * rels = & args->threadrelchunks[my_tid][i];
            mergeRtotal += rels->R.num_tuples;
            mergeStotal += rels->S.num_tuples;

            /* evaluate join between each sorted part */
            relRparts[i]->tuples = rels->R.tuples;
            relRparts[i]->num_tuples = rels->R.num_tuples;
            relSparts[i]->tuples = rels->S.tuples;
            relSparts[i]->num_tuples = rels->S.num_tuples;
        }
    }
    else
    {
        uint32_t       j;
        const uint32_t perthread   = PARTFANOUT / args->nthreads;

        /* multi-threaded execution */
        /* merge remote relations and bring to local memory */
        const uint32_t start = my_tid * perthread;
        const uint32_t end = start + perthread;

        relation_t * Rparts[PARTFANOUT];
        relation_t * Sparts[PARTFANOUT];

        /* compute the size of merged relations to be stored locally */
        uint32_t f = 0;
        for(j = start; j < end; j ++) {
            for(int i = 0; i < args->nthreads; i ++) {
                //uint32_t tid = (my_tid + i) % args->nthreads;
                uint32_t tid = get_numa_shuffle_strategy(my_tid, i, args->nthreads);
                //printf("SHUF %d %d --> %d\n", i, my_tid, tid);
                relationpair_t * rels = & args->threadrelchunks[tid][j];
                //fprintf(stdout, "TID=%d Part-%d-size = %d\n", my_tid, f, rels->S.num_tuples);
                Rparts[f] = & rels->R;
                Sparts[f] = & rels->S;
                f++;

                mergeRtotal += rels->R.num_tuples;
                mergeStotal += rels->S.num_tuples;
            }
        }

        /* allocate memory at local node for temporary merge results */
        tmpoutR = (tuple_t *) malloc_aligned_threadlocal(mergeRtotal*sizeof(tuple_t));
        tmpoutS = (tuple_t *) malloc_aligned_threadlocal(mergeStotal*sizeof(tuple_t));

        /** returned merged relations, only if nthreads > 1 */
        mergedRelR->tuples = tmpoutR;
        mergedRelR->num_tuples = mergeRtotal;
        mergedRelS->tuples = tmpoutS;
        mergedRelS->num_tuples = mergeStotal;

        /* determine the L3 cache-size per thread */
        /* int nnuma = get_num_numa_regions(); */

        /* active number of threads in the current NUMA-region: */
        int active_nthreads_in_numa = get_num_active_threads_in_numa(numaregionid);

        /* index of the current thread in its NUMA-region: */
        int numatidx = get_thread_index_in_numa(my_tid);

        /* get the exclusive part of the merge buffer for the current thread */
        int bufsz_thr = (args->joincfg->MWAYMERGEBUFFERSIZE/active_nthreads_in_numa)
                        / sizeof(tuple_t);
        tuple_t * mergebuf = args->sharedmergebuffer[numaregionid]
                             + (numatidx * bufsz_thr);


#ifdef SKEW_HANDLING
        /* We decompose the merge tasks into number of merge tasks and
           add them to a task queue if the total merge size > expected
           average merge size + some threashold(currently 10%). Task
           queue is local to NUMA-region. TODO: we should also consider task
           stealing among NUMA-regions as an extension. We also
           consider skew only in S for the time being.
        */
        taskqueue_t * mergetaskqueue = args->numa_taskqueues[numaregionid];
        uint64_t DECOMPOSE_THRESHOLD = args->numS * SKEW_DECOMPOSE_MARGIN;
        int rv;
        if(mergeStotal > DECOMPOSE_THRESHOLD)
        {
            /* 1) Break-down the merge task and add to a task queue;
               first determine how many merge tasks we will create. */
            int nmergetasks = (mergeStotal + DECOMPOSE_THRESHOLD - 1)
                              / DECOMPOSE_THRESHOLD;
            intkey_t * merge_splitters = (intkey_t *)
                    malloc(sizeof(intkey_t) * nmergetasks);
            relation_t * heavyhits = 0;
            /* 2) Find splitters for range part. to different merge tasks.*/
            balanced_sorted_partition(Sparts,PARTFANOUT,SKEW_DECOMPOSE_SAMPLES,
                                      merge_splitters, nmergetasks, my_tid,
                                      &heavyhits);

            /* 3) Create and add merge tasks to the merge task queue. Also,
               return the largest merge task without adding to the queue. */
            mergetask_t * largest_task = 0;
            create_and_add_merge_tasks(mergetaskqueue,
                                       &largest_task,
                                       merge_splitters,
                                       Sparts,
                                       tmpoutS,
                                       nmergetasks,
                                       PARTFANOUT);
            free(merge_splitters);

            /* 5) Check if heavy hitters exist in the largest task, if yes, then
               find the boundaries of the heavy hitters and create specialized
               'direct copy' tasks for those. */
            detect_heavy_hitters_and_add_tasks(heavyhits,
                                               mergetaskqueue,
                                               largest_task,
                                               my_tid,
                                               PARTFANOUT);
            free(heavyhits->tuples);
            free(heavyhits);

        }
        else
#endif
        {
            /* Just add the merge tasks for rel-S to the merge task queue. */
            mergetask_t * mwtask = create_mergetask(PARTFANOUT);
            for(int p = 0; p < PARTFANOUT; p++){
                mwtask->runstomerge[p] = Sparts[p];
            }
            mwtask->totaltuples = mergeStotal;
            mwtask->output = tmpoutS;
            DEBUGMSG(1, "[---- TID=%d adding merge task with %d tuples.\n",
                     my_tid, mwtask->totaltuples);
            taskqueue_addfront(mergetaskqueue, mwtask);
        }
        /*** Wait until all threads complete skew handling logic. ***/
        BARRIER_ARRIVE(args->barrier, rv);

        /* Now do the multi-way merging for the relation R. */
        if(scalarmergeflag){
            scalar_multiway_merge(tmpoutR, Rparts, PARTFANOUT,
                                  mergebuf, bufsz_thr);
        }
        else {
            avx_multiway_merge(tmpoutR, Rparts, PARTFANOUT,
                               mergebuf, bufsz_thr);
        }

#ifdef SKEW_HANDLING
        /** 6) Grab merge tasks from the task queue and merge as usual or
            directly copy if it is a heavy-hitter. */
        mergetask_t * mwtask = 0;
        while((mwtask = (mergetask_t*)taskqueue_getfront(mergetaskqueue)))
        {
            DEBUGMSG(1, " TID=%d GOT TASK = %d tuples; nruns=%d, is-HH=%d \n",
                     my_tid, mwtask->totaltuples,
                     mwtask->numruns, mwtask->isheavyhitter);

            if(mwtask->totaltuples > 0)
            {

                if(mwtask->isheavyhitter){
                    /* if heavy-hitter then not merged, directly copied */
                    do_fast_memcpy(mwtask);
#if DEBUG_SORT_CHECK
                    for(int mm = 0; mm < mwtask->numruns; mm++)
                    {
                        if( mwtask->runstomerge[mm]->num_tuples > 0 )
                        {
                            if(is_sorted_helper(
                                    (int64_t*)mwtask->runstomerge[mm]->tuples,
                                    mwtask->runstomerge[mm]->num_tuples))
                                printf("[INFO ] HH-RUN[%d] -> sorted, size = %d\n",
                                       mm, mwtask->runstomerge[mm]->num_tuples);
                            else
                                printf("[ERROR] HH-RUN[%d] -> NOT sorted, size = %d\n",
                                       mm, mwtask->runstomerge[mm]->num_tuples);
                        }
                    }
#endif
                }
                else {
                    /* Now do the multi-way merging */
                    if(scalarmergeflag)
                    {
                        /* do normal scalar multi-way merge */
                        scalar_multiway_merge(mwtask->output,
                                              mwtask->runstomerge,
                                              mwtask->numruns,
                                              mergebuf, bufsz_thr);
                    }
                    else {
                        avx_multiway_merge(mwtask->output,
                                           mwtask->runstomerge,
                                           mwtask->numruns,
                                           mergebuf, bufsz_thr);
                    }
                }

#if DEBUG_SORT_CHECK
                if(is_sorted_helper((int64_t*)mwtask->output, mwtask->totaltuples))
                    printf("[INFO ] %d-thread -> sorted, size = %d\n",
                           my_tid, mwtask->totaltuples);
                else
                    printf("[ERROR] %d-thread -> NOT sorted, size = %d\n",
                           my_tid, mwtask->totaltuples);
                DEBUGMSG(1, " TASK-OUTPUT = %p -- size = %d -- tmpoutS = %p ; off = %d\n",
                         mwtask->output, mwtask->totaltuples, tmpoutS,
                         (mwtask->output - tmpoutS));
                DEBUGMSG(1, "***** FIRST = %d -- LAST = %d \n",
                         mwtask->output[0].key,
                         mwtask->output[mwtask->totaltuples-1].key);
#endif

            }
            /* clean-up. */
            free_mergetask(mwtask);
        }
#endif

    }

}

/**
 * Balanced partition of n sorted lists using equi-depth histograms
 * and combined CDF over them. Returns num X intkey_t,
 * which is a list of partitioning keys for ranges of each thread.
 */
void
balanced_sorted_partition(relation_t ** rels, int nrels, int eqhistsamples,
                          intkey_t * partkeys /* [out] */, int nparts, int my_tid,
                          relation_t ** heavyhitters)
{
    /* compute equi-depth histogram for each chunk with p=eqhistsamples values */
    relation_t eqhist[nrels];
    tuple_t tmp[nrels * eqhistsamples];
    int64_t total = 0;
    for(int i = 0; i < nrels; i++){
        eqhist[i].num_tuples = eqhistsamples;
        eqhist[i].tuples     = (tuple_t*)(tmp + i * eqhistsamples);
    }

    /* fill in the equi-hist values from each chunk */
    for(int i = 0; i < nrels; i++){
        int eqwidth = rels[i]->num_tuples / eqhistsamples;
        total += rels[i]->num_tuples;
        for(int p = eqhistsamples-1; p >= 0; p--){
            uint64_t  idx               = (p+1) * eqwidth - 1;
            tuple_t * tup               = &(rels[i]->tuples[idx]);
            eqhist[i].tuples[p].key     = tup->key;
            eqhist[i].tuples[p].payload = eqwidth;
        }
    }

    /* find heavy hitters */
    relation_t * heavyhits = (relation_t *) malloc(sizeof(relation_t));
    heavyhits->tuples = (tuple_t*)malloc(sizeof(tuple_t)*SKEW_MAX_HEAVY_HITTERS);
    heavyhits->num_tuples = 0;

    for(int i = 0; i < nrels; i++){
        find_heavy_hitters(&eqhist[i], SKEW_HEAVY_HITTER_THR, heavyhits);
    }
    std::sort((int64_t*)(heavyhits->tuples), (int64_t*)(heavyhits->tuples + heavyhits->num_tuples));
    // for(int h = 0; h < heavyhits->num_tuples; h++){
    //     printf("===> TID = %d, HEAVY-HITTER-%d = %d\n", my_tid, h, heavyhits->tuples[h].key);
    // }


    /* merge the histograms, new size can be up to (eqhistsamples * nrels) */
    std::map<intkey_t, int> cdfhist;
    for(int i = 0; i < nrels; i++){
        //if(my_tid == 3) printf("****************** HIST for %d\n", i);
        for(int p = eqhistsamples-1; p >= 0; p--){
            intkey_t key          = eqhist[i].tuples[p].key;
            int      count        = eqhist[i].tuples[p].payload;
            if(cdfhist.find(key) != cdfhist.end()){
                cdfhist[key]     += count;
            }
            else
                cdfhist[key] = count;
            //if(my_tid == 3) printf("KEY=%d ; COUNT=%d\n", key,count);
        }
    }

    /* compute a prefix-sum/cdf over the merged histograms */
    relation_t prefixcdf;
    prefixcdf.num_tuples = cdfhist.size();
    prefixcdf.tuples     = (tuple_t *) malloc(sizeof(tuple_t) * cdfhist.size());

    int sum = 0;
    int i = 0;
    for (std::map<intkey_t,int>::iterator it=cdfhist.begin();
         it != cdfhist.end(); ++it) {
        sum                         += it->second;
        prefixcdf.tuples[i].key      = it->first;
        prefixcdf.tuples[i].payload  = sum;
        i++;
    }

    // char fn[512];
    // sprintf(fn, "cdf-%d.tbl", my_tid);
    // write_relation(&prefixcdf, fn);

    // std::cout << "#KEY; COUNT\n";
    // for (i = 0; i < prefixcdf.num_tuples; i++)
    //     std::cout << prefixcdf.tuples[i].key << " "
    //               << prefixcdf.tuples[i].payload << std::endl;

    /* find partition boundaries */
    unsigned int j = 0;
    int64_t numperpart = total / nparts;

    for(i = 0; i < nparts-1; i++) {
        value_t count = (i+1) * numperpart;
        for(; j < prefixcdf.num_tuples; j++) {
            if(prefixcdf.tuples[j].payload == count){
                partkeys[i] = prefixcdf.tuples[j].key;
                // fprintf(stderr, "TASK-%d partition-key = %d\n",
                //         i, partkeys[i]);
                break;
            }
            else if(prefixcdf.tuples[j].payload > count){
                int cnt2 = prefixcdf.tuples[j].payload;
                int key2 = prefixcdf.tuples[j].key;
                int cnt1 = 0;
                int key1 = 0;

                if( j > 0 ) {
                    cnt1 = prefixcdf.tuples[j-1].payload;
                    key1 = prefixcdf.tuples[j-1].key;
                }

                double factor = (count - cnt1) / (double)(cnt2 - cnt1);
                /* find the partitioning key with interpolation */
                intkey_t partkey = (key2 - key1) * factor + key1;
                partkeys[i] = partkey;
                // fprintf(stderr, "TASK-%d partition-key = %d\n",
                //         i, partkey);
                break;
            }
#if 0
            else if(j == (prefixcdf.num_tuples-1)) {
                /* We can find a possible value with interpolation. */
                int cnt2 = prefixcdf.tuples[j].payload;
                int key2 = prefixcdf.tuples[j].key;
                int cnt1 = prefixcdf.tuples[j-1].payload;
                int key1 = prefixcdf.tuples[j-1].key;
                /* MAX = k2 + ( (k2-k1) * (Total-c2) ) / (c2-c1) */
                int maxkey = key2 + ((key2 - key1) * (total - cnt2))
                    / (double)(cnt2 - cnt1);
                /* find the partitioning key with interpolation */
                double factor = (count - cnt2) / (double)(total - cnt2);
                intkey_t partkey = key2 + (maxkey - key2) * factor;
                partkeys[i] = partkey;
                break;
            }
#endif
        }
    }

    free(prefixcdf.tuples);

    /* return heavyhitters */
    *heavyhitters = heavyhits;

}

void create_and_add_merge_tasks(taskqueue_t * mergequeue,
                                mergetask_t ** largest_ret,
                                intkey_t * merge_splitters,
                                relation_t ** relSparts,
                                tuple_t * outputptr,
                                const int ntasks,
                                const int PARTFANOUT)
{
    /** keep track of the largest mway task */
    mergetask_t * largest = 0;
    int my_tid = -1;
    int64_t startoffset[PARTFANOUT];
    uint64_t runsizes[PARTFANOUT];
    uint64_t task0tot = 0;
    {
        largest = create_mergetask(PARTFANOUT);
        /* t = 0 --> first task is directly assigned */
        intkey_t tk0 = merge_splitters[0];
        for(int p = 0; p < PARTFANOUT; p++){
            if ( relSparts[p]->num_tuples > 0 )
            {
                int64_t stoff = binsearch_lower_bound(relSparts[p]->tuples,
                                                      relSparts[p]->num_tuples,
                                                      tk0);
                startoffset[p] = stoff;
                runsizes[p] = relSparts[p]->num_tuples - stoff;
                task0tot += stoff;

                largest->runstomerge[p]->tuples = relSparts[p]->tuples;
                largest->runstomerge[p]->num_tuples = stoff;
            }
            else
            {
                startoffset[p] = 0;
                runsizes[p] = 0;
                largest->runstomerge[p]->tuples = 0;
                largest->runstomerge[p]->num_tuples = 0;
            }
        }
        largest->totaltuples = task0tot;
        largest->output = outputptr;
        outputptr += task0tot;
    }
    // printf("[INFO ] TID=%d merge-task-0 total = %d\n", my_tid, task0tot);
    for(int t = 1; t < ntasks-1; t++) {
        intkey_t tk = merge_splitters[t];
        mergetask_t * mwtask = create_mergetask(PARTFANOUT);
        uint64_t tasktot = 0;
        for(int p = 0; p < PARTFANOUT; p++){
            if ( runsizes[p] > 0 )
            {
                int64_t stoff = binsearch_lower_bound(relSparts[p]->tuples
                                                      + startoffset[p],
                                                      runsizes[p],
                                                      tk);
                mwtask->runstomerge[p]->tuples = relSparts[p]->tuples + startoffset[p];
                mwtask->runstomerge[p]->num_tuples = stoff;
                startoffset[p] += stoff;
                runsizes[p] -= stoff;
                tasktot += mwtask->runstomerge[p]->num_tuples;
            }
            else
            {
                mwtask->runstomerge[p]->tuples = 0;
                mwtask->runstomerge[p]->num_tuples = 0;
            }
        }
        mwtask->totaltuples = tasktot;
        mwtask->output = outputptr;
        outputptr += tasktot;

        // if(mwtask->totaltuples > DECOMPOSE_THR){
        //     int onefourth = mwtask->runstomerge[0]->num_tuples/4;
        //     int threefourth = 3*onefourth;
        //     printf("[IMPORTANT:TID=%d] HeavyHitter => NUM=%d; at[%d]=%d ; at[%d]=%d\n",
        //            my_tid, mwtask->runstomerge[0]->num_tuples,
        //            onefourth, mwtask->runstomerge[0]->tuples[onefourth].key,
        //            threefourth, mwtask->runstomerge[0]->tuples[threefourth].key);
        // }

        //printf("[INFO ] TID=%d creating merge task with %d total tuples.\n", my_tid, tasktot);
        if(largest == 0){
            if(mwtask->totaltuples > task0tot){
                largest = mwtask;
            }
            else {
                DEBUGMSG(1, "[---- TID=%d adding merge task with %d tuples.\n",
                         my_tid, mwtask->totaltuples);
                taskqueue_addtail(mergequeue, mwtask);
            }
        }
        else if(mwtask->totaltuples > largest->totaltuples){
            DEBUGMSG(1, "[---- TID=%d adding merge task with %d tuples.\n",
                     my_tid, largest->totaltuples);
            taskqueue_addtail(mergequeue, largest);
            largest = mwtask;
        }
        else {
            DEBUGMSG(1, "[---- TID=%d adding merge task with %d tuples.\n",
                     my_tid, mwtask->totaltuples);
            taskqueue_addtail(mergequeue, mwtask);
        }
    }
    {
        /* last task until the end of the run */
        mergetask_t * mwtask = create_mergetask(PARTFANOUT);
        uint64_t tasktot = 0;
        for(int p = 0; p < PARTFANOUT; p++){
            mwtask->runstomerge[p]->tuples = relSparts[p]->tuples + startoffset[p];
            mwtask->runstomerge[p]->num_tuples = runsizes[p];
            tasktot += mwtask->runstomerge[p]->num_tuples;
            // if(my_tid==3){
            //     printf("TID=3 --> key0=%d key-last=%d\n",
            //            mwtask->runstomerge[p]->tuples[0].key,
            //            mwtask->runstomerge[p]->tuples[mwtask->runstomerge[p]->num_tuples-1].key);
            // }
        }
        mwtask->totaltuples = tasktot;
        mwtask->output = outputptr;
        //printf("[INFO ] TID=%d creating merge task with %d total tuples to merge.\n", my_tid, tasktot);
        if(largest == 0){
            if(mwtask->totaltuples > task0tot){
                largest = mwtask;
            }
            else {
                DEBUGMSG(1, "[---- TID=%d adding merge task with %d tuples.\n",
                         my_tid, mwtask->totaltuples);
                taskqueue_addtail(mergequeue, mwtask);
            }
        }
        else if(mwtask->totaltuples > largest->totaltuples){
            DEBUGMSG(1, "[---- TID=%d adding merge task with %d tuples.\n",
                     my_tid, largest->totaltuples);
            taskqueue_addtail(mergequeue, largest);
            largest = mwtask;
        }
        else {
            DEBUGMSG(1, "[---- TID=%d adding merge task with %d tuples.\n",
                     my_tid, mwtask->totaltuples);
            taskqueue_addtail(mergequeue, mwtask);
        }
    }

    /* return the largest merge tasks without adding to the task queue. */
    *largest_ret = largest;
}

void
detect_heavy_hitters_and_add_tasks(relation_t * heavyhits,
                                   taskqueue_t * mergequeue,
                                   mergetask_t * largest_task,
                                   int my_tid,
                                   const int PARTFANOUT)
{
    relation_t ** relparts = largest_task->runstomerge;
    /* check if any heavyhits exist in relparts[0...F] then
       separately handle those */
    int heavyhitexist = 0;

    for(unsigned int h = 0; h < heavyhits->num_tuples; h++)
    {
        for(int p = 0; p < PARTFANOUT; p++)
        {
            if( relparts[p]->num_tuples > 0 )
            {
                if(relparts[p]->tuples[0].key <= heavyhits->tuples[h].key &&
                   relparts[p]->tuples[relparts[p]->num_tuples-1].key
                   >= heavyhits->tuples[h].key)
                {
                    heavyhitexist = heavyhits->tuples[h].key;
                    break;
                }
            }
        }

        if( heavyhitexist )
            break;
    }

    if( heavyhitexist )
    {
        /* try to identify heavyhit bounds and directly copy */
        DEBUGMSG(1, "$$$ HEAVYHIT FOUND = %d\n", heavyhitexist);
        mergetask_t * mwtask = create_mergetask(PARTFANOUT);
        mergetask_t * mwtask2 = create_mergetask(PARTFANOUT);
        int64_t tasktot = 0;
        mwtask2->totaltuples = 0;
        for(int p = 0; p < PARTFANOUT; p++)
        {
            if ( relparts[p]->num_tuples > 0 )
            {
                /* if(relparts[p]->tuples[0].key <= heavyhitexist && */
                /*    relparts[p]->tuples[relparts[p]->num_tuples-1].key  */
                /*    >= heavyhitexist) */
                /* { */
                uint64_t stoff = binsearch_lower_bound(relparts[p]->tuples,
                                                       relparts[p]->num_tuples,
                                                       heavyhitexist);
                uint64_t endoff = binsearch_upper_bound(relparts[p]->tuples,
                                                        relparts[p]->num_tuples,
                                                        heavyhitexist+1);

                if(endoff > stoff)
                {
                    // printf("--- HH@[%d]=%d --- HH@[%d]=%d\n",
                    //        stoff, relparts[p]->tuples[stoff].key,
                    //        endoff,
                    //       relparts[p]->tuples[endoff].key);
                    mwtask->runstomerge[p]->tuples = relparts[p]->tuples + stoff;
                    mwtask->runstomerge[p]->num_tuples = endoff - stoff + 1;
                    tasktot += mwtask->runstomerge[p]->num_tuples;
                    if(endoff < (relparts[p]->num_tuples-1)){
                        mwtask2->runstomerge[p]->tuples = relparts[p]->tuples + endoff + 1;
                        mwtask2->runstomerge[p]->num_tuples = relparts[p]->num_tuples - endoff -1;
                        mwtask2->totaltuples += mwtask2->runstomerge[p]->num_tuples;
                    }
                    else {
                        mwtask2->runstomerge[p]->tuples = 0;
                        mwtask2->runstomerge[p]->num_tuples = 0;
                    }
                    /* Update the original task. */
                    relparts[p]->num_tuples = stoff;
                }
                    /* else if (stoff == relparts[p]->num_tuples)  */
                    /* { */
                    /*     mwtask->runstomerge[p]->tuples = relparts[p]->tuples; */
                    /*     mwtask->runstomerge[p]->num_tuples = stoff; */
                    /*     tasktot += mwtask->runstomerge[p]->num_tuples; */
                    /*     mwtask2->runstomerge[p]->tuples = 0; */
                    /*     mwtask2->runstomerge[p]->num_tuples = 0; */
                    /*     /\* Update the original task. *\/ */
                    /*     relparts[p]->num_tuples = 0; */
                    /* } */
                else if (endoff == 0)
                {
                    mwtask->runstomerge[p]->tuples = 0;
                    mwtask->runstomerge[p]->num_tuples = 0;
                    mwtask2->runstomerge[p]->tuples = relparts[p]->tuples;
                    mwtask2->runstomerge[p]->num_tuples = relparts[p]->num_tuples;
                    mwtask2->totaltuples += mwtask2->runstomerge[p]->num_tuples;
                    /* Update the original task. */
                    relparts[p]->num_tuples = 0;
                }
                else {
                    mwtask->runstomerge[p]->tuples = 0;
                    mwtask->runstomerge[p]->num_tuples = 0;
                    mwtask2->runstomerge[p]->tuples = 0;
                    mwtask2->runstomerge[p]->num_tuples = 0;
                }
                /* } */
                /* else { */
                /*     mwtask->runstomerge[p]->tuples = 0; */
                /*     mwtask->runstomerge[p]->num_tuples = 0; */
                /*     mwtask2->runstomerge[p]->tuples = 0; */
                /*     mwtask2->runstomerge[p]->num_tuples = 0; */
                /* } */
            }
            else
            {
                mwtask->runstomerge[p]->tuples = 0;
                mwtask->runstomerge[p]->num_tuples = 0;
                mwtask2->runstomerge[p]->tuples = 0;
                mwtask2->runstomerge[p]->num_tuples = 0;
            }
        }
        mwtask->totaltuples = tasktot;

        /* Update the original task. */
        largest_task->totaltuples = largest_task->totaltuples
                                    - mwtask->totaltuples
                                    - mwtask2->totaltuples;

        /* Update starting output offsets. */
        mwtask->output = largest_task->output + largest_task->totaltuples;
        mwtask->isheavyhitter = 1;
        mwtask2->output = mwtask->output + tasktot;

        DEBUGMSG(1, "[---- TID=%d adding merge HH-copy task with %d tuples.\n",
                 my_tid, mwtask->totaltuples);
        DEBUGMSG(1, "[---- TID=%d adding merge task with %d tuples.\n",
                 my_tid, mwtask2->totaltuples);
        DEBUGMSG(1, "[---- TID=%d adding merge task with %d tuples.\n",
                 my_tid, largest_task->totaltuples);

        /* Add the largest one of these subtasks to the front of the queue. */
        if ( largest_task->totaltuples > mwtask->totaltuples )
        {
            if ( largest_task->totaltuples > mwtask2->totaltuples )
            {
                taskqueue_addfront(mergequeue, largest_task);
                taskqueue_addtail(mergequeue, mwtask);
                taskqueue_addtail(mergequeue, mwtask2);
            }
            else {
                taskqueue_addfront(mergequeue, mwtask2);
                taskqueue_addtail(mergequeue, largest_task);
                taskqueue_addtail(mergequeue, mwtask);
            }
        }
        else
        {
            if ( mwtask->totaltuples > mwtask2->totaltuples )
            {
                taskqueue_addfront(mergequeue, mwtask);
                taskqueue_addtail(mergequeue, largest_task);
                taskqueue_addtail(mergequeue, mwtask2);
            }
            else {
                taskqueue_addfront(mergequeue, mwtask2);
                taskqueue_addtail(mergequeue, mwtask);
                taskqueue_addtail(mergequeue, largest_task);
            }
        }

    }
    else
    {
        DEBUGMSG(1, "$$$ NO HEAVYHIT FOUND; JUST ADDING TO FRONT OF QUEUE.\n");
        taskqueue_addfront(mergequeue, largest_task);
    }
}

/**
 * Find the first offset in sortedrun which is >= skey.
 */
int64_t
binsearch_lower_bound(tuple_t * sortedrun, int64_t ntups, intkey_t skey)
{
    const tuple_t * tups = sortedrun;
    int64_t lo, hi, mid;
    lo = 0;
    hi = ntups - 1;

    while(lo < hi){
        mid = (lo + hi)/2;

        if(tups[mid].key >= skey)
            hi = mid;
        else
            lo = mid + 1;
    }

    if(tups[lo].key >= skey)
        return lo;
    else {
        DEBUGMSG(1, "1. TODO FIXME : can be a problem !\n");
        return lo+1;//return -1;/* not found */
    }
}

/**
 * Find the maximum offset in sortedrun which is < skey.
 */
int64_t
binsearch_upper_bound(tuple_t * sortedrun, int64_t ntup,  intkey_t skey)
{
    const tuple_t * tups = sortedrun;
    int64_t lo, hi, mid;
    lo = 0;
    hi = ntup - 1;

    while(lo < hi){
        mid = (lo + hi + 1)/2;

        if(tups[mid].key < skey)
            lo = mid;
        else
            hi = mid - 1;
    }

    if(tups[lo].key < skey)
        return lo;
    else{
        DEBUGMSG(1, "2. TODO FIXME : can be a problem !\n");
        return 0; //-1;/* not found */
    }
}


mergetask_t *
create_mergetask(int nruns)
{
    size_t alloc_sz = sizeof(mergetask_t) + nruns *
                                            (sizeof(relation_t *) + sizeof(relation_t));
    char * mem = (char *) malloc(alloc_sz);

    mergetask_t * mt = (mergetask_t *)(mem);
    mem += sizeof(mergetask_t);
    mt->runstomerge = (relation_t **)(mem);
    mem += nruns * sizeof(relation_t *);
    for(int i = 0; i < nruns; i++){
        mt->runstomerge[i] = (relation_t *) (mem);
        mem += sizeof(relation_t);
    }
    mt->numruns = nruns;
    mt->isheavyhitter = 0;

    return mt;
}

void
print_mergetask(mergetask_t * tsk)
{
    printf("--- Merge Task Details : NRUNS=%d, is-HH=%d, TOT-SZ=%d ---\n",
           tsk->numruns, tsk->isheavyhitter, tsk->totaltuples);
}

void
free_mergetask(mergetask_t * mt)
{
    free(mt);
}

/**
 * Determines and returns at most SKEW_MAX_HEAVY_HITTERS heavy hitter keys
 * from an equi-depth histogram. Heavy hitter is a key that occurs
 * above given threshold (SKEW_HEAVY_HITTER_THR) percent of the buckets in the
 * histogram.
 */
void
find_heavy_hitters(relation_t * equidepthhist, double thrpercent,
                   relation_t * heavyhits)
{
    if(heavyhits->num_tuples > SKEW_MAX_HEAVY_HITTERS)
        return;

    /* number of threshold buckets that a value must occur to be a HH */
    int64_t threshold = equidepthhist->num_tuples * thrpercent;
    int64_t count = 0;
    intkey_t lastkey = -1;

    for(unsigned int i = 0; i < equidepthhist->num_tuples; i++){
        intkey_t key = equidepthhist->tuples[i].key;

        if(key == lastkey){
            count++;
        }
        else {
            if(lastkey != -1){
                if(count > threshold){
                    /* search in HH first */
                    int notfound = 1;
                    for(unsigned int h = 0; h < heavyhits->num_tuples; h++){
                        if(heavyhits->tuples[h].key == lastkey){
                            notfound = 0;
                            break;
                        }
                    }

                    if(notfound){
                        heavyhits->tuples[heavyhits->num_tuples].key = lastkey;
                        heavyhits->num_tuples ++;
                        if(heavyhits->num_tuples > SKEW_MAX_HEAVY_HITTERS)
                            break;
                    }
                }
            }
            lastkey = key;
            count = 1;
        }
    }
}

/**
 * This method copies given relation tuples to the output array one by
 * one. Used for copying out heavy hitters. There is an optimization
 * opportunity to use AVX/SSE copy.
 */
void
do_fast_memcpy(mergetask_t * mwtask)
{
    tuple_t * out = mwtask->output;
    for(int i = 0; i < mwtask->numruns; i++){
        int64_t numtuples = mwtask->runstomerge[i]->num_tuples;
        nt_memcpy(out, mwtask->runstomerge[i]->tuples, numtuples * sizeof(tuple_t));
        out += numtuples;
    }
}

/**
 * Efficient memcpy operation using non-temporal load/stores.
 */
void
nt_memcpy(void * dst, void * src, size_t sz)
{
// #ifdef __AVX__
// #define ALIGN_BOUNDARY 64
//     char * dstptr, * srcptr;
//     char * endptr = (char *)src + sz;
//     for(dstptr = (char *)dst, srcptr = (char *)src; srcptr != endptr; ){
//         if(((uintptr_t)srcptr % ALIGN_BOUNDARY) == 0 &&
//            ((uintptr_t)dstptr % ALIGN_BOUNDARY) == 0)
//             break;

//         *dstptr ++ = *srcptr ++;
//     }

//     __m256i * src256 = (__m256i *) srcptr;
//     __m256i * dst256 = (__m256i *) dstptr;
//     __m256i * end256 = (__m256i *) ((uintptr_t)endptr & ~(ALIGN_BOUNDARY*2-1));
//     for( ; src256 != end256; src256 += 2, dst256 += 2) {
//      __asm volatile(
//                   "vmovntdqa 0(%0) , %%xmm0;  "
//                   "vmovntdqa 32(%0), %%xmm1;  "
//                   "vmovntdq %%xmm0, 0(%1) ;  "
//                   "vmovntdq %%xmm1, 32(%1) ;  "
//                   ::"r"(src256), "r"(dst256));
//     }

//     /* handle remainders */
//     for(dstptr = (char *)dst256, srcptr = (char*)src256; srcptr != endptr; ){
//         *dstptr ++ = *srcptr ++;
//     }


// #elif defined(__SSE4_2__)
// #define ALIGN_BOUNDARY 16
//     /* TODO: No AVX but SSE 4.2 or less? */
// #else
    /* fallback memcpy */
    memcpy(dst, src, sz);
// #endif
}
