/**
 * @file    merge.h
 * @author  Cagri Balkesen <cagri.balkesen@inf.ethz.ch>
 * @date    Tue Dec 11 18:24:10 2012
 * @version $Id $
 *
 * @brief   Various merging methods, i.e. scalar, parallel with AVX, etc.
 *
 * (c) 2014, ETH Zurich, Systems Group
 *
 */
#ifndef MERGE_H
#define MERGE_H

#include <stdint.h>

#include "../util/types.h" /* tuple_t */

/**
 * Merges two sorted lists of length len into a new sorted list of length
 * outlen. The following configurations are possible:
 *
 * #define MERGEBRANCHING 0:NORMAL-IF, 1:CMOVE, 2:PREDICATION
 * #define MERGEBITONICWIDTH 4,8,16
 * MERGEALIGNED: depending on if inpA, inpB and out are cache-line aligned.
 *
 * @warning Assumes that inputs will have items multiple of 8.
 * @param inpA sorted list A
 * @param inpB sorted list B
 * @param out merged output list
 * @param len length of input
 *
 * @return output size
 */
uint64_t
avx_merge_tuples(tuple_t * const inA,
                 tuple_t * const inB,
                 tuple_t * const outp,
                 const uint64_t lenA,
                 const uint64_t lenB);

/**
 * Merges two sorted lists of length len into a new sorted list of length
 * outlen. The following configurations are possible:
 *
 * #define MERGEBRANCHING 0:NORMAL-IF, 1:CMOVE, 2:PREDICATION
 * #define MERGEBITONICWIDTH 4,8,16
 * MERGEALIGNED: depending on if inpA, inpB and out are cache-line aligned.
 *
 * @warning Assumes that inputs will have items multiple of 8.
 * @param inpA sorted list A
 * @param inpB sorted list B
 * @param out merged output list
 * @param len length of input
 */
uint64_t
avx_merge_int64(int64_t * const inpA,
                int64_t * const inpB,
                int64_t * const out,
                const uint64_t lenA,
                const uint64_t lenB);

/**
 * Scalar method for merging 2 sorted lists into a 1 sorted list.
 *
 * @param inpA
 * @param inpB
 * @param out
 * @param lenA length of array A
 * @param lenB length of array B
 *
 * @return length of the new merged list
 */
uint64_t
scalar_merge_int64(int64_t * const inpA,
                   int64_t * const inpB,
                   int64_t * const out,
                   const uint64_t lenA,
                   const uint64_t lenB);

/**
 * Scalar merge routine for two sorted tuple arrays.
 *
 * @param inpA
 * @param inpB
 * @param out
 * @param lenA length of array A
 * @param lenB length of array B
 *
 * @return length of the new merged list
 */
uint64_t
scalar_merge_tuples(tuple_t * const inpA,
                    tuple_t * const inpB,
                    tuple_t * const out,
                    const uint64_t lenA,
                    const uint64_t lenB);


#endif /* MERGE_H */
