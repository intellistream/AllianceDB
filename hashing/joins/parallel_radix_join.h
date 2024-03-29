/**
 * @file    parallel_radix_join.h
 * @author  Cagri Balkesen <cagri.balkesen@inf.ethz.ch>
 * @date    Sun Feb 20:09:59 2012
 * @version $Id: parallel_radix_join.h 4419 2013-10-21 16:24:35Z bcagri $
 *
 * @brief  Provides interfaces for several variants of Radix Hash Join.
 *
 * (c) 2012, ETH Zurich, Systems Group
 *
 */

#ifndef PARALLEL_RADIX_JOIN_H
#define PARALLEL_RADIX_JOIN_H


#include "../utils/types.h" /* relation_t */
#include "common_functions.h"
#include "../benchmark.h"

/**
 * PRO: Parallel Radix Join Optimized.
 *
 * The "Parallel Radix Join Optimized" implementation denoted as PRO implements
 * the parallel radix join idea by Kim et al. with further optimizations. Mainly
 * it uses the bucket chaining for the build phase instead of the
 * histogram-based relation re-ordering and performs better than other
 * variations such as PRHO, which applies SIMD and prefetching
 * optimizations.

 * @param relR  input relation R - inner relation
 * @param relS  input relation S - inner relation
 *
 * @return number of result tuples
 */
result_t *
PRO(relation_t *relR, relation_t *relS, param_t cmd_params);

/**
 * RJ: Radix Join.
 *
 * The "Radix Join" implementation denoted as RJ implements
 * the single-threaded original multipass radix cluster join idea
 * by Manegold et al.
 *
 * @param relR  input relation R - inner relation
 * @param relS  input relation S - inner relation
 *
 * @warning nthreads parameter does not have any effect for this algorithm.
 * @return number of result tuples
 */
result_t *
RJ_st(relation_t *relR, relation_t *relS, param_t cmd_params);

/**
 * PRH: Parallel Radix Join Histogram-based.
 *
 * The "Parallel Radix Join Histogram-based" implementation denoted as PRH
 * implements the parallel radix join idea by Kim et al. without SIMD and
 * prefetching optimizations. The difference from PRO is that the build phase
 * is based on the histogram-based relation re-ordering idea.

 * @param relR  input relation R - inner relation
 * @param relS  input relation S - inner relation
 *
 * @return number of result tuples
 */
result_t *
PRH(relation_t *relR, relation_t *relS, param_t cmd_params);

/**
 * PRHO: Parallel Radix Join Histogram-based Optimized.
 *
 * The "Parallel Radix Join Histogram-based Optimized" implementation denoted
 * as PRHO implements the parallel radix join idea by Kim et al. with SIMD and
 * prefetching optimizations. The difference from PRH is that the probe phase
 * uses SIMD and software prefetching optimizations.

 * @param relR  input relation R - inner relation
 * @param relS  input relation S - inner relation
 *
 * @return number of result tuples
 */
result_t *
PRHO(relation_t *relR, relation_t *relS, param_t cmd_params);

#endif /* PARALLEL_RADIX_JOIN_H */
