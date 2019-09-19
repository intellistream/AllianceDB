//
// Created by tony on 10/09/19.
//

#ifndef ALLIANCEDB_PARTITION_H
#define ALLIANCEDB_PARTITION_H

#include "../util/types.h" /* relation_t, tuple_t */

/* just to enable compilation with g++ */
#if defined(__cplusplus) && !defined(restrict)
#define restrict __restrict__
#endif

/**
 * Radix clustering algorithm (originally described by Manegold et al)
 * The algorithm mimics the 2-pass radix clustering algorithm from
 * Kim et al. The difference is that it does not compute
 * prefix-sum, instead the sum (offset in the code) is computed iteratively.
 *
 * @warning This method puts padding between clusters, see
 * radix_cluster_nopadding for the one without padding.
 *
 * @param outRel [out] result of the partitioning
 * @param inRel [in] input relation
 * @param hist [out] number of tuples in each partition
 * @param R cluster bits
 * @param D radix bits per pass
 * @returns tuples per partition.
 */
void
radix_cluster(relation_t * restrict outRel,
              relation_t * restrict inRel,
              int32_t * restrict hist,
              int R,
              int D);


/**
 * Partition a given input relation into fanout=2^radixbits partitions using
 * radix clustering algorithm with least significant `nbits`.
 *
 * @param partitions output partitions, an array of relation ptrs of size fanout
 * @param input input relation
 * @param output used for writing out partitioning results
 * @param nbits number of least significant bits to use for partitioning
 *
 * @return
 */
void
partition_relation(relation_t ** partitions,
                   relation_t * input,
                   relation_t * output,
                   int radixbits,
                   int shiftbits);

/**
 * Partition a given input relation into fanout=2^radixbits partitions using
 * radix clustering algorithm with least significant `nbits`.
 *
 * @note uses software-managed buffers for partitioning
 *
 * @param partitions output partitions, an array of relation ptrs of size fanout
 * @param input input relation
 * @param output used for writing out partitioning results
 * @param nbits number of least significant bits to use for partitioning
 *
 * @return
 */
void
partition_relation_optimized(relation_t ** partitions,
                             relation_t * input,
                             relation_t * output,
                             uint32_t nbits,
                             uint32_t shiftbits);

/**
 * Partition a given input relation into fanout=2^radixbits partitions using
 * radix clustering algorithm with least significant `nbits`.
 *
 * @note uses software-managed buffers for partitioning and
 * integrates prefix sum in to the partition buffers.
 *
 * @param partitions output partitions, an array of relation ptrs of size fanout
 * @param input input relation
 * @param output used for writing out partitioning results
 * @param nbits number of least significant bits to use for partitioning
 *
 * @return
 */
void
partition_relation_optimized_V2(relation_t ** partitions,
                                relation_t * input,
                                relation_t * output,
                                uint32_t nbits,
                                uint32_t shiftbits);

/**
 * This code is a performance baseline benchmark for partitioning speed.
 * It basically just computes a histogram in the first phase and copies
 * out the input to output in the second phase with memcpy() instead of a
 * scattering with the prefix sum.
 *
 * @param partitions
 * @param input
 * @param output used for writing out partitioning results
 * @param nbits
 */
void
histogram_memcpy_bench(relation_t ** partitions,
                       relation_t * input,
                       relation_t * output,
                       uint32_t nbits);

/**
 * This function implements the parallel radix partitioning of a given input
 * relation. Parallel partitioning is done by histogram-based relation
 * re-ordering as described by Kim et al.
 *
 * @param part description of the relation to be partitioned
 */
//void
//parallel_radix_partition(part_t * const part);

#endif //ALLIANCEDB_PARTITION_H
