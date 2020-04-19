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

#ifndef ONLINE_JOIN_H
#define ONLINE_JOIN_H

#include "../utils/types.h" /* relation_t */
#include "../benchmark.h"
#include "common_functions.h"
/**
 * Join-Matrix Model SHJ.
 *
 * The "Join-Matrix Model" was originally proposed by Mohammed Elseidy et al. in VLDB 2014.
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
SHJ_JM_NP(relation_t *relR, relation_t *relS,  param_t cmd_params);      

result_t *
SHJ_JM_P(relation_t *relR, relation_t *relS,  param_t cmd_params);      

/**
 * Join-Biclique Model SHJ.
 *
 * The "Join-Biclique Model" was originally proposed by Qian Lin et al. in SIGMOD 2015.
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
SHJ_JB_NP(relation_t *relR, relation_t *relS,  param_t cmd_params);      

/**
 * Join-Biclique Model SHJ with CountRand Algorithm
 * @param relR
 * @param relS
 * @param nthreads
 * @return
 */
result_t *
SHJ_JBCR_NP(relation_t *relR, relation_t *relS,  param_t cmd_params);      


/**
 * HandShake Model
 *
 * The "HandShake Model" was originally proposed by Teubner et al. in SIGMOD 2011.
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
SHJ_HS_NP(relation_t *relR, relation_t *relS,  param_t cmd_params);      


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
SHJ_st(relation_t *relR, relation_t *relS,  param_t cmd_params);      


/**
 * Single thread PMJ
 * @param relR
 * @param relS
 * @param nthreads
 * @return
 */
result_t *
PMJ_st(relation_t *relR, relation_t *relS,  param_t cmd_params);      

/**
 * PMJ JM NP
 * @param relR
 * @param relS
 * @param nthreads
 * @return
 */
result_t *
PMJ_JM_NP(relation_t *relR, relation_t *relS,  param_t cmd_params);      

/**
 * PMJ JB NP
 * @param relR
 * @param relS
 * @param nthreads
 * @return
 */
result_t *
PMJ_JB_NP(relation_t *relR, relation_t *relS,  param_t cmd_params);      


/**
 * PMJ JB NP
 * @param relR
 * @param relS
 * @param nthreads
 * @return
 */
result_t *
PMJ_JBCR_NP(relation_t *relR, relation_t *relS,  param_t cmd_params);      

/**
 * PMJ HS NP
 * @param relR
 * @param relS
 * @param nthreads
 * @return
 */
result_t *
PMJ_HS_NP(relation_t *relR, relation_t *relS,  param_t cmd_params);      

/**
 * Single thread Ripple Join
 * @param relR
 * @param relS
 * @param nthreads
 * @return
 */
result_t *
RPJ_st(relation_t *relR, relation_t *relS,  param_t cmd_params);      

/**
 * Join-Matrix Model RPJ.
 *
 * The "Join-Matrix Model" was originally proposed by Mohammed Elseidy et al. in VLDB 2014.
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
RPJ_JM_NP(relation_t *relR, relation_t *relS,  param_t cmd_params);      

/**
 * Join-Matrix Model RPJ.
 *
 * The "Join-Matrix Model" was originally proposed by Mohammed Elseidy et al. in VLDB 2014.
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
RPJ_JB_NP(relation_t *relR, relation_t *relS,  param_t cmd_params);      

/**
 * Join-Matrix Model RPJ.
 *
 * The "Join-Matrix Model" was originally proposed by Mohammed Elseidy et al. in VLDB 2014.
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
RPJ_JBCR_NP(relation_t *relR, relation_t *relS,  param_t cmd_params);      

result_t *
RPJ_HS_NP(relation_t *relR, relation_t *relS,  param_t cmd_params);      

#endif /* NO_PARTITIONING_JOIN_H */
