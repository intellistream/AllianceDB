#include <stdlib.h>             /* malloc, posix_memalign */
#include <stdio.h>              /* perror */

//#include "jemalloc/jemalloc.h"

#ifdef HAVE_LIBNUMA
#include <numa.h>               /* numa_alloc_local(), numa_free() */
#endif

#include "memalloc.h"
#include "params.h"  /* parameters, macro defs. */

void *
malloc_aligned(size_t size)
{
    void * ret;
    int rv;
    rv = posix_memalign((void**)&ret, CACHE_LINE_SIZE, size);

    if (rv) {
        perror("[ERROR] malloc_aligned() failed: out of memory");
        return 0;
    }

    return ret;
}

void *
malloc_aligned_threadlocal(size_t size)
{
#if HAVE_LIBNUMA
    return numa_alloc_local(size);
#else
    return malloc_aligned(size);
#endif
}

void
free_threadlocal(void * ptr, size_t size)
{
#if HAVE_LIBNUMA
    numa_free(ptr, size);
#else
    free(ptr);
#endif
}
