/** @version $Id $ */

#include "merge.h"
#include "avxsort_core.h" /* Just because of inlines. TODO: fix this */

#ifndef CACHE_LINE_SIZE
#define CACHE_LINE_SIZE 64
#endif

/**
 * Parallel AVX Merge Parameters:
 * #define MERGEBITONICWIDTH 4,8,16
 */
#ifndef MERGEBITONICWIDTH
#define MERGEBITONICWIDTH 16 /* 4,8,16 */
#endif

/**
 * Merge branching, see avxcommon.h:
 * There are 2 ways to implement branches:
 *     1) With conditional move instr.s using inline assembly (IFELSEWITHCMOVE).
 *     2) With software predication (IFELSEWITHPREDICATION).
 *     3) With normal if-else
 */


uint64_t
scalar_merge_int64(int64_t * const inA,
                   int64_t * const inB,
                   int64_t * const outp,
                   const uint64_t lenA,
                   const uint64_t lenB)
{
    uint64_t i, j, k;

    i = 0; j = 0; k = 0;

    while ( i < lenA && j < lenB) {
        if(inA[i] < inB[j]) {
            outp[k] = inA[i];
            k++;
            i++;
        }
        else {
            outp[k] = inB[j];
            k++;
            j++;
        }
    }

    while ( i < lenA ) {
        outp[k] = inA[i];
        k++;
        i++;
    }

    while ( j < lenB ) {
        outp[k] = inB[j];
        k++;
        j++;
    }

    return k;
}

uint64_t
scalar_merge_tuples(tuple_t * const inA,
                    tuple_t * const inB,
                    tuple_t * const outp,
                    const uint64_t lenA,
                    const uint64_t lenB)
{
    uint64_t i, j, k;

    i = 0; j = 0; k = 0;

    while ( i < lenA && j < lenB) {
        if(inA[i].key < inB[j].key) {
            outp[k] = inA[i];
            k++;
            i++;
        }
        else {
            outp[k] = inB[j];
            k++;
            j++;
        }
    }

    while ( i < lenA ) {
        outp[k] = inA[i];
        k++;
        i++;
    }

    while ( j < lenB ) {
        outp[k] = inB[j];
        k++;
        j++;
    }

    return k;
}

uint64_t
avx_merge_tuples(tuple_t * const inA,
                 tuple_t * const inB,
                 tuple_t * const outp,
                 const uint64_t lenA,
                 const uint64_t lenB)
{
    double * const inpA = (double * const) inA;
    double * const inpB = (double * const) inB;
    double * const out = (double * const) outp;

    int isaligned = 0, iseqlen = 0;

    /* is-aligned ? */
    isaligned = (((uintptr_t)inpA % CACHE_LINE_SIZE) == 0) &&
                (((uintptr_t)inpB % CACHE_LINE_SIZE) == 0) &&
                (((uintptr_t)out  % CACHE_LINE_SIZE) == 0);

    /* is equal length? */
    /*There is a bug when size2 = size1 and eqlen enabled */
    /*iseqlen = (lenA == lenB);*/
    /* \todo FIXME There is a problem when using merge-eqlen variants, because the
    merge routine does not consider that other lists begin where one list ends
    and might be overwriting a few tuples. */
    if(iseqlen) {
        if(isaligned){
#if (MERGEBITONICWIDTH == 4)
            merge4_eqlen_aligned(inpA, inpB, out, lenA);
#elif (MERGEBITONICWIDTH == 8)
            merge8_eqlen_aligned(inpA, inpB, out, lenA);
#elif (MERGEBITONICWIDTH == 16)
            merge16_eqlen_aligned(inpA, inpB, out, lenA);
#endif
        }
        else {
#if (MERGEBITONICWIDTH == 4)
            merge4_eqlen(inpA, inpB, out, lenA);
#elif (MERGEBITONICWIDTH == 8)
            merge8_eqlen(inpA, inpB, out, lenA);
#elif (MERGEBITONICWIDTH == 16)
            merge16_eqlen(inpA, inpB, out, lenA);
#endif
        }
    }
    else {
        if(isaligned){
#if (MERGEBITONICWIDTH == 4)
            merge4_varlen_aligned(inpA, inpB, out, lenA, lenB);
#elif (MERGEBITONICWIDTH == 8)
            merge8_varlen_aligned(inpA, inpB, out, lenA, lenB);
#elif (MERGEBITONICWIDTH == 16)
            merge16_varlen_aligned(inpA, inpB, out, lenA, lenB);
#endif
        }
        else {
#if (MERGEBITONICWIDTH == 4)
            merge4_varlen(inpA, inpB, out, lenA, lenB);
#elif (MERGEBITONICWIDTH == 8)
            merge8_varlen(inpA, inpB, out, lenA, lenB);
#elif (MERGEBITONICWIDTH == 16)
            merge16_varlen(inpA, inpB, out, lenA, lenB);
#endif
        }
    }

    return (lenA + lenB);
}


uint64_t
avx_merge_int64(double * const inpA,
                double * const inpB,
                double * const out,
                const uint64_t lenA,
                const uint64_t lenB)
{
    int isaligned = 0, iseqlen = 0;

    /* is-aligned ? */
    isaligned = (((uintptr_t)inpA % CACHE_LINE_SIZE) == 0) &&
                (((uintptr_t)inpB % CACHE_LINE_SIZE) == 0) &&
                (((uintptr_t)out  % CACHE_LINE_SIZE) == 0);

    /* is equal length? */
    /* iseqlen = (lenA == lenB); */
    /* TODO: There is a problem when using merge-eqlen variants, because the
    merge routine does not consider that other lists begin where one list ends
    and might be overwriting a few tuples. */
    if(iseqlen) {
        if(isaligned){
#if (MERGEBITONICWIDTH == 4)
            merge4_eqlen_aligned(inpA, inpB, out, lenA);
#elif (MERGEBITONICWIDTH == 8)
            merge8_eqlen_aligned(inpA, inpB, out, lenA);
#elif (MERGEBITONICWIDTH == 16)
            merge16_eqlen_aligned(inpA, inpB, out, lenA);
#endif
        }
        else {
#if (MERGEBITONICWIDTH == 4)
            merge4_eqlen(inpA, inpB, out, lenA);
#elif (MERGEBITONICWIDTH == 8)
            merge8_eqlen(inpA, inpB, out, lenA);
#elif (MERGEBITONICWIDTH == 16)
            merge16_eqlen(inpA, inpB, out, lenA);
#endif
        }
    }
    else {
        if(isaligned){
#if (MERGEBITONICWIDTH == 4)
            merge4_varlen_aligned(inpA, inpB, out, lenA, lenB);
#elif (MERGEBITONICWIDTH == 8)
            merge8_varlen_aligned(inpA, inpB, out, lenA, lenB);
#elif (MERGEBITONICWIDTH == 16)
            merge16_varlen_aligned(inpA, inpB, out, lenA, lenB);
#endif
        }
        else {
#if (MERGEBITONICWIDTH == 4)
            merge4_varlen(inpA, inpB, out, lenA, lenB);
#elif (MERGEBITONICWIDTH == 8)
            merge8_varlen(inpA, inpB, out, lenA, lenB);
#elif (MERGEBITONICWIDTH == 16)
            merge16_varlen(inpA, inpB, out, lenA, lenB);
#endif
        }
    }

    return (lenA + lenB);
}
