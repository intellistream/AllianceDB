/**
 * @file    no_partitioning_join.h
 * @author  Cagri Balkesen <cagri.balkesen@inf.ethz.ch>
 * @date    Sun Feb  5 20:12:56 2012
 * @version $Id: no_partitioning_join.h 4419 2013-10-21 16:24:35Z bcagri $
 *
 * @brief  The interface of No partitioning optimized (NPO) join algorithm.
 *
 * (c) 2012, ETH Zurich, Systems Group
 *
 */

#ifndef NO_PARTITIONING_JOIN_H
#define NO_PARTITIONING_JOIN_H

typedef struct param_t param_t;

#include "../utils/types.h" /* relation_t */
#include "../benchmark.h"

/**
 * NPO: No Partitioning Join Optimized.
 *
 * The "No Partitioning Join Optimized" implementation denoted as NPO
 * which was originally proposed by Blanas et al. in SIGMOD 2011.
 *
 * The following is a multi-threaded implementation. Just returns the
 * number of result tuples.
 *
 * @param relR input relation R - inner relation
 * @param relS input relation S - outer relation
 *
 * @return number of result tuples
 */
result_t *
NPO(relation_t *relR, relation_t *relS, param_t cmd_params);

/**
 * The No Partitioning Join Optimized (NPO) as a single-threaded
 * implementation. Just returns the number of result tuples.
 *
 * @param relR input relation R - inner relation
 * @param relS input relation S - outer relation
 *
 * @return number of result tuples
 */
result_t *
NPO_st(relation_t *relR, relation_t *relS, param_t cmd_params);


#endif /* NO_PARTITIONING_JOIN_H */
