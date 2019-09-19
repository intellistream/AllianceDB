#include <immintrin.h> /* AVX intrinsics */
#include <stdlib.h> /* calloc() */
#include <string.h> /* memset() */
#include <math.h>   /* log2() */
#include <stdio.h>

#include "partition.h"
/* #include "iacaMarks.h" */

#ifdef PERF_COUNTERS
#include "perf_counters.h"      /* PCM_x */
#endif

#ifndef CACHE_LINE_SIZE
#define CACHE_LINE_SIZE 64
#endif

#ifdef __GNUC__
#define UNUSED __attribute__ ((unused))
#else
#define UNUSED
#endif


/** Number of tuples that fits into a single cache line */
#define TUPLESPERCACHELINE (CACHE_LINE_SIZE/sizeof(tuple_t))

/** Modulo hash function using bitmask and shift */
#define HASH_BIT_MODULO(K, MASK, NBITS) (((K-1) & MASK) >> NBITS)

/** Align a pointer to given size */
#define ALIGNPTR(PTR, SZ) (((uintptr_t)PTR + (SZ-1)) & ~(SZ-1))

/** Align N to number of tuples that is a multiple of cache lines */
#define ALIGN_NUMTUPLES(N) ((N+TUPLESPERCACHELINE-1) & ~(TUPLESPERCACHELINE-1))

/** Data structure for representing a single cache line */
typedef union cacheline {
    struct {
        tuple_t tuples[TUPLESPERCACHELINE];
    } tuples;
    struct {
        tuple_t tuples[TUPLESPERCACHELINE - 1];
        uint32_t slot;
    } data;
} cacheline_t;


/**
 * Makes a non-temporal write of 64 bytes from src to dst.
 * Uses vectorized non-temporal stores if available, falls
 * back to assignment copy.
 *
 * @param dst
 * @param src
 *
 * @return
 */
static inline void
store_nontemp_64B(void * dst, void * src)
{
#ifdef __AVX__
    register __m256i * d1 = (__m256i*) dst;
    register __m256i s1 = *((__m256i*) src);
    register __m256i * d2 = d1+1;
    register __m256i s2 = *(((__m256i*) src)+1);

    _mm256_stream_si256(d1, s1);
    _mm256_stream_si256(d2, s2);
#elif defined(__SSE2__)

    register __m128i * d1 = (__m128i*) dst;
    register __m128i * d2 = d1+1;
    register __m128i * d3 = d1+2;
    register __m128i * d4 = d1+3;
    register __m128i s1 = *(__m128i*) src;
    register __m128i s2 = *((__m128i*)src + 1);
    register __m128i s3 = *((__m128i*)src + 2);
    register __m128i s4 = *((__m128i*)src + 3);

    _mm_stream_si128 (d1, s1);
    _mm_stream_si128 (d2, s2);
    _mm_stream_si128 (d3, s3);
    _mm_stream_si128 (d4, s4);

#else
    /* just copy with assignment */
    *(cacheline_t *)dst = *(cacheline_t *)src;

#endif
}

void
radix_cluster(relation_t * restrict outRel,
              relation_t * restrict inRel,
              int32_t * restrict hist,
              int R,
              int D)
{
    uint32_t i;
    uint32_t M = ((1 << D) - 1) << R;
    uint32_t offset;
    uint32_t fanOut = 1 << D;

    /* the following are fixed size when D is same for all the passes,
       and can be re-used from call to call. Allocating in this function
       just in case D differs from call to call. */
    uint32_t dst[fanOut];

    /* count tuples per cluster */
    for( i=0; i < inRel->num_tuples; i++ ){
        uint32_t idx = HASH_BIT_MODULO(inRel->tuples[i].key, M, R);
        hist[idx]++;
    }
    offset = 0;
    /* determine the start and end of each cluster depending on the counts. */
    for ( i=0; i < fanOut; i++ ) {
        /* dst[i]      = outRel->tuples + offset; */
        /* determine the beginning of each partitioning by adding some
           padding to avoid L1 conflict misses during scatter. */
        dst[i] = offset;// + i * SMALL_PADDING_TUPLES;
        offset += hist[i];
    }


    const uint32_t num = inRel->num_tuples;
    tuple_t * restrict in = inRel->tuples;
    tuple_t * restrict out = outRel->tuples;

    /* copy tuples to their corresponding clusters at appropriate offsets */
#if 1
    for( i=0; i < num; i++ ){
        /* IACA_START */
        register uint32_t idx = HASH_BIT_MODULO(in[i].key, M, R);
        register uint32_t d = dst[idx] ++;
        out[d] = in[i];
    }
    /* IACA_END */
#elif 0
    /*ORIGINAL version */
    for(i=0; i < inRel->num_tuples; i++){
        /* IACA_START */
        uint32_t idx   = HASH_BIT_MODULO(inRel->tuples[i].key, M, R);
        outRel->tuples[ dst[idx] ] = inRel->tuples[i];
        ++dst[idx];
    }
    /* IACA_END */
#endif
}


void
radix_cluster_optimized(relation_t * restrict outRel,
                        relation_t * restrict inRel,
                        int32_t * restrict hist,
                        int R,
                        int D)
{
    uint32_t i;
    uint32_t offset = 0;
    const uint32_t M       = ((1 << D) - 1) << R;
    const uint32_t fanOut  = 1 << D;
    const uint32_t ntuples = inRel->num_tuples;

    tuple_t * input  = inRel->tuples;
    tuple_t * output = outRel->tuples;

    uint32_t    dst[fanOut]    __attribute__((aligned(CACHE_LINE_SIZE)));
    cacheline_t buffer[fanOut] __attribute__((aligned(CACHE_LINE_SIZE)));

    /* count tuples per cluster */
    for( i = 0; i < ntuples; i++ ){
        uint32_t idx = HASH_BIT_MODULO(input->key, M, R);
        hist[idx]++;
        input++;
    }

    for ( i = 0; i < fanOut; i++ ) {
        buffer[i].data.slot = 0;
    }

    /* determine the start and end of each cluster depending on the counts. */
    for ( i = 0; i < fanOut; i++ ) {
        dst[i] = offset;
        /* for aligning partition-outputs to cacheline: */
        offset += ALIGN_NUMTUPLES(hist[i]);
    }

    input = inRel->tuples;
    /* copy tuples to their corresponding clusters at appropriate offsets */
    for( i = 0; i < ntuples; i++ ){
        uint32_t  idx     = HASH_BIT_MODULO(input->key, M, R);
        /* store in the cache-resident buffer first */
        uint32_t  slot    = buffer[idx].data.slot;
        tuple_t * tup     = (tuple_t *)(buffer + idx);
        tup[slot] = *input;
        input ++;
        slot++;

        if(slot == TUPLESPERCACHELINE){
            store_nontemp_64B((output+dst[idx]), (buffer+idx));
            slot = 0;
            dst[idx] += TUPLESPERCACHELINE;
        }
        buffer[idx].data.slot = slot;
    }

    /* flush the remainder tuples in the buffer */
    for ( i = 0; i < fanOut; i++ ) {
        uint32_t  num  = buffer[i].data.slot;
        if(num > 0){
            tuple_t * dest = output + dst[i];

            for(uint32_t j = 0; j < num; j++) {
                dest[j] = buffer[i].data.tuples[j];
            }
        }
    }
}

void
radix_cluster_optimized_V2(relation_t * restrict outRel,
                           relation_t * restrict inRel,
                           int32_t * restrict hist,
                           int R,
                           int D)
{
    uint32_t i;
    uint32_t offset = 0;
    const uint32_t M       = ((1 << D) - 1) << R;
    const uint32_t fanOut  = 1 << D;
    const uint32_t ntuples = inRel->num_tuples;

    tuple_t * input  = inRel->tuples;
    tuple_t * output = outRel->tuples;

    cacheline_t buffer[fanOut] __attribute__((aligned(CACHE_LINE_SIZE)));

    for( i = 0; i < ntuples; i++ ){
        uint32_t idx = HASH_BIT_MODULO(input->key, M, R);
        hist[idx]++;
        input++;
    }

    /* determine the start and end of each cluster depending on the counts. */
    for ( i = 0; i < fanOut; i++ ) {
        buffer[i].data.slot = offset;
        /* for aligning partition-outputs to cacheline: */
        /* hist[i] = (hist[i] + (64/sizeof(tuple_t))) & ~((64/sizeof(tuple_t))-1); */
        //offset += hist[i];
        offset += ALIGN_NUMTUPLES(hist[i]);
    }

    input = inRel->tuples;
    /* copy tuples to their corresponding clusters at appropriate offsets */
    for( i = 0; i < ntuples; i++ ){
        uint32_t  idx     = HASH_BIT_MODULO(input->key, M, R);
        /* store in the cache-resident buffer first */
        uint32_t  slot    = buffer[idx].data.slot;
        tuple_t * tup     = (tuple_t *)(buffer + idx);
        uint32_t  slotMod = (slot) & (TUPLESPERCACHELINE - 1); /* % operator */
        tup[slotMod] = *input;
        input ++;

        if(slotMod == (TUPLESPERCACHELINE-1)){
            /* uintptr_t p = (uintptr_t)(output+slot-TUPLESPERCACHELINE); */
            /* if(p % 64 != 0){ */
            /*     printf("there is a problem\n"); */
            /* } */
            store_nontemp_64B((output+slot-(TUPLESPERCACHELINE-1)), (buffer+idx));
        }
        buffer[idx].data.slot = slot+1;
    }

    /* flush the remainder tuples in the buffer */
    for ( i = 0; i < fanOut; i++ ) {
        uint32_t  slot = buffer[i].data.slot;
        uint32_t  num  = (slot) & (TUPLESPERCACHELINE - 1);
        if(num > 0){
            tuple_t * dest = output + slot - num;

            for(uint32_t j = 0; j < num; j++) {
                dest[j] = buffer[i].data.tuples[j];
            }
        }
    }
}

/**
 * Partition a given input relation into fanout=2^radixbits partitions using
 * radix clustering algorithm with least significant `nbits`.
 *
 * @param partitions output partitions, an array of relation ptrs of size fanout
 * @param input input relation
 * @param output used for writing out partitioning results
 * @param radixbits number of bits to use for partitioning
 * @param shiftbits shift amount to left
 *
 * @return
 */
void
partition_relation(relation_t ** partitions,
                   relation_t * input,
                   relation_t * output,
                   int radixbits,
                   int shiftbits)
{
    int i;
    uint32_t offset = 0;
    const int fanOut = 1 << radixbits;
    int32_t * hist;

    hist = (int32_t*) calloc(fanOut+1, sizeof(int32_t));

    radix_cluster(output, input, hist, shiftbits, radixbits);//18-->0: for partition_test

    for(i = 0; i < fanOut; i++) {
        relation_t * part = partitions[i];

        part->num_tuples = hist[i];
        part->tuples = output->tuples + offset;

        offset += hist[i];
    }

    free(hist);
}

void
partition_relation_optimized(relation_t ** partitions,
                             relation_t * input,
                             relation_t * output,
                             uint32_t nbits,
                             uint32_t shiftbits)
{
    int i;
    uint32_t offset = 0;
    const int fanOut = 1 << nbits;

    int32_t * hist, * histAligned;
    hist = (int32_t*) calloc(fanOut + 16, sizeof(int32_t));
    histAligned = (int32_t *) ALIGNPTR(hist, 64);

    radix_cluster_optimized(output, input, histAligned, shiftbits, nbits);

    for(i = 0; i < fanOut; i++) {
        relation_t * part = partitions[i];
        part->num_tuples = histAligned[i];
        part->tuples = output->tuples + offset;

        offset += ALIGN_NUMTUPLES(histAligned[i]);
    }
    free(hist);
}

void
partition_relation_optimized_V2(relation_t ** partitions,
                                relation_t * input,
                                relation_t * output,
                                uint32_t nbits,
                                uint32_t shiftbits)
{
    int i;
    uint32_t offset = 0;
    const int fanOut = 1 << nbits;

    int32_t * hist, * histAligned;
    hist = (int32_t*) calloc(fanOut + 16, sizeof(int32_t));
    histAligned = (int32_t *) ALIGNPTR(hist, 64);

    /* int32_t histAligned[fanOut+1] __attribute__((aligned(CACHE_LINE_SIZE))); */
    /* memset(histAligned, 0, (fanOut+1)*sizeof(int32_t)); */

    radix_cluster_optimized_V2(output, input, histAligned, shiftbits, nbits);

    for(i = 0; i < fanOut; i++) {
        relation_t * part = partitions[i];
        part->num_tuples = histAligned[i];
        part->tuples = output->tuples + offset;

        offset += ALIGN_NUMTUPLES(histAligned[i]);
    }

    free(hist);
}

void
baseline_histogram_memcpy(relation_t * restrict outRel,
                          relation_t * restrict inRel,
                          int32_t * restrict hist,
                          int R,
                          int D)
{
    uint32_t i;
    uint32_t M = ((1 << D) - 1) << R;
    uint32_t offset;
    uint32_t fanOut = 1 << D;

    /* the following are fixed size when D is same for all the passes,
       and can be re-used from call to call. Allocating in this function
       just in case D differs from call to call. */
    UNUSED uint32_t dst[fanOut];

    /* count tuples per cluster */
    for( i=0; i < inRel->num_tuples; i++ ){
        uint32_t idx = HASH_BIT_MODULO(inRel->tuples[i].key, M, R);
        hist[idx]++;
    }
    offset = 0;
    /* determine the start and end of each cluster depending on the counts. */
    for ( i=0; i < fanOut; i++ ) {
        /* dst[i]      = outRel->tuples + offset; */
        /* determine the beginning of each partitioning by adding some
           padding to avoid L1 conflict misses during scatter. */
        dst[i] = offset;/* + i * SMALL_PADDING_TUPLES; */
        offset += hist[i];
    }
    /* just memcpy, no scatter */
    memcpy(outRel->tuples, inRel->tuples, inRel->num_tuples * sizeof(tuple_t));
}

void
histogram_memcpy_bench(relation_t ** partitions,
                       relation_t * input,
                       relation_t * output,
                       uint32_t nbits)
{
    const int fanOut = 1 << nbits;
    int32_t * hist;

    hist = (int32_t*) calloc(fanOut+1, sizeof(int32_t));

    baseline_histogram_memcpy(output, input, hist, 0, nbits);

    free(hist);
}

