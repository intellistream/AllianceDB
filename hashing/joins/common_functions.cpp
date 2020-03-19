//
// Created by Shuhao Zhang on 26/10/19.
//

#include "../timer/t_timer.h"
#include "../utils/generator.h"          /* numa_localize() */
#include "../utils/lock.h"               /* lock, unlock */
#include "npj_types.h"          /* bucket_t, hashtable_t, bucket_buffer_t */
#include "npj_params.h"         /* constant parameters */
#include <stdlib.h>             /* memalign */
#include <stdio.h>              /* printf */
#include <string.h>             /* memset */
#include <list>
#include <thread>
#include "common_functions.h"
#include "../utils/params.h"


/** An experimental feature to allocate input relations numa-local */
extern int nthreads;      /* defined in generator.c */
extern int numalocalize;  /* defined in generator.c */
//struct t_window window0;
//struct t_window window1;


#ifndef NEXT_POW_2
/**
 *  compute the next number, greater than or equal to 32-bit unsigned v.
 *  taken from "bit twiddling hacks":
 *  http://graphics.stanford.edu/~seander/bithacks.html
 */
#define NEXT_POW_2(V)                           \
    do {                                        \
        V--;                                    \
        V |= V >> 1;                            \
        V |= V >> 2;                            \
        V |= V >> 4;                            \
        V |= V >> 8;                            \
        V |= V >> 16;                           \
        V++;                                    \
    } while(0)
#endif

std::string print_relation(tuple_t *tuple, int length) {
    std::string tmp = "";
    tmp.append("[");

    for (int i = 0; i < length; i++)
        tmp.append(std::to_string(tuple[i].key)).append(",");
    tmp.append("]\n");
    return tmp;
}


void *
malloc_aligned(size_t size) {
    void *ret;
    int rv;
    rv = posix_memalign((void **) &ret, CACHE_LINE_SIZE, size);

    if (rv) {
        DEBUGMSG("[ERROR] malloc_aligned() failed: out of memory")
        return 0;
    }

    return ret;
}

/**
 * Allocates a hashtable of NUM_BUCKETS and inits everything to 0.
 *
 * @param ht pointer to a hashtable_t pointer
 */
void
allocate_hashtable(hashtable_t **ppht, uint32_t nbuckets) {
    hashtable_t *ht;

    ht = (hashtable_t *) malloc(sizeof(hashtable_t));
    ht->num_buckets = nbuckets;
    NEXT_POW_2((ht->num_buckets));

    /* allocate hashtable buckets cache line aligned */
    if (posix_memalign((void **) &ht->buckets, CACHE_LINE_SIZE,
                       ht->num_buckets * sizeof(bucket_t))) {
        perror("Aligned allocation failed!\n");
        exit(EXIT_FAILURE);
    }

    /** Not an elegant way of passing whether we will numa-localize, but this
        feature is experimental anyway. */
    if (numalocalize) {
        tuple_t *mem = (tuple_t *) ht->buckets;
        uint32_t ntuples = (ht->num_buckets * sizeof(bucket_t)) / sizeof(tuple_t);
        numa_localize(mem, ntuples, nthreads);
    }

    memset(ht->buckets, 0, ht->num_buckets * sizeof(bucket_t));
    ht->skip_bits = 0; /* the default for modulo hash */
    ht->hash_mask = (ht->num_buckets - 1) << ht->skip_bits;
    *ppht = ht;
}

void destroy_hashtable(hashtable_t *ht) {
    free(ht->buckets);
    free(ht);
}


void free_bucket_buffer(bucket_buffer_t *buf) {
    do {
        bucket_buffer_t *tmp = buf->next;
        free(buf);
        buf = tmp;
    } while (buf);
}


void build_hashtable_st(hashtable_t *ht, relation_t *rel) {
    uint32_t i;
    const uint32_t hashmask = ht->hash_mask;
    const uint32_t skipbits = ht->skip_bits;

    for (i = 0; i < rel->num_tuples; i++) {
        build_hashtable_single(ht, rel, i, hashmask, skipbits);
    }
}


void
debuild_hashtable_single(const hashtable_t *ht, const tuple_t *tuple, const uint32_t hashmask,
                         const uint32_t skipbits) {

    uint32_t index_ht;
    intkey_t idx = HASH(tuple->key, hashmask, skipbits);
    bucket_t *b = ht->buckets + idx;
    for (index_ht = 0; index_ht < b->count; index_ht++) {
        if (tuple->key == b->tuples[index_ht].key) {
            b->tuples[index_ht].key = -1;//set it to never match.
        }
    }
}

void build_hashtable_single(const hashtable_t *ht, const tuple_t *tuple,
                            const uint32_t hashmask,
                            const uint32_t skipbits) {
    tuple_t *dest;
    bucket_t *curr, *nxt;
    int32_t idx = HASH(tuple->key, hashmask, skipbits);

    /* copy the tuple to appropriate hash bucket */
    /* if full, follow nxt pointer to find correct place */
    curr = ht->buckets + idx;
    nxt = curr->next;

    if (curr->count == BUCKET_SIZE) {
        if (!nxt || nxt->count == BUCKET_SIZE) {
            bucket_t *b;
            b = (bucket_t *) calloc(1, sizeof(bucket_t));
            curr->next = b;
            b->next = nxt;
            b->count = 1;
            dest = b->tuples;
        } else {
            dest = nxt->tuples + nxt->count;
            nxt->count++;
        }
    } else {
        dest = curr->tuples + curr->count;//let dest point to correct place of bucket.
        curr->count++;

        if (curr->count > BUCKET_SIZE) {
            printf("this is wrong 2..\n");
        }
    }
    *dest = *tuple;//copy the content of rel-tuples[i] to bucket.
}

void build_hashtable_single(const hashtable_t *ht, const relation_t *rel, uint32_t i, const uint32_t hashmask,
                            const uint32_t skipbits) {
    build_hashtable_single(ht, &rel->tuples[i], hashmask, skipbits);
}

/**
 * Used by NPO
 * @param ht
 * @param rel
 * @param output
 * @param timer
 * @return
 */
int64_t probe_hashtable(hashtable_t *ht, relation_t *rel, void *output, T_TIMER *timer) {
    uint32_t i;
    int64_t matches;

    const uint32_t hashmask = ht->hash_mask;
    const uint32_t skipbits = ht->skip_bits;
#ifdef PREFETCH_NPJ
    size_t prefetch_index = PREFETCH_DISTANCE;
#endif
    matches = 0;
    for (i = 0; i < rel->num_tuples; i++) {
        probe_hashtable_single_measure(ht, rel, i, hashmask, skipbits, &matches, timer, output);
    }
    return matches;
}

int64_t probe_hashtable_single_measure(const hashtable_t *ht, const tuple_t *tuple, const uint32_t hashmask,
                                       const uint32_t skipbits, int64_t *matches,
        /*void *(*thread_fun)(const tuple_t *, const tuple_t *, int64_t *),*/
                                       T_TIMER *timer, bool ISTupleR, void *output) {
    uint32_t index_ht;

#ifdef JOIN_RESULT_MATERIALIZE
    chainedtuplebuffer_t *chainedbuf = (chainedtuplebuffer_t *) output;
#endif

    intkey_t idx = HASH(tuple->key, hashmask, skipbits);
    bucket_t *b = ht->buckets + idx;
    do {
        for (index_ht = 0; index_ht < b->count; index_ht++) {
            if (tuple->key == b->tuples[index_ht].key) {
//                (*matches)++;
//                DUMMY()
//                this_thread::sleep_for(chrono::microseconds(timer->simulate_compute_time));
#ifdef JOIN_RESULT_MATERIALIZE
                /* copy to the result buffer */
                /** We materialize only <S-key, S-RID> */
                tuple_t *joinres = cb_next_writepos(chainedbuf);
                joinres->key = tuple->key;
                joinres->payloadID = tuple->payloadID;
#endif
                (*matches)++;
#ifndef NO_TIMING
                END_PROGRESSIVE_MEASURE(tuple->payloadID, timer, ISTupleR)
#endif
                /*if (thread_fun) {
                    thread_fun(tuple, &b->tuples[index_ht], matches);
                }*/
            }
        }
        b = b->next;/* follow overflow pointer */
    } while (b);
    return *matches;
}

int64_t probe_hashtable_single_measure(const hashtable_t *ht, const relation_t *rel, uint32_t index_rel,
                                       const uint32_t hashmask, const uint32_t skipbits, int64_t *matches,
        /*void *(*thread_fun)(const tuple_t *, const tuple_t *, int64_t *),*/
                                       T_TIMER *timer, void *output) {
    return probe_hashtable_single_measure(ht, &rel->tuples[index_rel], hashmask, skipbits, matches,
            /*thread_fun,*/ timer, false, output);
}

/**
 * match function typically for RPJ
 * @param list
 * @param tuple
 * @param matches
 */
void match_single_tuple(const list<tuple_t *> list, const tuple_t *tuple, int64_t *matches,
        /*void *(*thread_fun)(const tuple_t *, const tuple_t *, int64_t *),*/ T_TIMER *timer,
                        bool ISTupleR, void *output) {
    // TODO: refactor RPJ related methods to RPJ helper
    for (auto it = list.begin(); it != list.end(); it++) {
        /*if (thread_fun) {
            thread_fun(tuple, it.operator*(), matches);
        }*/
        if (tuple->key == it.operator*()->key) {
            (*matches)++;
//            DUMMY()
//            this_thread::sleep_for(chrono::microseconds(timer->simulate_compute_time));
#ifndef NO_TIMING
            END_PROGRESSIVE_MEASURE(tuple->payloadID, (timer), ISTupleR)
#endif
        }
    }
    DEBUGMSG("JOINING: matches: %d, tuple: %d\n", *matches, tuple->key);
}

int find_index(const tuple_t *rel, const int length, const tuple_t *tuple) {
    // TODO: refactor RPJ related methods to RPJ helper
    for (int i = 0; i < length; i++) {
        if (rel[i].key == tuple->key) {
            return i;
        }
    }
    return -1;
}

/**
 * Returns a new bucket_t from the given bucket_buffer_t.
 * If the bucket_buffer_t does not have enough space, then allocates
 * a new bucket_buffer_t and adds to the list.
 *
 * @param result [out] the new bucket
 * @param buf [in,out] the pointer to the bucket_buffer_t pointer
 */
static inline void
get_new_bucket(bucket_t **result, bucket_buffer_t **buf) {
    if ((*buf)->count < OVERFLOW_BUF_SIZE) {
        *result = (*buf)->buf + (*buf)->count;
        (*buf)->count++;
    } else {
        /* need to allocate new buffer */
        bucket_buffer_t *new_buf = (bucket_buffer_t *)
                malloc(sizeof(bucket_buffer_t));
        new_buf->count = 1;
        new_buf->next = *buf;
        *buf = new_buf;
        *result = new_buf->buf;
    }
}

void build_hashtable_mt(hashtable_t *ht, relation_t *rel, bucket_buffer_t **overflowbuf) {
    uint32_t i;
    const uint32_t hashmask = ht->hash_mask;
    const uint32_t skipbits = ht->skip_bits;

#ifdef PREFETCH_NPJ
    size_t prefetch_index = PREFETCH_DISTANCE;
#endif

    for (i = 0; i < rel->num_tuples; i++) {
        tuple_t *dest;
        bucket_t *curr, *nxt;

#ifdef PREFETCH_NPJ
        if (prefetch_index < rel->num_tuples) {
            intkey_t idx_prefetch = HASH(rel->tuples[prefetch_index++].key,
                                         hashmask, skipbits);
            __builtin_prefetch(ht->buckets + idx_prefetch, 1, 1);
        }
#endif

        int32_t idx = HASH(rel->tuples[i].key, hashmask, skipbits);
        /* copy the tuple to appropriate hash bucket */
        /* if full, follow nxt pointer to find correct place */
        curr = ht->buckets + idx;
        lock(&curr->latch);
        nxt = curr->next;

        if (curr->count == BUCKET_SIZE) {
            if (!nxt || nxt->count == BUCKET_SIZE) {
                bucket_t *b;
                /* b = (bucket_t*) calloc(1, sizeof(bucket_t)); */
                /* instead of calloc() everytime, we pre-allocate */
                get_new_bucket(&b, overflowbuf);
                curr->next = b;
                b->next = nxt;
                b->count = 1;
                dest = b->tuples;
            } else {
                dest = nxt->tuples + nxt->count;
                nxt->count++;
            }
        } else {
            dest = curr->tuples + curr->count;
            curr->count++;
        }

        *dest = rel->tuples[i];
        unlock(&curr->latch);
    }
}

const std::string red("\033[0;31m");
const std::string reset("\033[0m");


tuple_t *copy_tuples(const tuple_t *tuples, int size) {
    auto relRsz = size * sizeof(tuple_t)
                  + RELATION_PADDING(1, CACHELINEPADDING(1));//TODO: think why we need to patch this.
    tuple_t *copy = (tuple_t *) malloc_aligned(relRsz);

    for (auto i = 0; i < size; i++)
        copy[i] = tuples[i];
    return copy;
}

std::string print_tuples(const tuple_t *tuples, int size) {

    std::string tmp = "";
    tmp.append("[");

    for (auto i = 0; i < size; i++)
        tmp.append(std::to_string(tuples[i].key)).append(",");
    tmp.append("]");
    return tmp;
}

std::string print_window(const std::list<intkey_t> &list) {

    std::string tmp = "";
    tmp.append("[");

    for (auto const &v : list)
        tmp.append(std::to_string(v)).append(",");
    tmp.append("]");
    return tmp;
}



