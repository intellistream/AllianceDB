/**
 * @file    params.h
 * @author  Cagri Balkesen <cagri.balkesen@inf.ethz.ch>
 * @date    Fri Dec 14 13:39:54 2012
 * @version $Id $
 *
 * @brief   Common and important parameters, defines and macros.
 *
 * (c) 2012-2014, ETH Zurich, Systems Group
 *
 */
#ifndef PARAMS_H_
#define PARAMS_H_


/** The partitioning fan-out for the inital step of sort-merge joins */
#ifndef NRADIXBITS_DEFAULT
#define NRADIXBITS_DEFAULT 7
#endif

/** Default partitioning fan-out, can be adjusted from command line. */
#ifndef PARTFANOUT_DEFAULT
#define PARTFANOUT_DEFAULT (1<<NRADIXBITS_DEFAULT)
#endif

/** System cache line size */
#ifndef CACHE_LINE_SIZE
#define CACHE_LINE_SIZE 64
#endif

/** L2 Cache size of the system in bytes */
#ifndef L2_CACHE_SIZE
#define L2_CACHE_SIZE (256*1024)
#endif

/** L3 Cache size of the system in bytes */
#ifndef L3_CACHE_SIZE
#define L3_CACHE_SIZE (20*1024*1024)
#endif

/**
 * There is a padding at the end of all partitions of each thread to align to
 * multiples of CACHE_LINE_SIZE for AVX merge routines to use aligned variants
 * of instructions.
 */
#ifndef CACHELINEPADDING
#define CACHELINEPADDING(PARTFANOUT) ((PARTFANOUT) * CACHE_LINE_SIZE/sizeof(tuple_t))
#endif

/**
 * There is a padding at the end of both input and temporary relations to
 * compensate for each cache line padding at the end of each partition.
 * @warning must be allocated at the end of relations based on nr. of threads.
 */
#ifndef RELATION_PADDING
#define RELATION_PADDING(NTHREADS, PARTFANOUT) ((NTHREADS) * CACHELINEPADDING(PARTFANOUT) * sizeof(tuple_t))
#endif

/**
 * Determines the size of the multi-way merge buffer.
 * Ideally, it should match the size of the L3 cache.
 * @note this buffer is shared by active nr. of threads in a NUMA-region.
 */
#ifndef MWAY_MERGE_BUFFER_SIZE_DEFAULT
#define MWAY_MERGE_BUFFER_SIZE_DEFAULT L3_CACHE_SIZE /* 20MB L3 cache as default value */
#endif

/** Number of tuples that fits into a single cache line */
#define TUPLESPERCACHELINE (CACHE_LINE_SIZE/sizeof(tuple_t))

/** Align N to number of tuples that is a multiple of cache lines */
#define ALIGN_NUMTUPLES(N) (((N)+TUPLESPERCACHELINE-1) & ~(TUPLESPERCACHELINE-1))


#endif /* PARAMS_H_ */
