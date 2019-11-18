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

/**
 * @defgroup Types Common Types
 * Common type definitions used by all join implementations.
 * @{
 */

#ifdef KEY_8B /* 64-bit key/value, 16B tuples */
typedef int64_t intkey_t;
typedef int64_t value_t;
#else /* 32-bit key/value, 8B tuples */
typedef int32_t intkey_t;
typedef int32_t value_t;
#endif

typedef struct tuple_t tuple_t;
typedef struct relation_t relation_t;

typedef struct result_t result_t;
typedef struct threadresult_t threadresult_t;

typedef struct joinconfig_t joinconfig_t;

/**
 * Type definition for a tuple, depending on KEY_8B a tuple can be 16B or 8B
 * @note this layout is chosen as a work-around for AVX double operations.
 */
struct tuple_t {
    tuple_t(int key) {
        this->key = key;
    }

    tuple_t() {

    }

    value_t payload;
    intkey_t key;
};

/**
 * Type definition for a relation.
 * It consists of an array of tuples and a size of the relation.
 */
struct relation_t {
    tuple_t *tuples;
    uint64_t num_tuples;
};

/** Holds the join results of a thread */
struct threadresult_t {
    int64_t nresults;
    void *results;
    uint32_t threadid;
};

/** Type definition for join results. */
struct result_t {
    int64_t totalresults;
    threadresult_t *resultlist;
    int nthreads;
};


/**
 * Various NUMA shuffling strategies for data shuffling phase of join
 * algorithms as also described by NUMA-aware data shuffling paper [CIDR'13].
 *
 * NUMA_SHUFFLE_RANDOM, NUMA_SHUFFLE_RING, NUMA_SHUFFLE_NEXT
 */
enum numa_strategy_t {
    RANDOM, RING, NEXT
};

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
