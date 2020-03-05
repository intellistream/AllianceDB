/**
 * @file    scalarsort.h
 * @author  Cagri Balkesen <cagri.balkesen@inf.ethz.ch>
 * @date    Tue Sep 25 15:19:37 2012
 * @version $Id$
 *
 * @brief  Implementation of sorting using scalar sorting algorithms,
 * mainly C++ sort() or C qsort().
 *
 * (c) 2014, ETH Zurich, Systems Group
 *
 */
#ifndef SCALARSORT_H
#define SCALARSORT_H

#include <stdint.h>

#include "../utils/types.h" /* tuple_t */

/**
 * \ingroup sorting
 * Sorts given array of items using either C++ sort() or C qsort().
 *
 * @note output array must be pre-allocated before the call.
 *
 * @param inputptr
 * @param outputptr
 * @param nitems
 */
void
scalarsort_int64(int64_t ** inputptr, int64_t ** outputptr, uint64_t nitems);

/**
 * \ingroup sorting
 * \copydoc scalarsort_int64
 */
void
scalarsort_int32(int32_t ** inputptr, int32_t ** outputptr, uint64_t nitems);

/**
 * \ingroup sorting
 * Sorts given array of tuples on "key" field using either C++ sort() or C qsort().
 *
 * @note output array must be pre-allocated before the call.
 *
 * @param inputptr
 * @param outputptr
 * @param nitems
 */
void
scalarsort_tuples(tuple_t ** inputptr, tuple_t ** outputptr, uint64_t nitems);

#endif /* SCALARSORT_H */
