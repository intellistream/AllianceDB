#include "numa_shuffle.h"

/**
 * Various NUMA shuffling strategies as also described by NUMA-aware
 * data shuffling paper:
 * NUMA_SHUFFLE_RANDOM, NUMA_SHUFFLE_RING, NUMA_SHUFFLE_NEXT
 *
 * enum numa_strategy_t {RANDOM, RING, NEXT};
 */
static enum numa_strategy_t numastrategy_;

/** @note make sure that nthreads is always < 512 */
static tuple_t shuffleorder[512];


/**
 * Various NUMA shuffling strategies for data shuffling phase of join
 * algorithms as also described by NUMA-aware data shuffling paper [CIDR'13].
 *
 * NUMA_SHUFFLE_RANDOM, NUMA_SHUFFLE_RING, NUMA_SHUFFLE_NEXT
 */
void
numa_shuffle_init(enum numa_strategy_t numastrategy, int nthreads)
{
    numastrategy_ = numastrategy;
    if(numastrategy == RANDOM){
        /* if random, initialize a randomization once */
        relation_t ss;
        ss.tuples = (tuple_t *) shuffleorder;
        ss.num_tuples = nthreads;
        for(int s=0; s < nthreads; s++)
            ss.tuples[s].key = s;
        knuth_shuffle(&ss);
    }
}

/**
 * Machine specific implementation of NUMA-shuffling strategies.
 *
 * @note The implementation needs to be customized for different hardware
 *       based on the NUMA topology.
 *
 * @note RING-based shuffling is expected to work well only when using all
 *       the threads. Better way to apply ring-based shuffling is to change the
 *       CPU-mappings in "cpu-mapping.txt" and use NEXT-based shuffling.
 *
 * @param my_tid logical thread id of the calling thread.
 * @param nextidx next thread index for shuffling (between 0 and nthreads)
 * @param nthreads number of total threads
 * @return the logical thread id of the destination thread for data shuffling
 */
int
get_numa_shuffle_strategy(int my_tid, int nextidx, int nthreads)
{
    if(numastrategy_ == RANDOM){
        return shuffleorder[nextidx].key;
    }
    else if(numastrategy_ == RING){
        /* for Intel-E5-4640: */
        /*
        static int numa[64] = {
                0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60,
                1, 5, 9, 13, 17, 21, 25, 29, 33, 37, 41, 45, 49, 53, 57, 61,
                2, 6, 10, 14, 18, 22, 26, 30, 34, 38, 42, 46, 50, 54, 58, 62,
                3, 7, 11, 15, 19, 23, 27, 31, 35, 39, 43, 47, 51, 55, 59, 63 };
        nid = numa[my_tid];
        */
#if 0
        int phyid;
        phyid = get_cpu_id(my_tid);
        return (phyid + nextidx) % nthreads; // --> NUMA-SHUFF-RING
#endif
#if 0
        int nid = get_numa_index_of_logical_thread(phyid);
        return get_logical_thread_at_numa_index((nid + nextidx) % nthreads) % nthreads;
#endif
        return (my_tid + (nthreads/get_num_numa_regions()) + nextidx) % nthreads;
    }
    else /* NEXT */ {
        return (my_tid + nextidx) % nthreads; // --> NUMA-SHUFF-NEXT-THR
    }
}
