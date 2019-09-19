/**
 * @file    avx_multiwaymerge.h
 * @author  Cagri Balkesen <cagri.balkesen@inf.ethz.ch>
 * @date    Tue Dec 11 18:24:10 2012
 * @version $Id $
 *
 * @brief   Multi-way merging methods, parallel with AVX.
 *
 * (c) 2014, ETH Zurich, Systems Group
 *
 */

#ifndef AVXMULTIWAYMERGE_H
#define AVXMULTIWAYMERGE_H

#include "../util/types.h" /* tuple_t, relation_t */

/**
 * AVX-based Multi-Way Merging with cache-resident merge buffers.
 *
 * @note this implementation is the most optimized version, it uses AVX
 * merge routines, eliminates modulo operation for ring buffers and
 * decomposes the merge of ring buffers.
 *
 * @param output resulting merged runs
 * @param parts input relations to merge
 * @param nparts number of input relations (fan-in)
 * @param fifobuffer cache-resident fifo buffer
 * @param bufntuples fifo buffer size in number of tuples
 *
 * @return total number of tuples
 */
uint64_t
avx_multiway_merge(tuple_t * output,
                   relation_t ** parts,
                   uint32_t nparts,
                   tuple_t * fifobuffer,
                   uint32_t bufntuples);

#endif /* AVXMULTIWAYMERGE_H */
