/**
 * @file    sortmergejoin_mpsm.c
 * @author  Cagri Balkesen <cagri.balkesen@inf.ethz.ch>
 * @date    Sat Dec 15 15:10:38 2012
 * @version $Id $
 *
 * @brief   Implementation of the Massively Parallel Sort-Merge Join (MPSM) as
 *          described in VLDB'12 Paper by Albutiu et al.
 *
 * \ingroup Joins
 */
#include <stdlib.h> /* malloc() */
#include <math.h>   /* log2(), ceil() */

#include "avxsort.h" /* avxsort_tuples() */
#include "common_functions.h"
#include "scalarsort.h" /* scalarsort_tuples() */
#include "sortmergejoin_mpsm.h"

#ifdef JOIN_MATERIALIZE
#include "../utils/tuple_buffer.h"
#endif

/** Modulo hash function using bitmask and shift */
#define HASH_BIT_MODULO(K, MASK, NBITS) (((K-1) & MASK) >> NBITS)

/**
 * MPSM: Main thread of Partial-Sort-Scan-Join as described by Albutiu et al.
 *
 * @param param parameters of the thread, see arg_t for details.
 */
void *
mpsmjoin_thread(void * param);


/** @note The implementation of mpsm is excluded from this source code package. Contact us if you need it. */
result_t *
sortmergejoin_mpsm(relation_t *relR, relation_t *relS, joinconfig_t *joincfg, int exp_id, int window_size, int gap)
{
    /* Just a place holder */
    printf("[WARN ] The implementation of mpsm is excluded from this source code package. Contact us if you need it.\n");
    exit(0);
    return NULL;
}

void *
mpsmjoin_thread(void * param)
{
    return 0;
}
