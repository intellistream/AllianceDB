//
// Created by Shuhao Zhang on 8/9/19.
//

#ifndef ALLIANCEDB_MEMALLOC_H
#define ALLIANCEDB_MEMALLOC_H


#define MALLOC(X) malloc_aligned(X)
#define FREE(X,SZ) free(X)

#include <sys/types.h>

/**
 * Cache-line aligned memory allocation with posix_memalign()
 *
 * @param size
 */
void *
malloc_aligned(size_t size);

/**
 * Thread local, cache-line aligned memory allocation with posix_memalign()
 * Uses libNUMA if available, otherwise just malloc_aligned().
 *
 * @param size
 */
void *
malloc_aligned_threadlocal(size_t size);

/**
 * Releases thread-local allocated memory.
 *
 * @param ptr
 * @param size
 */
void
free_threadlocal(void * ptr, size_t size);

#endif //ALLIANCEDB_MEMALLOC_H
