#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <limits.h>
//#include <check.h>

#include "testutil.h"
#include "../datagen/generator.h"

#ifndef CACHE_LINE_SIZE
#define CACHE_LINE_SIZE 64
#endif

int
is_sorted_int64(int64_t *items, uint64_t nitems) {
    int64_t curr = -1;
    uint64_t i;
    int warned = 0;
    for (i = 0; i < nitems; i++) {
        if (items[i] < curr) {
            fprintf(stderr,
                    "[ERROR] item[%" PRIu64 "]=%" PRId64 " is less than item[%" PRIu64 "]=%" PRId64 "\n",
                    i, items[i], i - 1, curr);
        }

        if (items[i] == curr) {
            if (!warned) {
                fprintf(stderr, "[WARN ] Equal items, still ok... "\
                       "item[%" PRIu64 "]=%" PRId64 " is equal to item[%" PRIu64 "]=%" PRId64 "\n",
                        i, items[i], i - 1, curr);
                warned = 1;
            }
        } else if (items[i] < curr) {
            return 0;
        }

        curr = items[i];
    }

    return 1;
}

int
is_sorted_int32(int32_t *items, uint64_t nitems) {
    int32_t curr = -1;
    uint64_t i;
    int warned = 0;
    for (i = 0; i < nitems; i++) {
        if (items[i] < curr) {
            fprintf(stderr,
                    "[ERROR] item[%" PRIu64 "]=%d is less than item[%" PRIu64 "]=%d\n",
                    i, items[i], i - 1, curr);
        }

        if (items[i] == curr) {
            if (!warned) {
                fprintf(stderr, "[WARN ] Equal items, still ok... "\
                       "item[%" PRIu64 "]=%d is equal to item[%" PRIu64 "]=%d\n",
                        i, items[i], i - 1, curr);
                warned = 1;
            }
        } else if (items[i] < curr) {
            return 0;
        }

        curr = items[i];
    }

    return 1;
}

int
is_sorted_tuples(tuple_t *items, uint64_t nitems) {
    int32_t curr = -1;
    uint64_t i;
    int warned = 0;
    for (i = 0; i < nitems; i++) {
        if (items[i].key < curr) {
            fprintf(stderr,
                    "[ERROR] item[%" PRIu64 "]=%d is less than item[%" PRIu64 "]=%d\n",
                    i, items[i].key, i - 1, curr);
        }

        if (items[i].key == curr) {
            if (!warned) {
                fprintf(stderr, "[WARN ] Equal items, still ok... "\
                       "item[%" PRIu64 "]=%d is equal to item[%" PRIu64 "]=%d\n",
                        i, items[i].key, i - 1, curr);
                warned = 1;
            }
        } else if (items[i].key < curr) {
            return 0;
        }

        curr = items[i].key;
    }

    return 1;
}

int
is_sorted_tuples_noassert(tuple_t *items, uint64_t nitems) {
    int32_t curr = -1;
    uint64_t i;
//    int warned = 0;
    for (i = 0; i < nitems; i++) {
//        if(items[i].key < curr)
//            fprintf (stderr,
//                    "[ERROR] item[%" PRIu64 "]=%d is less than item[%" PRIu64 "]=%d\n",
//                    i, items[i].key, i-1, curr);

        if (items[i].key == curr) {
//            if(!warned){
//                fprintf(stderr, "[WARN ] Equal items, still ok... "
//                       "item[%" PRIu64 "]=%d is equal to item[%" PRIu64 "]=%d\n",
//                       i, items[i].key, i-1, curr);
//                warned = 1;
//            }
        } else if (items[i].key < curr) {
            return 0;
        }

        curr = items[i].key;
    }

    return 1;
}

int
is_array_equal(int64_t *arr1, int64_t *arr2, uint64_t sz1, uint64_t sz2) {
    if (sz1 != sz2)
        return 0;

    uint64_t i;
    for (i = 0; i < sz1; i++) {
        if (arr1[i] != arr2[i])
            return 0;
    }

    return 1;
}


int64_t *
generate_rand_int64(int num) {
    int j;

    int64_t *A = (int64_t *) malloc(sizeof(int64_t) * num);

    int64_t NaNExpMask = (0x7FFL << 52U);
    int64_t NaNlowbitclear = ~(1L << 52U);
    /* generate random data */
    for (j = 0; j < num; j++) {
        int64_t val = rand();
        val <<= 32;
        val |= rand();
        A[j] = val;
        //A[j] = rand() % LONG_MAX;
        /* avoid NaN in values */
        int64_t *bitpattern = (int64_t *) &(A[j]);
        if (((*bitpattern) & NaNExpMask) == NaNExpMask) {
            *bitpattern &= NaNlowbitclear;
        }
    }

    return A;
}

int32_t *
generate_rand_int32(int num) {
    int j;

    int32_t *A = (int32_t *) malloc(sizeof(int32_t) * num);

    /* generate random increasing order data */
    for (j = 0; j < num; j++) {
        A[j] = rand() % INT_MAX;
    }

    return A;
}

tuple_t *
generate_rand_tuples(int num) {

    relation_t rel;
    rel.tuples = (tuple_t *) malloc(sizeof(tuple_t) * num);
    rel.num_tuples=num;
    uint64_t i;

    for (i = 0; i < num; i++) {
        rel.tuples[i].key = (i + 1);
        rel.tuples[i].payload = i;
    }

    /* randomly shuffle elements */
    knuth_shuffle(&rel);

    return rel.tuples;
}

int64_t *
generate_rand_ordered_int64(int num) {
    int64_t startA;
    int j;
    uint64_t INCRMOD = 100;

    int64_t maxint = ~(1 << 31) - INCRMOD;

    int64_t *A = (int64_t *) malloc(sizeof(int64_t) * num);
    startA = 1 + rand() % INCRMOD;

    /* generate random increasing order data */
    for (j = 0; j < num; j++) {
        A[j] = startA;

        if (startA < maxint)
            startA += rand() % INCRMOD;
    }

    return A;
}

int32_t *
generate_rand_ordered_int32(int num) {
    int32_t startA;
    int j;
    uint32_t INCRMOD = 100;

    intkey_t maxint = ~(1 << 31) - INCRMOD;

    int32_t *A = (int32_t *) malloc(sizeof(int32_t) * num);
    startA = 1 + rand() % INCRMOD;

    /* generate random increasing order data */
    for (j = 0; j < num; j++) {
        A[j] = startA;

        if (startA < maxint)
            startA += rand() % INCRMOD;
    }

    return A;
}

tuple_t *
generate_rand_ordered_tuples(int num) {
    intkey_t startA;
    int j;
    uint32_t INCRMOD = 100;

    intkey_t maxint = ~(1 << 31) - INCRMOD;

    tuple_t *A = (tuple_t *) malloc(sizeof(tuple_t) * num);
    startA = 1 + rand() % INCRMOD;

    /* generate random increasing order data */
    for (j = 0; j < num; j++) {
        A[j].key = startA;

        if (startA < maxint)
            startA += rand() % INCRMOD;
    }

    return A;
}
