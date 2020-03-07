/**
 * @file    types.h
 * @author  Cagri Balkesen <cagri.balkesen@inf.ethz.ch>
 * @date    Tue May 22 16:43:30 2012
 * @version $Id: types.h 4419 2013-10-21 16:24:35Z bcagri $
 *
 * @brief  Provides general type definitions used by all join algorithms.
 *
 *
 */
#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <vector>
#include <iostream>
#include <sstream>
#include <chrono>

using namespace std;

/**
 * @defgroup Types Common Types
 * Common type definitions used by all join implementations.
 * @{
 */

struct table_t {
//    int32_t value;
//    char value[256];
    char value[10240];
};

#ifdef KEY_8B /* 64-bit key/payload, 16B tuples */
typedef int64_t intkey_t;
typedef int64_t value_t;
#else /* 32-bit key/payload, 8B tuples */
typedef int32_t intkey_t;
typedef int32_t value_t;
#endif

#if !defined PRId64
# undef PRId64
# define PRId64 "lld"
#endif

#if !defined PRIu64
# undef PRIu64
# define PRIu64 "llu"
#endif

typedef struct tuple_t    tuple_t;
typedef struct relation_t relation_t;
typedef struct relation_payload_t relation_payload_t;
typedef struct result_t result_t;
typedef struct threadresult_t threadresult_t;
typedef struct joinconfig_t joinconfig_t;

/**
 * Type definition for a tuple, depending on KEY_8B a tuple can be 16B or 8B
 * @note this layout is chosen as a work-around for AVX double operations.
 */
struct tuple_t {
//    value_t  payload;
//    intkey_t key;
    value_t payloadID;//TODO: make sure payload is simply the id of the tuple.
    intkey_t key;//little end, lowest is the most significant bit.
    bool operator < (const tuple_t& str) const
    {
        return (key < str.key);
    }
};


/**
 * Type definition for a relation.
 * It consists of an array of tuples and a size of the relation.
 */
struct relation_t {
    tuple_t * tuples;
    uint64_t  num_tuples;
    relation_payload_t *payload;
};

// add a new structure to save real payload, let original payload be index of this struct
struct relation_payload_t {
//    time_t *ts;//add timestamp for each tuple in the relation.
    std::chrono::milliseconds *ts;//add timestamp for each tuple in the relation.
//    table_t *rows;
    uint64_t num_tuples;
};

/** Holds the join results of a thread */
struct threadresult_t {
    int64_t  nresults;
    void *   results;
    uint32_t threadid;
};

/** Type definition for join results. */
struct result_t {
    int64_t          totalresults;
    threadresult_t * resultlist;
    int              nthreads;
};


/**
 * Various NUMA shuffling strategies for data shuffling phase of join
 * algorithms as also described by NUMA-aware data shuffling paper [CIDR'13].
 *
 * NUMA_SHUFFLE_RANDOM, NUMA_SHUFFLE_RING, NUMA_SHUFFLE_NEXT
 */
enum numa_strategy_t {RANDOM, RING, NEXT};

/** Join configuration parameters. */
struct joinconfig_t {
    int NTHREADS;
    int PARTFANOUT;
    int SCALARSORT;
    int SCALARMERGE;
    int MWAYMERGEBUFFERSIZE;
    enum numa_strategy_t NUMASTRATEGY;
};

/** @} */

#endif /* TYPES_H */
