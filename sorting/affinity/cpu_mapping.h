/**
 * @file   cpu_mapping.h
 * @author Cagri Balkesen <cagri.balkesen@inf.ethz.ch>
 * @date   Tue May 22 16:35:12 2012
 *
 * @brief  Provides NUMA-aware CPU mapping utility functions.
 *
 * (c) 2014, ETH Zurich, Systems Group
 *
 */
#include "../joins/common_functions.h"
#ifndef CPU_MAPPING_H
#define CPU_MAPPING_H

/**
 * @defgroup cpumapping CPU mapping tool
 * @{
 */

/**
 * if the custom cpu mapping file exists, logical to physical mappings are
 * initialized from that file, next it will first try libNUMA if available,
 * and finally round-robin as last option.
 */
#ifndef CUSTOM_CPU_MAPPING
#define CUSTOM_CPU_MAPPING EXP_DIR"/cpu-mapping.txt"
#endif

/**
 * Initialize cpu mappings and NUMA topology (if libNUMA available).
 * Try custom cpu mapping file first, if does not exist then round-robin
 * initialization among available CPUs reported by the system.
 */
void
cpu_mapping_init();

/**
 * De-initialize/free cpu mapping data structures.
 */
void
cpu_mapping_cleanup();


/**
 * Returns SMT aware logical to physical CPU mapping for a given logical thread id.
 */
int get_cpu_id(int thread_id);


/** @} */

/**
 * @defgroup numa NUMA related utility methods.
 * @{
 */

/**
 * Returns whether given logical thread id is the first thread in its NUMA region.
 *
 * @param logicaltid logical thread id
 *
 * @return true or false
 */
int
is_first_thread_in_numa_region(int logicaltid);

/**
 * Returns the index of the given logical thread within its NUMA-region.
 *
 * @param logicaltid logical thread id
 *
 * @return index of the thread within its NUMA-region
 */
int
get_thread_index_in_numa(int logicaltid);

/**
 * Returns the NUMA-region id of the given logical thread id.
 *
 * @param logicaltid the logical thread id
 *
 * @return NUMA-region id
 */
int
get_numa_region_id(int logicaltid);

/**
 * Returns number of NUMA regions.
 *
 * @return
 */
int
get_num_numa_regions(void);


/**
 * Set the given thread by physical thread-id (i.e. returned from get_cpu_id())
 * as active in its NUMA-region.
 *
 * @param phytid physical thread id returned by get_cpu_id()
 */
void
numa_thread_mark_active(int phytid);

/**
 * Return the active number of threads in the given NUMA-region.
 *
 * @param numaregionid id of the NUMA-region
 *
 * @return active (i.e. running) number of threads in the NUMA-region.
 */
int
get_num_active_threads_in_numa(int numaregionid);

/**
 * Return the linearized index offset of the thread in the NUMA-topology mapping.
 *
 * @param logicaltid
 * @return numa index in 1d-NUMA mapping.
 */
int
get_numa_index_of_logical_thread(int logicaltid);

/**
 * Return the logical thread id from the linearized the NUMA-topology mapping.
 *
 * @param numaidx offset index in the 1d mapping
 * @return logical thread id
 */
int
get_logical_thread_at_numa_index(int numaidx);

/** @} */

#endif /* CPU_MAPPING_H */

