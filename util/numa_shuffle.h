//
// Created by Shuhao Zhang on 8/9/19.
//

#ifndef ALLIANCEDB_NUMA_SHUFFLE_H
#define ALLIANCEDB_NUMA_SHUFFLE_H

#include "types.h" /* enum numa_strategy_t */
#include "../affinity/cpu_mapping.h" /* get_cpu_id() */
#include "../datagen/generator.h" /* knuth_shuffle() */

/**
 * \ingroup numa
 *
 * Initialize the NUMA shuffling strategy with one of the following:
 *
 * RANDOM, RING, NEXT
 */
void
numa_shuffle_init(enum numa_strategy_t numastrategy, int nthreads);

/**
 * \ingroup numa
 *
 * Machine specific implementation of NUMA-shuffling strategies.
 *
 * @note The implementation needs to be customized for different hardware
 *       based on the NUMA topology.
 *
 * @param my_tid logical thread id of the calling thread.
 * @param i next thread index for shuffling (between 0 and nthreads)
 * @param nthreads number of total threads
 * @return the logical thread id of the destination thread for data shuffling
 */
int
get_numa_shuffle_strategy(int my_tid, int i, int nthreads);



#endif //ALLIANCEDB_NUMA_SHUFFLE_H
