/* @version $Id: generator.c 4546 2013-12-07 13:56:09Z bcagri $ */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sched.h>              /* CPU_ZERO, CPU_SET */
#include <pthread.h>            /* pthread_attr_setaffinity_np */
#include <stdio.h>              /* perror */
#include <stdlib.h>             /* posix_memalign */
#include <math.h>               /* fmod, pow */
#include <time.h>               /* time() */
#include <unistd.h>             /* getpagesize() */
#include <string.h>             /* memcpy() */

#include "cpu_mapping.h"        /* get_cpu_id() */
#include "generator.h"          /* create_relation_*() */
//#include "affinity.h"           /* pthread_attr_setaffinity_np */
#include "genzipf.h"            /* gen_zipf() */
#include "lock.h"
#include "../joins/prj_params.h"         /* RELATION_PADDING for Parallel Radix */
#include "barrier.h"

/* return a random number in range [0,N] */
#define RAND_RANGE(N) ((double)rand() / ((double)RAND_MAX + 1) * (N))
#define RAND_RANGE48(N, STATE) ((double)nrand48(STATE)/((double)RAND_MAX+1)*(N))
#define MALLOC(SZ) alloc_aligned(SZ+RELATION_PADDING) /*malloc(SZ+RELATION_PADDING)*/
#define FREE(X, SZ) free(X)

#ifndef BARRIER_ARRIVE
/** barrier wait macro */
#define BARRIER_ARRIVE(B, RV)                            \
    RV = pthread_barrier_wait(B);                       \
    if(RV !=0 && RV != PTHREAD_BARRIER_SERIAL_THREAD){  \
        printf("Couldn't wait on barrier\n");           \
        exit(EXIT_FAILURE);                             \
    }
#endif

/* Uncomment the following to persist input relations to disk. */
/* #define PERSIST_RELATIONS 1 */

/** An experimental feature to allocate input relations numa-local */
int numalocalize;
int nthreads;

static int seeded = 0;
static unsigned int seedValue;

void *
alloc_aligned(size_t size) {
    void *ret;
    int rv;
    rv = posix_memalign((void **) &ret, CACHE_LINE_SIZE, size);

    if (rv) {
        perror("[ERROR] alloc_aligned() failed: out of memory");
        return 0;
    }

    /** Not an elegant way of passing whether we will numa-localize, but this
        feature is experimental anyway. */
    if (numalocalize) {
        tuple_t *mem = (tuple_t *) ret;
        uint64_t ntuples = size / sizeof(tuple_t);
        numa_localize(mem, ntuples, nthreads);
    }

    return ret;
}

void
seed_generator(unsigned int seed) {
    srand(seed);
    seedValue = seed;
    seeded = 1;
}

/** Check wheter seeded, if not seed the generator with current time */
static void
check_seed() {
    if (!seeded) {
        seedValue = time(NULL);
        srand(seedValue);
        seeded = 1;
    }
}


/**
 * Shuffle tuples of the relation using Knuth shuffle.
 *
 * @param relation
 */
void
knuth_shuffle(relation_t *relation) {
    int i;
    for (i = relation->num_tuples - 1; i > 0; i--) {
        int64_t j = RAND_RANGE(i);
        intkey_t tmp = relation->tuples[i].key;
        relation->tuples[i].key = relation->tuples[j].key;
        relation->tuples[j].key = tmp;
    }
}

void
knuth_shuffle48(relation_t *relation, unsigned short *state) {
    int i;
    for (i = relation->num_tuples - 1; i > 0; i--) {
        int64_t j = RAND_RANGE48(i, state);
        intkey_t tmp = relation->tuples[i].key;
        relation->tuples[i].key = relation->tuples[j].key;
        relation->tuples[j].key = tmp;
    }
}

/**
 * Generate unique tuple IDs with Knuth shuffling
 * relation must have been allocated
 */
void
random_unique_gen(relation_t *rel) {
    uint64_t i;

    for (i = 0; i < rel->num_tuples; i++) {
        rel->tuples[i].key = (i + 1);
        rel->tuples[i].payload = i;
    }

    /* randomly shuffle elements */
    knuth_shuffle(rel);
}

struct create_arg_t {
    relation_t rel;
    int64_t firstkey;
    int64_t maxid;
    uint64_t ridstart;
    relation_t *fullrel;
    volatile void *locks;
    pthread_barrier_t *barrier;
};

typedef struct create_arg_t create_arg_t;

/**
 * Create random unique keys starting from firstkey
 */
void *
random_unique_gen_thread(void *args) {
    create_arg_t *arg = (create_arg_t *) args;
    relation_t *rel = &arg->rel;
    int64_t firstkey = arg->firstkey;
    int64_t maxid = arg->maxid;
    uint64_t ridstart = arg->ridstart;
    uint64_t i;

    /* for randomly seeding nrand48() */
    unsigned short state[3] = {0, 0, 0};
    unsigned int seed = time(NULL) + *(unsigned int *) pthread_self();
    memcpy(state, &seed, sizeof(seed));

    for (i = 0; i < rel->num_tuples; i++) {
        rel->tuples[i].key = firstkey;
        rel->tuples[i].payload = ridstart + i;

        if (firstkey == maxid)
            firstkey = 0;

        firstkey++;
    }

    /* randomly shuffle elements */
    /* knuth_shuffle48(rel, state); */

    /* wait at a barrier until all threads finish initializing data */
    BARRIER_ARRIVE(arg->barrier, i);

    /* parallel synchronized knuth-shuffle */
    volatile char *locks = (volatile char *) (arg->locks);
    relation_t *fullrel = arg->fullrel;

    uint64_t rel_offset_in_full = rel->tuples - fullrel->tuples;
    uint64_t k = rel_offset_in_full + rel->num_tuples - 1;
    for (i = rel->num_tuples - 1; i > 0; i--, k--) {
        int64_t j = RAND_RANGE48(k, state);
        lock(locks + k);  /* lock this rel-idx=i, fullrel-idx=k */
        lock(locks + j);  /* lock full rel-idx=j */

        intkey_t tmp = fullrel->tuples[k].key;
        fullrel->tuples[k].key = fullrel->tuples[j].key;
        fullrel->tuples[j].key = tmp;

        unlock(locks + j);
        unlock(locks + k);
    }

    return 0;
}

/**
 * Just initialize mem. to 0 for making sure it will be allocated numa-local
 */
void *
numa_localize_thread(void *args) {
    create_arg_t *arg = (create_arg_t *) args;
    relation_t *rel = &arg->rel;
    uint64_t i;

    for (i = 0; i < rel->num_tuples; i++) {
        rel->tuples[i].key = 0;
    }

    return 0;
}


/**
 * Read a 2-column relation from a file, rel is already allocated (preferably
 * NUMA-aware).
 */
void
read_relation(relation_t *rel, char *filename);

/**
 * Write relation to a file.
 */
void
write_relation(relation_t *rel, char *filename) {
    FILE *fp = fopen(filename, "w");
    uint64_t i;

    fprintf(fp, "#KEY, VAL\n");

    for (i = 0; i < rel->num_tuples; i++) {
        fprintf(fp, "%d %d\n", rel->tuples[i].key, rel->tuples[i].payload);
    }

    fclose(fp);
}

/**
 * Generate tuple IDs -> random distribution
 * relation must have been allocated
 */
void
random_gen(relation_t *rel, const int64_t maxid) {
    uint64_t i;

    for (i = 0; i < rel->num_tuples; i++) {
        rel->tuples[i].key = RAND_RANGE(maxid);
        rel->tuples[i].payload = i;
    }
}

int
create_relation_pk(relation_t *relation, int64_t num_tuples) {

    check_seed();

    relation->num_tuples = num_tuples;
    relation->tuples = (tuple_t *) MALLOC(relation->num_tuples * sizeof(tuple_t));

    if (!relation->tuples) {
        perror("out of memory");
        return -1;
    }

    random_unique_gen(relation);

#ifdef PERSIST_RELATIONS
    write_relation(relation, "R.tbl");
#endif

    return 0;
}

int
parallel_create_relation(relation_t *relation, uint64_t num_tuples,
                         uint32_t nthreads, uint64_t maxid) {
    int rv;
    uint32_t i;
    uint64_t offset = 0;

    check_seed();

    relation->num_tuples = num_tuples;

    /* we need aligned allocation of items */
    relation->tuples = (tuple_t *) MALLOC(num_tuples * sizeof(tuple_t));

    if (!relation->tuples) {
        perror("out of memory");
        return -1;
    }

    create_arg_t args[nthreads];
    pthread_t tid[nthreads];
    cpu_set_t set;
    pthread_attr_t attr;
    pthread_barrier_t barrier;

    unsigned int pagesize;
    unsigned int npages;
    unsigned int npages_perthr;
    uint64_t ntuples_perthr;
    uint64_t ntuples_lastthr;

    pagesize = getpagesize();
    npages = (num_tuples * sizeof(tuple_t)) / pagesize + 1;
    npages_perthr = npages / nthreads;
    ntuples_perthr = npages_perthr * (pagesize / sizeof(tuple_t));

    if (npages_perthr == 0)
        ntuples_perthr = num_tuples / nthreads;

    ntuples_lastthr = num_tuples - ntuples_perthr * (nthreads - 1);

    pthread_attr_init(&attr);

    rv = pthread_barrier_init(&barrier, NULL, nthreads);
    if (rv != 0) {
        printf("[ERROR] Couldn't create the barrier\n");
        exit(EXIT_FAILURE);
    }


    volatile void *locks = (volatile void *) calloc(num_tuples, sizeof(char));

    for (i = 0; i < nthreads; i++) {
        int cpu_idx = get_cpu_id(i);

        CPU_ZERO(&set);
        CPU_SET(cpu_idx, &set);
        pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &set);

        args[i].firstkey = (offset + 1) % maxid;
        args[i].maxid = maxid;
        args[i].ridstart = offset;
        args[i].rel.tuples = relation->tuples + offset;
        args[i].rel.num_tuples = (i == nthreads - 1) ? ntuples_lastthr
                                                     : ntuples_perthr;

        args[i].fullrel = relation;
        args[i].locks = locks;
        args[i].barrier = &barrier;

        offset += ntuples_perthr;

        rv = pthread_create(&tid[i], &attr, random_unique_gen_thread,
                            (void *) &args[i]);
        if (rv) {
            fprintf(stderr, "[ERROR] pthread_create() return code is %d\n", rv);
            exit(-1);
        }
    }

    for (i = 0; i < nthreads; i++) {
        pthread_join(tid[i], NULL);
    }

    /* randomly shuffle elements */
    /* knuth_shuffle(relation); */

    /* clean up */
    free((char *) locks);
    pthread_barrier_destroy(&barrier);

#ifdef PERSIST_RELATIONS
    char * const tables[] = {"R.tbl", "S.tbl"};
    static int rs = 0;
    write_relation(relation, tables[(rs++)%2]);
#endif

    return 0;
}

int
load_relation(relation_t *relation, char *filename, uint64_t num_tuples) {
    relation->num_tuples = num_tuples;

    /* we need aligned allocation of items */
    relation->tuples = (tuple_t *) MALLOC(num_tuples * sizeof(tuple_t));

    if (!relation->tuples) {
        perror("out of memory");
        return -1;
    }

    /* load from the given input file */
    read_relation(relation, filename);

    return 0;
}

int
numa_localize(tuple_t *relation, int64_t num_tuples, uint32_t nthreads) {
    uint32_t i, rv;
    uint64_t offset = 0;

    /* we need aligned allocation of items */
    create_arg_t args[nthreads];
    pthread_t tid[nthreads];
    cpu_set_t set;
    pthread_attr_t attr;

    unsigned int pagesize;
    unsigned int npages;
    unsigned int npages_perthr;
    uint64_t ntuples_perthr;
    uint64_t ntuples_lastthr;

    pagesize = getpagesize();
    npages = (num_tuples * sizeof(tuple_t)) / pagesize + 1;
    npages_perthr = npages / nthreads;
    ntuples_perthr = npages_perthr * (pagesize / sizeof(tuple_t));
    ntuples_lastthr = num_tuples - ntuples_perthr * (nthreads - 1);

    pthread_attr_init(&attr);

    for (i = 0; i < nthreads; i++) {
        int cpu_idx = get_cpu_id(i);

        CPU_ZERO(&set);
        CPU_SET(cpu_idx, &set);
        pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &set);

        args[i].firstkey = offset + 1;
        args[i].rel.tuples = relation + offset;
        args[i].rel.num_tuples = (i == nthreads - 1) ? ntuples_lastthr
                                                     : ntuples_perthr;
        offset += ntuples_perthr;

        rv = pthread_create(&tid[i], &attr, numa_localize_thread,
                            (void *) &args[i]);
        if (rv) {
            fprintf(stderr, "[ERROR] pthread_create() return code is %d\n", rv);
            exit(-1);
        }
    }

    for (i = 0; i < nthreads; i++) {
        pthread_join(tid[i], NULL);
    }

    return 0;
}


int
create_relation_fk(relation_t *relation, int64_t num_tuples, const int64_t maxid) {
    int32_t i, iters;
    int64_t remainder;
    relation_t tmp;

    check_seed();

    relation->num_tuples = num_tuples;
    relation->tuples = (tuple_t *) MALLOC(relation->num_tuples * sizeof(tuple_t));

    if (!relation->tuples) {
        perror("out of memory");
        return -1;
    }

    /* alternative generation method */
    iters = num_tuples / maxid;
    for (i = 0; i < iters; i++) {
        tmp.num_tuples = maxid;
        tmp.tuples = relation->tuples + maxid * i;
        random_unique_gen(&tmp);
    }

    /* if num_tuples is not an exact multiple of maxid */
    remainder = num_tuples % maxid;
    if (remainder > 0) {
        tmp.num_tuples = remainder;
        tmp.tuples = relation->tuples + maxid * iters;
        random_unique_gen(&tmp);
    }

#ifdef PERSIST_RELATIONS
    write_relation(relation, "S.tbl");
#endif

    return 0;
}

/**
 * Create a foreign-key relation using the given primary-key relation and
 * foreign-key relation size. Keys in pkrel is randomly distributed in the full
 * integer range.
 *
 * @param fkrel [output] foreign-key relation
 * @param pkrel [input] primary-key relation
 * @param num_tuples
 *
 * @return
 */
int
create_relation_fk_from_pk(relation_t *fkrel, relation_t *pkrel,
                           int64_t num_tuples) {
    int rv, i, iters;
    int64_t remainder;

    rv = posix_memalign((void **) &fkrel->tuples, CACHE_LINE_SIZE,
                        num_tuples * sizeof(tuple_t) + RELATION_PADDING);

    if (rv && !fkrel->tuples) {
        perror("[ERROR] Out of memory");
        return -1;
    }

    fkrel->num_tuples = num_tuples;

    /* alternative generation method */
    iters = num_tuples / pkrel->num_tuples;
    for (i = 0; i < iters; i++) {
        memcpy(fkrel->tuples + i * pkrel->num_tuples, pkrel->tuples,
               pkrel->num_tuples * sizeof(tuple_t));
    }

    /* if num_tuples is not an exact multiple of pkrel->num_tuples */
    remainder = num_tuples % pkrel->num_tuples;
    if (remainder > 0) {
        memcpy(fkrel->tuples + i * pkrel->num_tuples, pkrel->tuples,
               remainder * sizeof(tuple_t));
    }

    knuth_shuffle(fkrel);

    return 0;
}

int create_relation_nonunique(relation_t *relation, int64_t num_tuples,
                              const int64_t maxid) {
    check_seed();

    relation->num_tuples = num_tuples;
    relation->tuples = (tuple_t *) MALLOC(relation->num_tuples * sizeof(tuple_t));

    if (!relation->tuples) {
        perror("out of memory");
        return -1;
    }

    random_gen(relation, maxid);

    return 0;
}

double
zipf_ggl(double *seed) {
    double t, d2 = 0.2147483647e10;
    t = *seed;
    t = fmod(0.16807e5 * t, d2);
    *seed = t;
    return (t - 1.0e0) / (d2 - 1.0e0);
}

int
create_relation_zipf(relation_t *relation, int64_t num_tuples,
                     const int64_t maxid, const double zipf_param) {
    check_seed();

    relation->num_tuples = num_tuples;
    relation->tuples = (tuple_t *) MALLOC(relation->num_tuples * sizeof(tuple_t));

    if (!relation->tuples) {
        perror("out of memory");
        return -1;
    }

    gen_zipf(num_tuples, maxid, zipf_param, &relation->tuples);

    return 0;
}

void
delete_relation(relation_t *rel) {
    /* clean up */
    FREE(rel->tuples, rel->num_tuples * sizeof(tuple_t));
}

void
read_relation(relation_t *rel, char *filename) {

    FILE *fp = fopen(filename, "r");

    /* skip the header line */
    char c;
    do {
        c = fgetc(fp);
    } while (c != '\n');

    /* search for a whitespace for "key payload" format */
    int fmtspace = 0;
    int fmtcomma = 0;
    do {
        c = fgetc(fp);
        if (c == ' ') {
            fmtspace = 1;
            break;
        }
        if (c == ',') {
            fmtcomma = 1;
            break;
        }
    } while (c != '\n');

    /* rewind back to the beginning and start parsing again */
    rewind(fp);
    /* skip the header line */
    do {
        c = fgetc(fp);
    } while (c != '\n');

    uint64_t ntuples = rel->num_tuples;
    intkey_t key;
    value_t payload = 0;
    int warn = 1;
    for (uint64_t i = 0; i < ntuples; i++) {
        if (fmtspace) {
            fscanf(fp, "%d %d", &key, &payload);
        } else if (fmtcomma) {
            fscanf(fp, "%d,%d", &key, &payload);
        } else {
            fscanf(fp, "%d", &key);
        }

        if (warn && key < 0) {
            warn = 0;
            printf("[WARN ] key=%d, payload=%d\n", key, payload);
        }
        rel->tuples[i].key = key;
        rel->tuples[i].payload = payload;
    }

    fclose(fp);

}
