/**
 * @file    npj_types.h
 * @author  Cagri Balkesen <cagri.balkesen@inf.ethz.ch>
 * @date    Tue May 22 16:37:59 2012
 * @version $Id: npj_types.h 3017 2012-12-07 10:56:20Z bcagri $
 *
 * @brief  Provides type definitions used by No Partitioning Join implementations.
 *
 */
#ifndef NPO_TYPES_H
#define NPO_TYPES_H

#include <set>
#include <map>
#include <unordered_map>
#include "../utils/types.h" /* tuple_t */
#include "npj_params.h"

#define PRESAMPLING_SIZE 1000
#define RANDOM_BUFFER_SIZE 1000
#define RESERVOIR_SIZE 6250
#define RESERVOIR_STRATA_NUM 2

#define PROB_BUFF 2

/**
 * @defgroup NPOTypes Type definitions used by NPO.
 * @{
 */

typedef struct bucket_t bucket_t;
typedef struct hashtable_t hashtable_t;
typedef struct bucket_buffer_t bucket_buffer_t;

#if PADDED_BUCKET == 0
/**
 * Normal hashtable buckets.
 *
 * if KEY_8B then key is 8B and sizeof(bucket_t) = 48B
 * else key is 16B and sizeof(bucket_t) = 32B
 */
struct bucket_t {
    volatile char latch;
    /* 3B hole */
    uint32_t count=0;
    tuple_t tuples[BUCKET_SIZE];
    struct bucket_t *next;
};
#else /* PADDED_BUCKET: bucket is padded to cache line size */
/**
 * Cache-sized bucket where size of the bucket is padded
 * to cache line size (64B).
 */
struct bucket_t {
    volatile char     latch;
    /* 3B hole */
    uint32_t          count;
    tuple_t           tuples[BUCKET_SIZE];
    struct bucket_t * next;
} __attribute__ ((aligned(CACHE_LINE_SIZE)));
#endif /* PADDED_BUCKET */

/** Hashtable structure for NPO. */

struct hashtable_t {
    bucket_t *buckets;
//    std::priority_queue<std::pair<intkey_t, intkey_t>, std::vector<std::pair<intkey_t, intkey_t> >, greater<std::pair<intkey_t, intkey_t> > > q;
//    CountMinSketch *c;
//    std::vector<tuple_t> q;
    // tuple_t q[RANDOM_BUFFER_SIZE];
    tuple_t *rsv;
    uint32_t *rsv_rand_que = NULL;
    int rsv_que_head = 0;
    int pre_smp_thrshld;
    int div_prmtr;
    uint32_t hash_a;
    uint32_t hash_b;
    // std::set<intkey_t> *pre_set;
    // std::map<intkey_t, int> *pre_smp;
    int count_pre;
    std::unordered_map<intkey_t, int> *pre_smp;
    uint32_t Bernoulli_q;
    uint32_t Universal_p;
    uint32_t Epsilon;
    double Epsilon_d;
    int32_t cnt = 0;
    int64_t num_buckets;
    uint32_t hash_mask;
    uint32_t skip_bits;
};
/** Pre-allocated bucket buffers are used for overflow-buckets. */
struct bucket_buffer_t {
    struct bucket_buffer_t *next;
    uint32_t count;
    bucket_t buf[OVERFLOW_BUF_SIZE];
};

/** @} */

#endif /* NPO_TYPES_H */
