//
// Created by Shuhao Zhang on 26/10/19.
//

#ifndef ALLIANCEDB_COMMON_FUNCTIONS_H
#define ALLIANCEDB_COMMON_FUNCTIONS_H


#ifndef PTHREAD_BARRIER_SERIAL_THREAD
#define PTHREAD_BARRIER_SERIAL_THREAD 1
#endif

#ifndef BARRIER_ARRIVE
/** barrier wait macro */
#define BARRIER_ARRIVE(B, RV)                            \
    RV = pthread_barrier_wait(B);                       \
    if(RV !=0 && RV != PTHREAD_BARRIER_SERIAL_THREAD){  \
        printf("Couldn't wait on barrier\n");           \
        exit(EXIT_FAILURE);                             \
    }
#endif

#ifdef JOIN_RESULT_MATERIALIZE
#include "tuple_buffer.h"       /* for materialization */
#endif

#ifndef HASH
#define HASH(X, MASK, SKIP) (((X) & MASK) >> SKIP)
#endif

/** Debug msg logging method */
#ifdef DEBUG
#define DEBUGMSG(COND, MSG, ...)                                    \
    if(COND) { fprintf(stdout, "[DEBUG] "MSG, ## __VA_ARGS__); }
#else
#define DEBUGMSG(COND, MSG, ...)
#endif


#define  MEASURE
#define expected_results 12800000.0

/**
 * Allocates a hashtable of NUM_BUCKETS and inits everything to 0.
 *
 * @param ht pointer to a hashtable_t pointer
 */
void allocate_hashtable(hashtable_t **ppht, uint32_t nbuckets);

/**
 * Single-thread hashtable build method, ht is pre-allocated.
 *
 * @param ht hastable to be built
 * @param rel the build relation
 */
void
build_hashtable_st(hashtable_t *ht, relation_t *rel);

/**
 * Probes the hashtable for the given outer relation, returns num results.
 * This probing method is used for both single and multi-threaded version.
 *
 * @param ht hashtable to be probed
 * @param rel the probing outer relation
 * @param output chained tuple buffer to write join results, i.e. rid pairs.
 *
 * @return number of matching tuples
 */
int64_t
probe_hashtable(hashtable_t *ht, relation_t *rel, void *output, uint64_t progressivetimer[]);


void
build_hashtable_single(const hashtable_t *ht, const relation_t *rel, uint32_t i, const uint32_t hashmask,
                       const uint32_t skipbits);

int64_t proble_hashtable_single_measure(const hashtable_t *ht, const relation_t *rel, uint32_t index_rel,
                                        const uint32_t hashmask, const uint32_t skipbits, int64_t *matches,
                                        uint64_t progressivetimer[]);

int64_t proble_hashtable_single(const hashtable_t *ht, const relation_t *rel, uint32_t index_rel,
                                const uint32_t hashmask, const uint32_t skipbits);


/**
 * Multi-thread hashtable build method, ht is pre-allocated.
 * Writes to buckets are synchronized via latches.
 *
 * @param ht hastable to be built
 * @param rel the build relationO
 * @param overflowbuf pre-allocated chunk of buckets for overflow use.
 */
void
build_hashtable_mt(hashtable_t *ht, relation_t *rel,
                   bucket_buffer_t **overflowbuf);

/**
 * Releases memory allocated for the hashtable.
 *
 * @param ht pointer to hashtable
 */
void destroy_hashtable(hashtable_t *ht);

/** De-allocates all the bucket_buffer_t */
void
free_bucket_buffer(bucket_buffer_t *buf);

#endif //ALLIANCEDB_COMMON_FUNCTIONS_H
