/**
 * @file    avxsort.h
 * @author  Cagri Balkesen <cagri.balkesen@inf.ethz.ch>
 * @date    Tue Dec 11 18:24:10 2012
 * @version $Id: avxsort.h 3408 2013-02-25 16:27:52Z bcagri $
 *
 * @brief   Implementation of SIMD sorting using AVX instructions.
 *
 *
 */

#ifndef ALLIANCEDB_AVXSORT_H
#define ALLIANCEDB_AVXSORT_H


#include <stdint.h>

#include "../utils/types.h" /* tuple_t */

/**
 * @defgroup sorting Sorting routines
 * @{
 */


/**
 * Sorts given array of items using AVX instructions. If the input is aligned
 * to cache-line, then aligned version of the implementation is executed.
 *
 * @note output array must be pre-allocated before the call.
 *
 * @param inputptr
 * @param outputptr
 * @param nitems
 */
void
avxsort_int64(int64_t **inputptr, int64_t **outputptr, uint64_t nitems);

/**
 * \copydoc avxsort_int64
 * @note currently not implemented.
 */
void
avxsort_int32(int32_t **inputptr, int32_t **outputptr, uint64_t nitems);

/**
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
avxsort_tuples(tuple_t **inputptr, tuple_t **outputptr, uint64_t nitems);

/** @} */

#endif //ALLIANCEDB_AVXSORT_H
