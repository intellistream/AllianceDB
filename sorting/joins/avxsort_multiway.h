/**
 * @file    avxsort_multiwaymerge.h
 * @author  Cagri Balkesen <cagri.balkesen@inf.ethz.ch>
 * @date    Tue Dec 11 18:24:10 2012
 * @version $Id $
 *
 * @brief   AVX sorting that uses multi-way merge for inputs > L3-size.
 *
 *
 */

#ifndef AVXSORT_MULTIWAYMERGE_H_
#define AVXSORT_MULTIWAYMERGE_H_

#include <stdint.h>

#include "../utils/types.h" /* tuple_t */

/**
 * \ingroup sorting
 *
 * As an optimization, this algorithm does the
 * merging phase using a multi-way merge-tree and consumes less memory
 * bandwidth if the input size is greater than the L3 cache size.
 * Sorts given array of tuples on "key" field using AVX instructions. If the
 * input is aligned to cache-line, then aligned version of the implementation
 * is executed.
 *
 * @note output array must be pre-allocated before the call.
 *
 * @param inputptr
 * @param outputptr
 * @param nitems
 */
void
avxsortmultiway_tuples(tuple_t ** inputptr, tuple_t ** outputptr, uint64_t nitems);

/**
 * \ingroup sorting
 *
 * As an optimization, this algorithm does the
 * merging phase using a multi-way merge-tree and consumes less memory
 * bandwidth if the input size is greater than the L3 cache size.
 * Sorts given array of tuples on "key" field using AVX instructions. If the
 * input is aligned to cache-line, then aligned version of the implementation
 * is executed.
 *
 * @note output array must be pre-allocated before the call.
 *
 * @param inputptr
 * @param outputptr
 * @param nitems
 */
void
avxsortmultiway_int64(int64_t ** inputptr, int64_t ** outputptr, uint64_t nitems);

#endif /* AVXSORT_MULTIWAYMERGE_H_ */
