//
// Created by Shuhao Zhang on 15/11/19.
//


#include "sort_common.h"

int
is_sorted_helper(int64_t *items, uint64_t nitems) {
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
    tuple_t *tuples = (tuple_t *) items;
    for (i = 0; i < nitems; i++) {
        /*
        if (tuples[i].key == curr) {
            if (!warned) {
                printf("[WARN ] Equal items, still ok... "\
                       "item[%d].key=%d is equal to item[%d].key=%d\n",
                       i, tuples[i].key, i - 1, curr);
                warned = 1;
            }
        } else */
        if (tuples[i].key <= curr) {
            printf("[ERROR] item[%d].key=%d is less or equal than item[%d].key=%d\n",
                   i, tuples[i].key, i - 1, curr);
            return 0;
        }

        curr = tuples[i].key;
    }

    return 1;

#endif
}