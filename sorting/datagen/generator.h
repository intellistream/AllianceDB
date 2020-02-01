/**
 * @file    generator.h
 * @author  Cagri Balkesen <cagri.balkesen@inf.ethz.ch>
 * @date    Fri May 18 14:05:07 2012
 * @version $Id$ 
 * 
 * @brief  Provides methods to generate data sets of various types
 * 
 * (c) 2014, ETH Zurich, Systems Group
 *
 */

#ifndef GENERATOR_H
#define GENERATOR_H

#include "../util/types.h" /* relation_t, tuple_t */

/**
 * @defgroup DataGeneration Data Set Generation
 *
 * @warning all relevant memory must be allocated before calling data generation.
 *
 * @{
 */

/**
 * Seed the random number generator before calling create_relation_xx. If not
 * called, then generator will be initialized with the time of the call which
 * produces different random numbers from run to run.
 */
void
seed_generator(unsigned int seed);

/**
 * Create relation with non-unique keys uniformly distributed between [0, maxid]
 */
int
create_relation_nonunique(relation_t *reln, int64_t ntuples, const int64_t maxid);

/**
 * Create relation with only primary keys (i.e. keys are unique from 1 to
 * num_tuples)
 */
int
create_relation_pk(relation_t *reln, int64_t ntuples);

/**
 * Create relation with foreign keys (i.e. duplicated keys exist). If ntuples is
 * an exact multiple of maxid, (ntuples/maxid) sub-relations with shuffled keys
 * following each other are generated.
 */
int
create_relation_fk(relation_t *reln, int64_t ntuples, const int64_t maxid);

/**
 * Create a foreign-key relation using the given primary-key relation and
 * foreign-key relation size. If the keys in pkrel is randomly distributed in
 * the full integer range, then
 */
int
create_relation_fk_from_pk(relation_t *fkrel, relation_t *pkrel, int64_t ntuples);

/**
 * Create relation with keys distributed with zipf between [0, maxid]
 * - zipf_param is the parameter of zipf distr (aka s)
 * - maxid is equivalent to the alphabet size
 */
int
create_relation_zipf(relation_t * reln, int64_t ntuples,
                     const int64_t maxid, const double zipfparam);


/**
 * Create relation with only primary keys (i.e. keys are unique from 1 to
 * maxid). Creation procedure is executed by
 * nthreads in parallel, where each memory is initialized thread local.
 */
int
parallel_create_relation(relation_t *reln, uint64_t ntuples,
                         uint32_t nthreads, uint64_t maxid);

/**
 * Create relation with foreign keys (i.e. duplicated keys exist). If ntuples is
 * an exact multiple of maxid, (ntuples/maxid) sub-relations with shuffled keys
 * following each other are generated. Creation procedure is executed by
 * nthreads in parallel, where each memory is initialized thread local.
 */
int
parallel_create_relation_fk(relation_t *reln, int64_t ntuples,
                            const int64_t maxid, uint32_t nthreads);

/**
 * This is just to make sure that chunks of the temporary memory
 * will be numa local to threads. Just initialize memory to 0 for
 * making sure it will be allocated numa-local.
 */
int
numa_localize(tuple_t * relation, int64_t num_tuples, uint32_t nthreads);

/**
 * Write relation to a file.
 */
void
write_relation(relation_t * rel, char * filename);

/**
 * Shuffle tuples of the relation using Knuth shuffle.
 *
 * @param relation
 */
void
knuth_shuffle(relation_t * relation);

/** @} */

#endif /* GENERATOR_H */
