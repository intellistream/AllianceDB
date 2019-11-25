/**
 * @file    avxcommon.h
 * @author  Cagri Balkesen <cagri.balkesen@inf.ethz.ch>
 * @date    Tue Dec 11 18:24:10 2012
 * @version $Id $
 *
 * @brief   Common AVX code, kernels etc. used by implementations.
 *
 *
 */
#ifndef ALLIANCEDB_AVXCOMMON_H
#define ALLIANCEDB_AVXCOMMON_H

#define HAVE_AVX

#include <immintrin.h> /* AVX intrinsics */

/* just to enable compilation with g++ */
#if defined(__cplusplus)
#undef restrict
#define restrict __restrict__
#endif

typedef struct block4 {
    int64_t val[4];
} block4;
typedef struct block8 {
    int64_t val[8];
} block8;
typedef struct block16 {
    int64_t val[16];
} block16;

/**
 * There are 2 ways to implement branches:
 *     1) With conditional move instr.s using inline assembly (IFELSEWITHCMOVE).
 *     2) With software predication (IFELSEWITHPREDICATION).
 *     3) With normal if-else
 */
#define IFELSEWITHCMOVE       0
#define IFELSEWITHPREDICATION 1
#define IFELSEWITHNORMAL      0

/** Load 2 AVX 256-bit registers from the given address */
#define LOAD8(REGL, REGH, ADDR)                                         \
    do {                                                                \
        REGL = _mm256_load_pd((double const *) ADDR);                   \
        REGH = _mm256_load_pd((double const *)(((block4 *)ADDR) + 1));  \
    } while(0)

/** Load unaligned 2 AVX 256-bit registers from the given address */
#define LOAD8U(REGL, REGH, ADDR)                                        \
    do {                                                                \
        REGL = _mm256_loadu_pd((double const *) ADDR);                  \
        REGH = _mm256_loadu_pd((double const *)(((block4 *)ADDR) + 1)); \
    } while(0)

/** Store 2 AVX 256-bit registers to the given address */
#define STORE8(ADDR, REGL, REGH)                                    \
    do {                                                            \
        _mm256_store_pd((double *) ADDR, REGL);                     \
        _mm256_store_pd((double *)(((block4 *) ADDR) + 1), REGH);   \
    } while(0)

/** Store unaligned 2 AVX 256-bit registers to the given address */
#define STORE8U(ADDR, REGL, REGH)                                   \
    do {                                                            \
        _mm256_storeu_pd((double *) ADDR, REGL);                    \
        _mm256_storeu_pd((double *)(((block4 *) ADDR) + 1), REGH);  \
    } while(0)


/**
 * @note Reversing 64-bit values in an AVX register. It will be possible with
 * single _mm256_permute4x64_pd() instruction in AVX2.
 */
#define REVERSE(REG)                                    \
    do {                                                \
        /* first reverse each 128-bit lane */           \
        REG = _mm256_permute_pd(REG, 0x5);              \
        /* now shuffle 128-bit lanes */                 \
        REG = _mm256_permute2f128_pd(REG, REG, 0x1);    \
    } while(0)


/** Bitonic merge kernel for 2 x 4 elements after the reversing step. */
#define BITONIC4(O1, O2, A, B)                                          \
    do {                                                                \
        /* Level-1 comparisons */                                       \
        __m256d l1 = _mm256_min_pd(A, B);                               \
        __m256d h1 = _mm256_max_pd(A, B);                               \
                                                                        \
        /* Level-1 shuffles */                                          \
        __m256d l1p = _mm256_permute2f128_pd(l1, h1, 0x31);             \
        __m256d h1p = _mm256_permute2f128_pd(l1, h1, 0x20);             \
                                                                        \
        /* Level-2 comparisons */                                       \
        __m256d l2 = _mm256_min_pd(l1p, h1p);                           \
        __m256d h2 = _mm256_max_pd(l1p, h1p);                           \
                                                                        \
        /* Level-2 shuffles */                                          \
        __m256d l2p = _mm256_shuffle_pd(l2, h2, 0x0);                   \
        __m256d h2p = _mm256_shuffle_pd(l2, h2, 0xF);                   \
                                                                        \
        /* Level-3 comparisons */                                       \
        __m256d l3 = _mm256_min_pd(l2p, h2p);                           \
        __m256d h3 = _mm256_max_pd(l2p, h2p);                           \
                                                                        \
        /* Level-3 shuffles implemented with unpcklps unpckhps */       \
        /* AVX cannot shuffle both inputs from same 128-bit lane */     \
        /* so we need 2 more instructions for this operation. */        \
        __m256d l4 = _mm256_unpacklo_pd(l3, h3);                        \
        __m256d h4 = _mm256_unpackhi_pd(l3, h3);                        \
        O1 = _mm256_permute2f128_pd(l4, h4, 0x20);                      \
        O2 = _mm256_permute2f128_pd(l4, h4, 0x31);                      \
    } while(0)



/** Bitonic merge network for 2 x 8 elements without reversing B */
#define BITONIC8(O1, O2, O3, O4, A1, A2, B1, B2)                        \
    do {                                                                \
        /* Level-0 comparisons */                                       \
        __m256d l11 = _mm256_min_pd(A1, B1);                            \
        __m256d l12 = _mm256_min_pd(A2, B2);                            \
        __m256d h11 = _mm256_max_pd(A1, B1);                            \
        __m256d h12 = _mm256_max_pd(A2, B2);                            \
                                                                        \
        BITONIC4(O1, O2, l11, l12);                                     \
        BITONIC4(O3, O4, h11, h12);                                     \
    } while(0)


/** Bitonic merge kernel for 2 x 4 elements */
#define BITONIC_MERGE4(O1, O2, A, B)                                    \
    do {                                                                \
        /* reverse the order of input register B */                     \
        REVERSE(B);                                                     \
        BITONIC4(O1, O2, A, B);                                         \
    } while(0)


/** Bitonic merge kernel for 2 x 8 elements */
#define BITONIC_MERGE8(O1, O2, O3, O4, A1, A2, B1, B2)  \
        do {                                            \
            /* reverse the order of input B */          \
            REVERSE(B1);                                \
            REVERSE(B2);                                \
                                                        \
            /* Level-0 comparisons */                   \
            __m256d l11 = _mm256_min_pd(A1, B2);        \
            __m256d l12 = _mm256_min_pd(A2, B1);        \
            __m256d h11 = _mm256_max_pd(A1, B2);        \
            __m256d h12 = _mm256_max_pd(A2, B1);        \
                                                        \
            BITONIC4(O1, O2, l11, l12);                 \
            BITONIC4(O3, O4, h11, h12);                 \
        } while(0)

/** Bitonic merge kernel for 2 x 16 elements */
#define BITONIC_MERGE16(O1, O2, O3, O4, O5, O6, O7, O8, \
                        A1, A2, A3, A4, B1, B2, B3, B4)         \
        do {                                                    \
            /** Bitonic merge kernel for 2 x 16 elemenets */    \
            /* reverse the order of input B */                  \
            REVERSE(B1);                                        \
            REVERSE(B2);                                        \
            REVERSE(B3);                                        \
            REVERSE(B4);                                        \
                                                                \
            /* Level-0 comparisons */                           \
            __m256d l01 = _mm256_min_pd(A1, B4);                \
            __m256d l02 = _mm256_min_pd(A2, B3);                \
            __m256d l03 = _mm256_min_pd(A3, B2);                \
            __m256d l04 = _mm256_min_pd(A4, B1);                \
            __m256d h01 = _mm256_max_pd(A1, B4);                \
            __m256d h02 = _mm256_max_pd(A2, B3);                \
            __m256d h03 = _mm256_max_pd(A3, B2);                \
            __m256d h04 = _mm256_max_pd(A4, B1);                \
                                                                \
            BITONIC8(O1, O2, O3, O4, l01, l02, l03, l04);       \
            BITONIC8(O5, O6, O7, O8, h01, h02, h03, h04);       \
        } while(0)


/**
 * There are 2 ways to implement branches:
 *     1) With conditional move instr.s using inline assembly (IFELSEWITHCMOVE).
 *     2) With software predication (IFELSEWITHPREDICATION).
 *     3) With normal if-else
 */
#if IFELSEWITHCMOVE
#define IFELSECONDMOVE(NXT, INA, INB, INCR)                             \
    do {                                                                \
        register block4 * tmpA, * tmpB;                                 \
        register int64_t tmpKey;                                        \
                                                                        \
        __asm__ ( "mov %[A], %[tmpA]\n"         /* tmpA <-- inA      */ \
                  "add %[INC], %[A]\n"          /* inA += 4          */ \
                  "mov %[B], %[tmpB]\n"         /* tmpB <-- inB      */ \
                  "mov (%[tmpA]), %[tmpKey]\n"  /* tmpKey <-- *inA   */ \
                  "add %[INC], %[B]\n"          /* inB += 4          */ \
                  "mov %[tmpA], %[NEXT]\n"      /* next <-- A        */ \
                  "cmp (%[tmpB]), %[tmpKey]\n"  /* cmp(tmpKey,*inB ) */ \
                  "cmovnc %[tmpB], %[NEXT]\n"   /* if(A>=B) next<--B */ \
                  "cmovnc %[tmpA], %[A]\n"      /* if(A>=B) A<--oldA */ \
                  "cmovc %[tmpB], %[B]\n"       /* if(A<B)  B<--oldB */ \
                  : [A] "=r" (INA), [B] "=r" (INB), [NEXT] "=r" (NXT),  \
                    [tmpA] "=r" (tmpA), [tmpB] "=r" (tmpB),             \
                    [tmpKey] "=r" (tmpKey)                              \
                  : "0" (INA), "1" (INB), [INC] "i" (INCR)              \
                  :                                                     \
                  );                                                    \
    } while(0)

#elif IFELSEWITHPREDICATION
#define IFELSECONDMOVE(NXT, INA, INB, INCR)                 \
    do {                                                    \
        int8_t cmp = *((int64_t *)INA) < *((int64_t *)INB); \
        NXT  = cmp ? INA : INB;                             \
        INA += cmp;                                         \
        INB += !cmp;                                        \
    } while(0)

#elif IFELSEWITHNORMAL
#define IFELSECONDMOVE(NXT, INA, INB, INCR)                 \
            do {                                            \
                if(*((int64_t *)INA) < *((int64_t *)INB)) { \
                    NXT = INA;                              \
                    INA ++;                                 \
                }                                           \
                else {                                      \
                    NXT = INB;                              \
                    INB ++;                                 \
                }                                           \
            } while(0)                                      \

#endif

#endif //ALLIANCEDB_AVXCOMMON_H
