/**
 * @file    scalar_multiwaymerge.h
 * @author  Cagri Balkesen <cagri.balkesen@inf.ethz.ch>
 * @date    Tue Dec 11 18:24:10 2012
 * @version $Id $
 *
 * @brief   Scalar Multi-Way merging with cache-resident buffers.
 *
 * (c) 2014, ETH Zurich, Systems Group
 *
 */
#ifndef SCALARMULTIWAYMERGE_H
#define SCALARMULTIWAYMERGE_H

#include "../utils/types.h" /* tuple_t, relation_t */

/**
 * Scalar Multi-Way Merging with cache-resident merge buffers.
 *
 * @note this implementation is the most optimized version, it eliminates
 * modulo operation for ring buffers and decomposes the merge of ring buffers.
 *
 * @param output resulting merged runs
 * @param parts input relations to merge
 * @param nparts number of input relations (fan-in)
 * @param bufntuples fifo buffer size in number of tuples
 * @param fifobuffer cache-resident fifo buffer
 *
 * @return total number of tuples
 */
uint64_t
scalar_multiway_merge(tuple_t * output,
                      relation_t ** parts,
                      uint32_t nparts,
                      tuple_t * fifobuffer,
                      uint32_t bufntuples);

/**
 * Scalar Multi-Way Merging with cache-resident merge buffers.
 *
 * @note this implementation uses modulo operation for accessing ring buffers.
 *
 * @param output resulting merged runs
 * @param parts input relations to merge
 * @param nparts number of input relations (fan-in)
 * @param bufntuples fifo buffer size in number of tuples
 * @param fifobuffer cache-resident fifo buffer
 *
 * @return total number of tuples
 */
uint64_t
scalar_multiway_merge_modulo(tuple_t * output,
                             relation_t ** parts,
                             uint32_t nparts,
                             tuple_t * fifobuffer,
                             uint32_t bufntuples);

/**
 * Scalar Multi-Way Merging with cache-resident merge buffers.
 *
 * @note this implementation uses bit-and'ing for accessing ring buffers.
 *       However, it requires ring buffer size to be power of 2, thus
 *       reducing effective cache usage.
 *
 * @param output resulting merged runs
 * @param parts input relations to merge
 * @param nparts number of input relations (fan-in)
 * @param bufntuples fifo buffer size in number of tuples
 * @param fifobuffer cache-resident fifo buffer
 *
 * @return total number of tuples
 */
uint64_t
scalar_multiway_merge_bitand(tuple_t * output,
                             relation_t ** parts,
                             uint32_t nparts,
                             tuple_t * fifobuffer,
                             uint32_t bufntuples);


#endif /* SCALARMULTIWAYMERGE_H */
