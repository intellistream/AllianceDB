/**
 * @file    memalloc.h
 * @author  Cagri Balkesen <cagri.balkesen@inf.ethz.ch>
 * @date    Sat Dec 15 13:39:54 2012
 * @version $Id $
 *
 * @brief   Common memory allocation functions and macros.
 *
 * (c) 2012-2014, ETH Zurich, Systems Group
 *
 */
#ifndef MEMALLOC_H_
#define MEMALLOC_H_

#define MALLOC(X) malloc_aligned(X)
#define FREE(X,SZ) free(X)

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

#endif /* MEMALLOC_H_ */
