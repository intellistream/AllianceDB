/* @version $Id $ */
#include <string.h> /* memcpy(), TODO: replace with simd_memcpy() */

#include "avx_multiwaymerge.h"
#include "avxcommon.h"
#include "../affinity/memalloc.h"

/* just make the code compile without AVX support */
#ifndef HAVE_AVX
#include "avxintrin_emu.h"
#endif

/*
#define CLS CACHE_LINE_SIZE
#define ALIGNDOWN(N) ((N) & ~((CLS/sizeof(tuple_t))-1))
*/

typedef struct merge_node_t merge_node_t;
struct merge_node_t {
    tuple_t *buffer;
    volatile uint32_t count;
    volatile uint32_t head;
    volatile uint32_t tail;
} __attribute__((packed));

/*#define MERGENODETEST 1*/
#ifdef MERGENODETEST
#include "testutil.h" /* is_sorted_tuples_noassert() */
/** For testing merge_node_t sortedness */
static int check_merge_node_sorted(merge_node_t * node, uint32_t fifosize);
#endif

static inline int
min(int x, int y) {
    return ((x < y) ? x : y);
}

#if 0
static int
is_sorted_helper(int64_t * items, uint64_t nitems)
{
#if defined(KEY_8B)

    intkey_t curr = 0;
    uint64_t i;

    tuple_t * tuples = (tuple_t *) items;

    for (i = 0; i < nitems; i++)
    {
        if(tuples[i].key < curr)
            return 0;

        curr = tuples[i].key;
    }

    return 1;

#else

    /* int64_t curr = 0; */
    /* uint64_t i; */

    /* for (i = 0; i < nitems; i++) */
    /* { */
    /*     if(items[i] < curr){ */
    /*         printf("ERR: item[%d]=%llu ; item[%d]=%llu\n", i, items[i], i-1, curr); */
    /*         /\* return 0; *\/ */
    /*         exit(0); */
    /*     } */

    /*     curr = items[i]; */
    /* } */

    /* return 1; */

    /* int64_t curr = 0; */
    /* uint64_t i; */
    /* int warned = 0; */
    /* for (i = 0; i < nitems; i++) */
    /* { */
    /*     if(items[i] == curr) { */
    /*         if(!warned){ */
    /*             printf("[WARN ] Equal items, still ok... "\ */
    /*                    "item[%d]=%lld is equal to item[%d]=%lld\n", */
    /*                    i, items[i], i-1, curr); */
    /*             warned =1; */
    /*         } */
    /*     } */
    /*     else if(items[i] < curr){ */
    /*         printf("[ERROR] item[%d]=%lld is less than item[%d]=%lld\n", */
    /*                i, items[i], i-1, curr); */
    /*         return 0; */
    /*     } */

    /*     curr = items[i]; */
    /* } */

    /* return 1; */

    intkey_t curr = 0;
    uint64_t i;
    int warned = 0;
    tuple_t * tuples = (tuple_t *)items;
    for (i = 0; i < nitems; i++)
    {
        if(tuples[i].key == curr) {
            if(!warned){
                printf("[WARN ] Equal items, still ok... "\
                       "item[%d].key=%d is equal to item[%d].key=%d\n",
                       i, tuples[i].key, i-1, curr);
                warned =1;
            }
        }
        else if(tuples[i].key < curr){
            printf("[ERROR] item[%d].key=%d is less than item[%d].key=%d\n",
                   i, tuples[i].key, i-1, curr);
            return 0;
        }

        curr = tuples[i].key;
    }

    return 1;

#endif
}
#endif

/*******************************************************************************
 *                                                                             *
 *                      AVX Merge Declerations                                 *
 *                                                                             *
 *******************************************************************************/

/**
 * Read two inputs into a merge node using AVX.
 *
 * @param node
 * @param inA
 * @param inB
 * @param lenA
 * @param lenB
 * @param fifosize
 *
 * @return
 */
uint32_t
readmerge_parallel_decomposed(merge_node_t *node,
                              tuple_t **inA,
                              tuple_t **inB,
                              uint32_t lenA,
                              uint32_t lenB,
                              uint32_t fifosize);

/**
 * Merge two children merge nodes using ring-buffer decomposing with AVX.
 *
 * @param node
 * @param right
 * @param left
 * @param fifosize
 * @param done
 */
void
merge_parallel_decomposed(merge_node_t *node,
                          merge_node_t *right,
                          merge_node_t *left,
                          uint32_t fifosize,
                          uint8_t rightdone, uint8_t leftdone);

/**
 * Do a merge on right and left nodes and store into the output buffer.
 * Merge should be done using SIMD merge. Uses 16x16 AVX merge routine.
 *
 * @param right right node
 * @param left left left node
 * @param output output buffer
 * @param fifosize size of the fifo queue
 */
uint64_t
mergestore_parallel_decomposed(merge_node_t *right,
                               merge_node_t *left,
                               tuple_t **output,
                               uint32_t fifosize,
                               uint8_t rightdone, uint8_t leftdone);



/*******************************************************************************
 *                                                                             *
 *                               Implementations                               *
 *                                                                             *
 *******************************************************************************/

/*******************************************************************************
 *            AVX Multi-Way Merge with Ring-Buffer Decomposition               *
 *******************************************************************************/
uint64_t
avx_multiway_merge(tuple_t *output,
                   relation_t **parts,
                   uint32_t nparts,
                   tuple_t *fifobuffer,
                   uint32_t bufntuples) {
    uint64_t totalmerged = 0;
    uint32_t nfifos = nparts - 2;
    uint32_t totalfifosize = bufntuples - nparts -
                             (nfifos * sizeof(merge_node_t)
                              + nfifos * sizeof(uint8_t)
                              + nparts * sizeof(relation_t)
                              + sizeof(tuple_t) - 1) / sizeof(tuple_t);

    uint32_t fifosize = totalfifosize / nfifos;
    /* align ring-buffer size to be multiple of 64-Bytes */
    /* fifosize = ALIGNDOWN(fifosize); */
    /* totalfifosize = fifosize * nfifos; */

    merge_node_t *nodes = (merge_node_t *) (fifobuffer + totalfifosize);
    uint8_t *done = (uint8_t *) (nodes + nfifos);

    /* printf("[INFO ] fifosize = %d, totalfifosize = %d tuples, %.2lf KiB\n", */
    /*        fifosize, totalfifosize, totalfifosize*sizeof(tuple_t)/1024.0); */

    for (uint32_t i = 0; i < nfifos; i++) {
        nodes[i].buffer = fifobuffer + fifosize * i;
        nodes[i].count = 0;
        nodes[i].head = 0;
        nodes[i].tail = 0;
        done[i] = 0;
    }

    uint32_t finished = 0;
    const uint32_t readthreshold = fifosize / 2;

    while (!finished) {
        finished = 1;
        int m = nfifos - 1;

        /* first iterate through leafs and read as much data as possible */
        for (uint32_t c = 0; c < nparts; c += 2, m--) {
            if (!done[m] && (nodes[m].count < readthreshold)) {

                uint32_t A = c;
                uint32_t B = c + 1;
                tuple_t *inA = parts[A]->tuples;
                tuple_t *inB = parts[B]->tuples;

                uint32_t nread;

                /*
                if(!check_merge_node_sorted(&nodes[m], fifosize)){
                     printf("before read:: Node not sorted\n");
                     exit(0);
                }*/

                nread = readmerge_parallel_decomposed(&nodes[m], &inA, &inB,
                                                      parts[A]->num_tuples,
                                                      parts[B]->num_tuples,
                                                      fifosize);

                /*
                if(!check_merge_node_sorted(&nodes[m], fifosize)){
                     printf("after read:: Node not sorted\n");
                     exit(0);
                }*/

                parts[A]->num_tuples -= (inA - parts[A]->tuples);
                parts[B]->num_tuples -= (inB - parts[B]->tuples);

                parts[A]->tuples = inA;
                parts[B]->tuples = inB;

                done[m] = (nread == 0
                           || ((parts[A]->num_tuples == 0)
                               && (parts[B]->num_tuples == 0)));

                finished &= done[m];
            }
        }

        /* now iterate inner nodes and do merge for ready nodes */
        for (; m >= 0; m--) {
            if (!done[m]) {
                int r = 2 * m + 2;
                int l = r + 1;
                merge_node_t *right = &nodes[r];
                merge_node_t *left = &nodes[l];

                uint8_t children_done = (done[r] | done[l]);

                if ((children_done || nodes[m].count < readthreshold)
                    && nodes[m].count < fifosize) {
                    if (children_done || (right->count >= readthreshold
                                          && left->count >= readthreshold)) {

                        /* if(!check_node_sorted(right, fifosize)) */
                        /*     printf("Right Node not sorted\n"); */

                        /* if(!check_node_sorted(left, fifosize)) */
                        /*     printf("Left Node not sorted\n"); */

                        /* do a merge on right and left */
                        /* TODO: FIXME: "(done[r] & done[l])" doesn't work for fan-in of 128 */
                        /* TODO: FIXME: "children_done" doesn't work for fan-in of 8 */
                        merge_parallel_decomposed(&nodes[m], right, left,
                                                  fifosize,
                                                  done[r], done[l]//children_done /* full-merge? */
                        );

                        /* if(!check_node_sorted(right, fifosize)) */
                        /*     printf("After merge- Right Node not sorted\n"); */

                        /* if(!check_node_sorted(left, fifosize)) */
                        /*     printf("After merge- Left Node not sorted\n"); */

                        /*
                        if(!check_merge_node_sorted(&nodes[m], fifosize))
                             printf("After merge - Node not sorted\n");*/
                    }
                    done[m] = (done[r] & done[l]) &&
                              (right->count == 0 && left->count == 0);
                }

                finished &= done[m];
            }
        }

        totalmerged +=
                /* finally iterate for the root node and store data */
                mergestore_parallel_decomposed(&nodes[0], &nodes[1], &output, fifosize,
                                               done[0], done[1] /* full-merge? */
                );
    }

    /* free(fifobuffer); */
    return totalmerged;
}

/** This kernel takes two lists from ring buffers that can be linearly merged
    without a modulo operation on indices */
inline void __attribute__((always_inline))
merge16kernel(tuple_t *restrict A, tuple_t *restrict B, tuple_t *restrict Out,
              uint32_t *ri, uint32_t *li, uint32_t *oi, uint32_t *outnslots,
              uint32_t rend, uint32_t lend) {
    int32_t lenA = rend - *ri, lenB = lend - *li;
    int32_t nslots = *outnslots;
    int32_t remNslots = nslots & 0xF;
    int32_t lenA16 = lenA & ~0xF, lenB16 = lenB & ~0xF;

    uint32_t rii = *ri, lii = *li, oii = *oi;
    nslots -= remNslots;

    if (nslots > 0 && lenA16 > 16 && lenB16 > 16) {

        register block16 *inA = (block16 *) A;
        register block16 *inB = (block16 *) B;
        block16 *const endA = (block16 *) (A + lenA) - 1;
        block16 *const endB = (block16 *) (B + lenB) - 1;

        block16 *outp = (block16 *) Out;

        register block16 *next = inB;

        __m256d outreg1l1, outreg1l2, outreg1h1, outreg1h2;
        __m256d outreg2l1, outreg2l2, outreg2h1, outreg2h2;

        __m256d regAl1, regAl2, regAh1, regAh2;
        __m256d regBl1, regBl2, regBh1, regBh2;

        LOAD8U(regAl1, regAl2, inA);
        LOAD8U(regAh1, regAh2, ((block8 *) (inA) + 1));
        inA++;

        LOAD8U(regBl1, regBl2, inB);
        LOAD8U(regBh1, regBh2, ((block8 *) (inB) + 1));
        inB++;

        BITONIC_MERGE16(outreg1l1, outreg1l2, outreg1h1, outreg1h2,
                        outreg2l1, outreg2l2, outreg2h1, outreg2h2,
                        regAl1, regAl2, regAh1, regAh2,
                        regBl1, regBl2, regBh1, regBh2);

        /* store outreg1 */
        STORE8U(outp, outreg1l1, outreg1l2);
        STORE8U(((block8 *) outp + 1), outreg1h1, outreg1h2);
        nslots -= 16;
        outp++;

        while (nslots > 0 && inA < endA && inB < endB) {

            nslots -= 16;
            /** The inline assembly below does exactly the following code: */
            /* Option 3: with assembly */
            IFELSECONDMOVE(next, inA, inB, 128);

            regAl1 = outreg2l1;
            regAl2 = outreg2l2;
            regAh1 = outreg2h1;
            regAh2 = outreg2h2;

            LOAD8U(regBl1, regBl2, next);
            LOAD8U(regBh1, regBh2, ((block8 *) next + 1));

            BITONIC_MERGE16(outreg1l1, outreg1l2, outreg1h1, outreg1h2,
                            outreg2l1, outreg2l2, outreg2h1, outreg2h2,
                            regAl1, regAl2, regAh1, regAh2,
                            regBl1, regBl2, regBh1, regBh2);

            /* store outreg1 */
            STORE8U(outp, outreg1l1, outreg1l2);
            STORE8U(((block8 *) outp + 1), outreg1h1, outreg1h2);
            outp++;
        }

        {
            /* flush the register to one of the lists */
            int64_t /*tuple_t*/ hireg[4] __attribute__((aligned(32)));
            _mm256_store_pd((double *) hireg, outreg2h2);

            /* if(((tuple_t *)inA)->key >= hireg[3].key){*/
            if (*((int64_t *) inA) >= hireg[3]) {
                /* store the last remaining register values to A */
                inA--;
                STORE8U(inA, outreg2l1, outreg2l2);
                STORE8U(((block8 *) inA + 1), outreg2h1, outreg2h2);
            } else {
                /* store the last remaining register values to B */
                inB--;
                STORE8U(inB, outreg2l1, outreg2l2);
                STORE8U(((block8 *) inB + 1), outreg2h1, outreg2h2);
            }
        }

        rii = *ri + ((tuple_t *) inA - A);
        lii = *li + ((tuple_t *) inB - B);
        oii = *oi + ((tuple_t *) outp - Out);

        A = (tuple_t *) inA;
        B = (tuple_t *) inB;
        Out = (tuple_t *) outp;
    }
    nslots += remNslots;

    /* serial-merge */
    while ((nslots > 0 && rii < rend && lii < lend)) {
        tuple_t *in = B;
        uint32_t cmp = *((int64_t *) A) < *((int64_t *) B);/*(A->key < B->key);*/
        uint32_t notcmp = !cmp;

        rii += cmp;
        lii += notcmp;

        if (cmp)
            in = A;

        nslots--;
        oii++;
        *Out = *in;
        Out++;
        A += cmp;
        B += notcmp;
    }

    *ri = rii;
    *li = lii;
    *oi = oii;
    *outnslots = nslots;
}

/** This kernel takes two lists from ring buffers that can be linearly merged
    without a modulo operation on indices */
inline void __attribute__((always_inline))
merge8kernel(tuple_t *restrict A, tuple_t *restrict B, tuple_t *restrict Out,
             uint32_t *ri, uint32_t *li, uint32_t *oi, uint32_t *outnslots,
             uint32_t rend, uint32_t lend) {
    int32_t lenA = rend - *ri, lenB = lend - *li;
    int32_t nslots = *outnslots;
    int32_t remNslots = nslots & 0x7;
    int32_t lenA8 = lenA & ~0x7, lenB8 = lenB & ~0x7;

    uint32_t rii = *ri, lii = *li, oii = *oi;
    nslots -= remNslots;

    if (nslots > 0 && lenA8 > 8 && lenB8 > 8) {

        register block8 *inA = (block8 *) A;
        register block8 *inB = (block8 *) B;
        block8 *const endA = (block8 *) (A + lenA) - 1;
        block8 *const endB = (block8 *) (B + lenB) - 1;

        block8 *outp = (block8 *) Out;

        register block8 *next = inB;
        register __m256d outreg1l, outreg1h;
        register __m256d outreg2l, outreg2h;

        register __m256d regAl, regAh;
        register __m256d regBl, regBh;

        LOAD8U(regAl, regAh, inA);
        LOAD8U(regBl, regBh, next);

        inA++;
        inB++;

        BITONIC_MERGE8(outreg1l, outreg1h, outreg2l, outreg2h,
                       regAl, regAh, regBl, regBh);

        /* store outreg1 */
        STORE8U(outp, outreg1l, outreg1h);
        outp++;
        nslots -= 8;

        while (nslots > 0 && inA < endA && inB < endB) {

            nslots -= 8;
            /** The inline assembly below does exactly the following code: */
            /* Option 3: with assembly */
            IFELSECONDMOVE(next, inA, inB, 64);

            regAl = outreg2l;
            regAh = outreg2h;
            LOAD8U(regBl, regBh, next);

            BITONIC_MERGE8(outreg1l, outreg1h, outreg2l, outreg2h,
                           regAl, regAh, regBl, regBh);

            /* store outreg1 */
            STORE8U(outp, outreg1l, outreg1h);
            outp++;
        }

        {
            /* flush the register to one of the lists */
            int64_t /*tuple_t*/ hireg[4] __attribute__((aligned(32)));
            _mm256_store_pd((double *) hireg, outreg2h);

            /* if(((tuple_t *)inA)->key >= hireg[3].key){*/
            if (*((int64_t *) inA) >= hireg[3]) {
                /* store the last remaining register values to A */
                inA--;
                STORE8U(inA, outreg2l, outreg2h);
            } else {
                /* store the last remaining register values to B */
                inB--;
                STORE8U(inB, outreg2l, outreg2h);
            }
        }

        rii = *ri + ((tuple_t *) inA - A);
        lii = *li + ((tuple_t *) inB - B);
        oii = *oi + ((tuple_t *) outp - Out);

        A = (tuple_t *) inA;
        B = (tuple_t *) inB;
        Out = (tuple_t *) outp;
    }
    nslots += remNslots;

    /* serial-merge */
    while ((nslots > 0 && rii < rend && lii < lend)) {
        tuple_t *in = B;
        uint32_t cmp = *((int64_t *) A) < *((int64_t *) B);/*(A->key < B->key);*/
        uint32_t notcmp = !cmp;

        rii += cmp;
        lii += notcmp;

        if (cmp)
            in = A;

        nslots--;
        oii++;
        *Out = *in;
        Out++;
        A += cmp;
        B += notcmp;
    }

    *ri = rii;
    *li = lii;
    *oi = oii;
    *outnslots = nslots;
}

inline void __attribute__((always_inline))
parallel_read(tuple_t **A, tuple_t **B, tuple_t **Out,
              uint32_t *ri, uint32_t *li, uint32_t *oi, uint32_t *outnslots,
              uint32_t lenA, uint32_t lenB) {
    uint32_t _ri = *ri, _li = *li, _oi = *oi;

    merge16kernel(*A, *B, *Out, ri, li, oi, outnslots, lenA, lenB);

    *A += (*ri - _ri);
    *B += (*li - _li);
    *Out += (*oi - _oi);
}

uint32_t
readmerge_parallel_decomposed(merge_node_t *node,
                              tuple_t **inA,
                              tuple_t **inB,
                              uint32_t lenA,
                              uint32_t lenB,
                              uint32_t fifosize) {
    uint32_t nodecount = node->count;
    uint32_t nodehead = node->head;
    uint32_t nodetail = node->tail;
    tuple_t *Out = node->buffer;

    tuple_t *A = *inA;
    tuple_t *B = *inB;

    /* size related variables */
    uint32_t ri = 0, li = 0, outnslots;

    uint32_t oi = nodetail, oend;
    uint32_t oi2 = 0, oend2 = 0;

    if (nodehead > nodetail) {
        oend = nodehead;
    } else {
        oend = fifosize;
        oi2 = 0;
        oend2 = nodehead;
    }
    outnslots = oend - oi;

    Out += oi;

    /* fill first chunk of the node buffer */
    parallel_read(&A, &B, &Out,
                  &ri, &li, &oi, &outnslots,
                  lenA, lenB);

    nodecount += (oi - nodetail);
    nodetail = ((oi == fifosize) ? 0 : oi);

    if (outnslots == 0 && oend2 != 0) {
        outnslots = oend2 - oi2;
        Out = node->buffer;

        /* fill second chunk of the node buffer */
        parallel_read(&A, &B, &Out,
                      &ri, &li, &oi2, &outnslots,
                      lenA, lenB);

        nodecount += oi2;
        nodetail = ((oi2 == fifosize) ? 0 : oi2);
    }

    if (nodecount < fifosize) {
        outnslots = fifosize - nodecount;
        oi = nodetail;
        oend = (nodetail + outnslots);
        if (oend > fifosize)
            oend = fifosize;
        outnslots = oend - oi;

        if (ri < lenA) {
            do {
                while (outnslots > 0 && ri < lenA) {
                    outnslots--;
                    oi++;
                    *Out = *A;
                    ri++;
                    A++;
                    Out++;
                    nodecount++;
                    nodetail++;
                }

                if (oi == oend) {
                    oi = 0;
                    oend = nodehead;
                    if (nodetail >= fifosize) {
                        nodetail = 0;
                        Out = node->buffer;
                    }
                    outnslots = oend - oi;
                }

            } while (nodecount < fifosize && ri < lenA);
        } else if (li < lenB) {
            do {
                while (outnslots > 0 && li < lenB) {
                    outnslots--;
                    oi++;
                    *Out = *B;
                    li++;
                    B++;
                    Out++;
                    nodecount++;
                    nodetail++;
                }

                if (oi == oend) {
                    oi = 0;
                    oend = nodehead;
                    if (nodetail >= fifosize) {
                        nodetail = 0;
                        Out = node->buffer;
                    }
                    outnslots = oend - oi;
                }
            } while (nodecount < fifosize && li < lenB);
        }
    }
    *inA = A;
    *inB = B;

    node->tail = nodetail;
    node->count = nodecount;

    /* if(!check_node_sorted(node, fifosize)) */
    /*     printf("in merge_read() - Node not sorted\n"); */

    return (ri + li);
}

/**
 * Copy from src to dst using SIMD instructions.
 * @param dst
 * @param src
 * @param sz
 */
void
simd_memcpy(void *dst, void *src, size_t sz) {
    char *src_ptr = (char *) src;
    char *dst_ptr = (char *) dst;
    char *src_end = (char *) src + sz - 64;
    /* further improvement with aligned load/store */
    for (; src_ptr <= src_end; src_ptr += 64, dst_ptr += 64) {
        __asm volatile(
        "movdqu 0(%0) , %%xmm0;  "
        "movdqu 16(%0), %%xmm1;  "
        "movdqu 32(%0), %%xmm2;  "
        "movdqu 48(%0), %%xmm3;  "
        "movdqu %%xmm0, 0(%1) ;  "
        "movdqu %%xmm1, 16(%1) ;  "
        "movdqu %%xmm2, 32(%1) ;  "
        "movdqu %%xmm3, 48(%1) ;  "
        ::"r"(src_ptr), "r"(dst_ptr));
    }

    /* copy remainders */
    src_end += 64;
    if (src_ptr < src_end) {
        memcpy(dst_ptr, src_ptr, (src_end - src_ptr));
    }
}

/**
 * Copy all tuples from src node to dest node.
 *
 * @param dest
 * @param src
 * @param fifosize
 */
void
direct_copy_avx(merge_node_t *dest, merge_node_t *src, uint32_t fifosize) {
    /* make sure dest has space and src has tuples */
    //assert(dest->count < fifosize);
    //assert(src->count > 0);

    /* Cases for the ring-buffer : 1) head < tail 2) head > tail */

    uint32_t dest_block_start[2];
    uint32_t dest_block_size[2];

    uint32_t src_block_start[2];
    uint32_t src_block_size[2];

    if (dest->head <= dest->tail) {/* Case 1) */
        /* dest block-1 */
        dest_block_start[0] = dest->tail;
        dest_block_size[0] = fifosize - dest->tail;
        /* dest block-2 */
        dest_block_start[1] = 0;
        dest_block_size[1] = dest->head;
    } else {
        /* Case 2) dest-> head > dest->tail */
        /* dest block-1 */
        dest_block_start[0] = dest->tail;
        dest_block_size[0] = dest->head - dest->tail;
        /* no block-2 */
        dest_block_size[1] = 0;
    }

    if (src->head >= src->tail) {/* Case 2) */
        /* src block-1 */
        src_block_start[0] = src->head;
        src_block_size[0] = fifosize - src->head;
        /* src block-2 */
        src_block_start[1] = 0;
        src_block_size[1] = src->tail;
    } else {
        /* Case 1) src-> head < src->tail */
        /* src block-1 */
        src_block_start[0] = src->head;
        src_block_size[0] = src->tail - src->head;
        /* no block-2 */
        src_block_size[1] = 0;
    }

    uint32_t copied = 0;
    for (int i = 0, j = 0; i < 2 && j < 2;) {
        uint32_t copysize = min(dest_block_size[i], src_block_size[j]);

        if (copysize > 0) {
            simd_memcpy(dest->buffer + dest_block_start[i],
                        src->buffer + src_block_start[j],
                        copysize * sizeof(tuple_t));

            dest_block_start[i] += copysize;
            src_block_start[j] += copysize;
            dest_block_size[i] -= copysize;
            src_block_size[j] -= copysize;
            copied += copysize;
        }

        if (dest_block_size[i] == 0)
            i++;

        if (src_block_size[j] == 0)
            j++;
    }

    dest->count += copied;
    dest->tail = (dest->tail + copied) % fifosize;
    src->count -= copied;
    src->head = (src->head + copied) % fifosize;

}

/**
 * Copy all tuples from src node to output array.
 *
 * @param dest pointer to tuple array for output
 * @param src
 * @param fifosize
 *
 * @return number of copied tuples
 */
uint32_t
direct_copy_to_output_avx(tuple_t *dest, merge_node_t *src, uint32_t fifosize) {
    /* make sure dest has space and src has tuples */
    //assert(src->count > 0);

    /* Cases for the ring-buffer : 1) head < tail 2) head > tail */
    uint32_t src_block_start[2];
    uint32_t src_block_size[2];

    if (src->head >= src->tail) {/* Case 2) */
        /* src block-1 */
        src_block_start[0] = src->head;
        src_block_size[0] = fifosize - src->head;
        /* src block-2 */
        src_block_start[1] = 0;
        src_block_size[1] = src->tail;
    } else {
        /* Case 1) src-> head < src->tail */
        /* src block-1 */
        src_block_start[0] = src->head;
        src_block_size[0] = src->tail - src->head;
        /* no block-2 */
        src_block_size[1] = 0;
    }

    uint32_t copied = 0;
    for (int j = 0; j < 2; j++) {
        uint32_t copysize = src_block_size[j];

        if (copysize > 0) {
            simd_memcpy((void *) (dest + copied),
                        src->buffer + src_block_start[j],
                        copysize * sizeof(tuple_t));

            copied += copysize;
        }
    }

    src->count -= copied;
    src->head = (src->head + copied) % fifosize;

    return copied;
}

void
merge_parallel_decomposed(merge_node_t *node,
                          merge_node_t *right,
                          merge_node_t *left,
                          uint32_t fifosize,
                          uint8_t rightdone, uint8_t leftdone) {
    /* directly copy tuples from right or left if one of them done but not the other */
    if (rightdone && right->count == 0) {
        if (!leftdone && left->count > 0) {
            direct_copy_avx(node, left, fifosize);
            return;
        }
    } else if (leftdone && left->count == 0) {
        if (!rightdone && right->count > 0) {
            direct_copy_avx(node, right, fifosize);
            return;
        }
    }

    /* both done? */
    uint8_t done = rightdone & leftdone;

    uint32_t righttail = right->tail;
    uint32_t rightcount = right->count;
    uint32_t righthead = right->head;
    uint32_t lefttail = left->tail;
    uint32_t leftcount = left->count;
    uint32_t lefthead = left->head;

    int rcases = 0, lcases = 0;
    uint32_t outnslots;

    uint32_t oi = node->tail, oend;
    if (node->head > node->tail) {
        oend = node->head;
    } else {
        oend = fifosize;
    }

    outnslots = oend - oi;

    uint32_t ri = righthead, rend;
    if (righthead >= righttail) {
        rend = fifosize;
        rcases = 1;
    } else {
        rend = righttail;
    }

    uint32_t li = lefthead, lend;
    if (lefthead >= lefttail) {
        lend = fifosize;
        lcases = 1;
    } else {
        lend = lefttail;
    }

    while (node->count < fifosize
           && (rightcount > 0 && leftcount > 0)) {
        register tuple_t *R = right->buffer + ri;
        register tuple_t *L = left->buffer + li;
        register tuple_t *Out = node->buffer + oi;

        /* serialmergekernel(R, L, Out, &ri, &li, &oi, &outnslots, rend, lend); */
        merge16kernel(R, L, Out, &ri, &li, &oi, &outnslots, rend, lend);

        node->count += (oi - node->tail);
        node->tail = ((oi == fifosize) ? 0 : oi);
        rightcount -= (ri - righthead);
        righthead = ((ri == fifosize) ? 0 : ri);
        leftcount -= (li - lefthead);
        lefthead = ((li == fifosize) ? 0 : li);


        if (oi == oend) {
            oi = 0;
            oend = node->head;
            outnslots = oend - oi;
        }

        if (rcases > 0 && ri == rend) {
            ri = 0;
            rend = righttail;
            rcases = 0;
        }

        if (lcases > 0 && li == lend) {
            li = 0;
            lend = lefttail;
            lcases = 0;
        }

    }

    /* not possible until we do not read new tuples anymore */
    if (done && node->count < fifosize) {
        tuple_t *Out = node->buffer + node->tail;

        outnslots = fifosize - node->count;
        oi = node->tail;
        oend = (node->tail + outnslots);
        if (oend > fifosize)
            oend = fifosize;

        outnslots = oend - oi;

        if (rightcount > 0) {
            tuple_t *R = right->buffer + righthead;

            ri = righthead;
            rend = righthead + rightcount;
            if (rend > fifosize)
                rend = fifosize;

            do {
                while (outnslots > 0 && ri < rend) {
                    outnslots--;
                    oi++;
                    ri++;
                    *Out = *R;
                    Out++;
                    R++;
                    node->count++;
                    rightcount--;
                    node->tail++;
                    righthead++;
                }

                /* node->count  += (oi - node->tail); */
                /* node->tail = ((oi == fifosize) ? 0 : oi); */
                /* rightcount -= (ri - righthead); */
                /* righthead = ((ri == fifosize) ? 0 : ri); */

                if (oi == oend) {
                    oi = 0;
                    oend = node->head;
                    if (node->tail >= fifosize) {
                        node->tail = 0;
                        Out = node->buffer;
                    }
                }

                if (rcases > 0 && ri == rend) {
                    ri = 0;
                    rend = righttail;
                    rcases = 0;
                    if (righthead >= fifosize) {
                        righthead = 0;
                        R = right->buffer;
                    }
                }
            } while (outnslots > 0 && rightcount > 0);

        } else if (leftcount > 0) {
            tuple_t *L = left->buffer + lefthead;

            li = lefthead;
            lend = lefthead + leftcount;
            if (lend > fifosize)
                lend = fifosize;

            do {
                while (outnslots > 0 && li < lend) {
                    outnslots--;
                    oi++;
                    li++;
                    *Out = *L;
                    Out++;
                    L++;
                    node->count++;
                    leftcount--;
                    node->tail++;
                    lefthead++;
                }

                /* node->count  += (oi - node->tail); */
                /* node->tail = ((oi == fifosize) ? 0 : oi); */
                /* leftcount -= (li - lefthead); */
                /* lefthead = ((li == fifosize) ? 0 : li); */

                if (oi == oend) {
                    oi = 0;
                    oend = node->head;
                    if (node->tail >= fifosize) {
                        node->tail = 0;
                        Out = node->buffer;
                    }
                }


                if (lcases > 0 && li == lend) {
                    li = 0;
                    lend = lefttail;
                    lcases = 0;
                    if (lefthead >= fifosize) {
                        lefthead = 0;
                        L = left->buffer;
                    }
                }

            } while (outnslots > 0 && leftcount > 0);
        }
    }

    /* if(!check_node_sorted(node, fifosize)) */
    /*     printf("Node not sorted rsz=%d, lsz=%d\n", rsz, lsz); */


    right->count = rightcount;
    right->head = righthead;
    left->count = leftcount;
    left->head = lefthead;

}

/**
 * Merge two merge node buffers to final output using 16-way AVX bitonic merge.
 *
 * @param A right merge node
 * @param B left merge node
 * @param[in,out] Out output array
 * @param[in,out] ri right node buffer index
 * @param[in,out] li left node buffer index
 * @param rend right node buffer end index
 * @param lend left node buffer end index
 */
inline void __attribute__((always_inline))
mergestore16kernel(tuple_t *restrict A, tuple_t *restrict B,
                   tuple_t **Out,
                   uint32_t *ri, uint32_t *li,
                   uint32_t rend, uint32_t lend) {
    int32_t lenA = rend - *ri, lenB = lend - *li;
    int32_t lenA16 = lenA & ~0xF, lenB16 = lenB & ~0xF;

    uint32_t rii = *ri, lii = *li;

    tuple_t *out = *Out;

    // if((uintptr_t)A % 64 != 0)
    //     printf("**** A not aligned = %d\n", (uintptr_t)A % 64);
    // if((uintptr_t)B % 64 != 0)
    //     printf("**** B not aligned = %d\n", (uintptr_t)B % 64);
    // if((uintptr_t)Out % 64 != 0)
    //     printf("**** Out not aligned = %d\n", (uintptr_t)Out % 64);

    if (lenA16 > 16 && lenB16 > 16) {

        register block16 *inA = (block16 *) A;
        register block16 *inB = (block16 *) B;
        block16 *const endA = (block16 *) (A + lenA) - 1;
        block16 *const endB = (block16 *) (B + lenB) - 1;

        block16 *outp = (block16 *) out;

        register block16 *next = inB;

        __m256d outreg1l1, outreg1l2, outreg1h1, outreg1h2;
        __m256d outreg2l1, outreg2l2, outreg2h1, outreg2h2;

        __m256d regAl1, regAl2, regAh1, regAh2;
        __m256d regBl1, regBl2, regBh1, regBh2;

        LOAD8U(regAl1, regAl2, inA);
        LOAD8U(regAh1, regAh2, ((block8 *) (inA) + 1));
        inA++;

        LOAD8U(regBl1, regBl2, inB);
        LOAD8U(regBh1, regBh2, ((block8 *) (inB) + 1));
        inB++;

        BITONIC_MERGE16(outreg1l1, outreg1l2, outreg1h1, outreg1h2,
                        outreg2l1, outreg2l2, outreg2h1, outreg2h2,
                        regAl1, regAl2, regAh1, regAh2,
                        regBl1, regBl2, regBh1, regBh2);

        /* store outreg1 */
        STORE8U(outp, outreg1l1, outreg1l2);
        STORE8U(((block8 *) outp + 1), outreg1h1, outreg1h2);
        outp++;

        while (inA < endA && inB < endB) {

            /** The inline assembly below does exactly the following code: */
            /* if(*((int64_t *)inA) < *((int64_t *)inB)) { */
            /*     next = inA; */
            /*     inA ++; */
            /* } */
            /* else { */
            /*     next = inB; */
            /*     inB ++; */
            /* } */
            /* Option 3: with assembly */
            IFELSECONDMOVE(next, inA, inB, 128);

            regAl1 = outreg2l1;
            regAl2 = outreg2l2;
            regAh1 = outreg2h1;
            regAh2 = outreg2h2;

            LOAD8U(regBl1, regBl2, next);
            LOAD8U(regBh1, regBh2, ((block8 *) next + 1));

            BITONIC_MERGE16(outreg1l1, outreg1l2, outreg1h1, outreg1h2,
                            outreg2l1, outreg2l2, outreg2h1, outreg2h2,
                            regAl1, regAl2, regAh1, regAh2,
                            regBl1, regBl2, regBh1, regBh2);

            /* store outreg1 */
            STORE8U(outp, outreg1l1, outreg1l2);
            STORE8U(((block8 *) outp + 1), outreg1h1, outreg1h2);
            outp++;
        }

        {
            /* flush the register to one of the lists */
//            int64_t /*tuple_t*/ hireg[4] __attribute__((aligned(16)));

            auto hireg = (int64_t *) malloc_aligned(4 * sizeof(int64_t));

            _mm256_store_pd((double *) hireg, outreg2h2);

            /*if(((tuple_t *)inA)->key >= hireg[3].key){*/
            if (*((int64_t *) inA) >= hireg[3]) {
                /* store the last remaining register values to A */
                inA--;
                STORE8U(inA, outreg2l1, outreg2l2);
                STORE8U(((block8 *) inA + 1), outreg2h1, outreg2h2);
            } else {
                /* store the last remaining register values to B */
                inB--;
                STORE8U(inB, outreg2l1, outreg2l2);
                STORE8U(((block8 *) inB + 1), outreg2h1, outreg2h2);
            }
        }

        rii = *ri + ((tuple_t *) inA - A);
        lii = *li + ((tuple_t *) inB - B);

        A = (tuple_t *) inA;
        B = (tuple_t *) inB;
        out = (tuple_t *) outp;
    }


    /* serial-merge */
    {
        int64_t *in1 = (int64_t *) A;
        int64_t *in2 = (int64_t *) B;
        int64_t *pout = (int64_t *) out;
        while (rii < rend && lii < lend) {
            int64_t *in = in2;
            uint32_t cmp = (*in1 < *in2);
            uint32_t notcmp = !cmp;
            rii += cmp;
            lii += notcmp;
            if (cmp)
                in = in1;
            *pout = *in;
            pout++;
            in1 += cmp;
            in2 += notcmp;
            /*
            if(*in1 < *in2){
                *pout = *in1;
                in1++;
                rii++;
            }
            else {
                *pout = *in2;
                in2++;
                lii++;
            }
            pout ++;
            */
        }
        out = (tuple_t *) pout;
        /* just for tuples, comparison on keys.
    while(rii < rend && lii < lend){
        tuple_t * in = B;
        //uint32_t cmp = (A->key < B->key);
        uint32_t cmp =
        uint32_t notcmp = !cmp;

        rii += cmp;
        lii += notcmp;

        if(cmp)
            in = A;

         *out = *in;
        out ++;
        A += cmp;
        B += notcmp;
    }
         */
    }

    *ri = rii;
    *li = lii;
    *Out = out;
}

/**
 * Do a merge on right and left nodes and store into the output buffer.
 * Merge should be done using SIMD merge. Uses 16x16 AVX merge routine.
 *
 * @param right right node
 * @param left left left node
 * @param output output buffer
 * @param fifosize size of the fifo queue
 */
uint64_t
mergestore_parallel_decomposed(merge_node_t *right,
                               merge_node_t *left,
                               tuple_t **output,
                               uint32_t fifosize,
                               uint8_t rightdone, uint8_t leftdone) {
    /* directly copy tuples from right or left if one of them done but not the other */
    if (rightdone && right->count == 0) {
        if (!leftdone && left->count > 0) {
            uint64_t numcopied = direct_copy_to_output_avx(*output, left, fifosize);
            /*
            if(is_sorted_helper((int64_t*)(*output), numcopied) == 0){
                printf("[ERROR] 1.\n");
            }
            */
            *output += numcopied;
            return numcopied;
        }
    } else if (leftdone && left->count == 0) {
        if (!rightdone && right->count > 0) {
            uint64_t numcopied = direct_copy_to_output_avx(*output, right, fifosize);
            /*
            if(is_sorted_helper((int64_t*)(*output), numcopied) == 0){
                printf("[ERROR] 2.\n");
            }
            */
            *output += numcopied;
            return numcopied;
        }
    }

    tuple_t *Out = *output;
    int rcases = 0, lcases = 0;

    uint32_t ri = right->head, rend;
    if (right->head >= right->tail) {
        rend = fifosize;
        rcases = 1;
    } else {
        rend = right->tail;
    }

    uint32_t li = left->head, lend;
    if (left->head >= left->tail) {
        lend = fifosize;
        lcases = 1;
    } else {
        lend = left->tail;
    }

    while (right->count > 0 && left->count > 0) {

        register tuple_t *R = right->buffer + ri;
        register tuple_t *L = left->buffer + li;

        /* serialmergestorekernel(R, L, &Out, &ri, &li, rend, lend); */
        mergestore16kernel(R, L, &Out, &ri, &li, rend, lend);

        right->count -= (ri - right->head);
        right->head = ((ri == fifosize) ? 0 : ri);
        left->count -= (li - left->head);
        left->head = ((li == fifosize) ? 0 : li);

        if (rcases > 0 && ri == rend) {
            ri = 0;
            rend = right->tail;
            rcases = 0;
        }

        if (lcases > 0 && li == lend) {
            li = 0;
            lend = left->tail;
            lcases = 0;
        }
    }

    /* not possible until we do not read new tuples anymore */
    uint8_t done = rightdone & leftdone;
    if (done) {
        if (right->count > 0) {
            tuple_t *R = right->buffer + right->head;

            ri = right->head;
            rend = right->head + right->count;
            if (rend > fifosize)
                rend = fifosize;

            do {
                // uint32_t sz = rend-ri;
                // memcpy((void*)Out, (void*)R, sz*sizeof(tuple_t));
                // Out += sz;
                // R += sz;
                // right->count -= sz;
                // right->head  += sz;
                // ri = rend;
                while (ri < rend) {
                    ri++;
                    *Out = *R;
                    Out++;
                    R++;
                    right->count--;
                    right->head++;
                }

                if (rcases > 0 && ri == rend) {
                    ri = 0;
                    rend = right->tail;
                    rcases = 0;
                    if (right->head >= fifosize) {
                        right->head = 0;
                        R = right->buffer;
                    }
                }
            } while (right->count > 0);

        } else if (left->count > 0) {
            tuple_t *L = left->buffer + left->head;

            li = left->head;
            lend = left->head + left->count;
            if (lend > fifosize)
                lend = fifosize;

            do {
                // uint32_t sz = lend-li;
                // memcpy((void*)Out, (void*)L, sz*sizeof(tuple_t));
                // Out += sz;
                // L += sz;
                // left->count -= sz;
                // left->head  += sz;
                // li = lend;
                while (li < lend) {
                    li++;
                    *Out = *L;
                    Out++;
                    L++;
                    left->count--;
                    left->head++;
                }

                if (lcases > 0 && li == lend) {
                    li = 0;
                    lend = left->tail;
                    lcases = 0;
                    if (left->head >= fifosize) {
                        left->head = 0;
                        L = left->buffer;
                    }
                }

            } while (left->count > 0);
        }
    }


    uint64_t numstored = (Out - *output);
    /*
    if(is_sorted_helper((int64_t*)(*output), numstored) == 0){
        printf("[ERROR] rightdone=%d leftdone=%d\n", rightdone, leftdone);
    }
    */
    *output = Out;

    return numstored;
}

#ifdef MERGENODETEST
/** For testing merge_node_t sortedness */
static int
check_merge_node_sorted(merge_node_t * node, uint32_t fifosize)
{
    int rv = 1;
    uint32_t oi = node->head, oend;
    uint32_t oi2 = 0, oend2 = 0;

    if(node->tail > node->head) {
        oend = node->tail;
    }
    else {
        oend = fifosize;
        oi2 = 0;
        oend2 = node->tail;
    }

    uint32_t sz = min(node->count, oend-oi);

    if(!is_sorted_tuples_noassert((tuple_t *) (node->buffer+oi), sz)){
        printf("[ERROR] node not sorted!\n");
        rv = 0;
    }

    if(oend != 0) {
        uint32_t sz = min(node->count, oend2-oi2);

        if(!is_sorted_tuples_noassert((tuple_t *) (node->buffer), sz)){
            printf("[ERROR] node not sorted!\n");
            rv = 0;
        }
    }

    return rv;
}
#endif
