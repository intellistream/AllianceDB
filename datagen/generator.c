#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sched.h>              /* CPU_ZERO, CPU_SET */
#include <pthread.h>            /* pthread_attr_setaffinity_np */
#include <stdio.h>              /* perror */
#include <stdlib.h>             /* RAND_MAX */
#include <math.h>               /* fmod, pow */
#include <time.h>               /* time() */
#include <unistd.h>             /* getpagesize() */
#include <string.h>             /* memcpy() */

#include "generator.h"
#include "cpu_mapping.h"        /* get_cpu_id() */
#include "affinity.h"           /* pthread_attr_setaffinity_np */
#include "genzipf.h"            /* gen_zipf() */
#include "lock.h"
#include "barrier.h"


/* return a random number in range [0,N] */
#define RAND_RANGE(N) ((double)rand() / ((double)RAND_MAX + 1) * (N))
#define RAND_RANGE48(N,STATE) ((double)nrand48(STATE)/((double)RAND_MAX+1)*(N))


static int seeded = 0;
static unsigned int seedValue;

void 
seed_generator(unsigned int seed) 
{
    srand(seed);
    seedValue = seed;
    seeded = 1;
}

/** Check wheter seeded, if not seed the generator with current time */
static void
check_seed()
{
    if(!seeded) {
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
knuth_shuffle(relation_t * relation)
{
    int i;
    for (i = relation->num_tuples - 1; i > 0; i--) {
        int64_t  j              = RAND_RANGE(i);
        intkey_t tmp            = relation->tuples[i].key;
        relation->tuples[i].key = relation->tuples[j].key;
        relation->tuples[j].key = tmp;
    }
}

void 
knuth_shuffle48(relation_t * relation, unsigned short * state)
{
    int i;
    for (i = relation->num_tuples - 1; i > 0; i--) {
        int64_t  j              = RAND_RANGE48(i, state);
        intkey_t tmp            = relation->tuples[i].key;
        relation->tuples[i].key = relation->tuples[j].key;
        relation->tuples[j].key = tmp;
    }
}

/**
 * Generate unique tuple IDs with Knuth shuffling
 * relation must have been allocated
 */
void
random_unique_gen(relation_t *rel) 
{
    uint64_t i;

    for (i = 0; i < rel->num_tuples; i++) {
        rel->tuples[i].key = (i+1);
    }

    /* randomly shuffle elements */
    knuth_shuffle(rel);
}

struct create_arg_t {
    relation_t          rel;
    int64_t             firstkey;
    int64_t             maxid;
    relation_t *        fullrel;
    volatile void *     locks;
    pthread_barrier_t * barrier;
};

typedef struct create_arg_t create_arg_t;


/** NaN masks */
static int64_t NaNExpMask = (0x7FFL << 52U);
static int64_t NaNlowbitclear = ~(1L << 52U);

/** Avoid NaN in values, since tuples are treated as double for AVX instructions */
void
avoid_NaN(int64_t * data)
{
    /* avoid NaN in values */
    int64_t * bitpattern = (int64_t *) data;
    if(((*bitpattern) & NaNExpMask) == NaNExpMask){
        *bitpattern &= NaNlowbitclear;
    }
}

/**
 * Create random unique keys starting from firstkey 
 */
void *
random_unique_gen_thread(void * args) 
{
    create_arg_t * arg      = (create_arg_t *) args;
    relation_t *   rel      = & arg->rel;
    int64_t        firstkey = arg->firstkey;
    int64_t        maxid    = arg->maxid;
    uint64_t i;

    value_t randstart = 5; /* rand() % 1000; */

    /* for randomly seeding nrand48() */
    unsigned short state[3] = {0, 0, 0};
    unsigned int seed       = time(NULL) + * (unsigned int *) pthread_self();
    memcpy(state, &seed, sizeof(seed));
    
    for (i = 0; i < rel->num_tuples; i++) {
        rel->tuples[i].key     = firstkey;
        rel->tuples[i].payload = randstart + i;

        if(firstkey == maxid)
            firstkey = 0;

        firstkey ++;
    }

    /* randomly shuffle elements */
    /* knuth_shuffle48(rel, state); */

    /* wait at a barrier until all threads finish initializing data */
    int rv;
    BARRIER_ARRIVE(arg->barrier, rv);

    /* parallel synchronized knuth-shuffle */
    volatile char * locks   = (volatile char *)(arg->locks);
    relation_t *    fullrel = arg->fullrel;

    uint64_t rel_offset_in_full = rel->tuples - fullrel->tuples;
    uint64_t k = rel_offset_in_full + rel->num_tuples - 1;
    for (i = rel->num_tuples - 1; i > 0; i--, k--) {
        int64_t  j = RAND_RANGE48(k, state);
        lock(locks+k);  /* lock this rel-idx=i, fullrel-idx=k */
        lock(locks+j);  /* lock full rel-idx=j */

        intkey_t tmp           = fullrel->tuples[k].key;
        fullrel->tuples[k].key = fullrel->tuples[j].key;
        fullrel->tuples[j].key = tmp;

        unlock(locks+j);
        unlock(locks+k);
    }

    return 0;
}

/** 
 * Just initialize mem. to 0 for making sure it will be allocated numa-local 
 */
void *
numa_localize_thread(void * args) 
{
    create_arg_t * arg = (create_arg_t *) args;
    relation_t *   rel = & arg->rel;
    uint64_t i;
    
    for (i = 0; i < rel->num_tuples; i++) {
        rel->tuples[i].key = 0;
    }

    return 0;
}

/**
 * Write relation to a file.
 */
void
write_relation(relation_t * rel, char * filename)
{
    FILE * fp = fopen(filename, "a");
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
random_gen(relation_t *rel, const int64_t maxid) 
{
    uint64_t i;

    for (i = 0; i < rel->num_tuples; i++) {
        rel->tuples[i].key = RAND_RANGE(maxid);
        rel->tuples[i].payload = rel->num_tuples - i;

        /* avoid NaN in values */
        avoid_NaN((int64_t*)&(rel->tuples[i]));
    }
}

int 
create_relation_pk(relation_t *relation, int64_t num_tuples) 
{
    check_seed();

    relation->num_tuples = num_tuples;
    
    if (!relation->tuples) {
        perror("memory must be allocated first");
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
                         uint32_t nthreads, uint64_t maxid) 
{
    int rv;
    uint32_t i;
    uint64_t offset = 0;
    
    check_seed();

    relation->num_tuples = num_tuples;


    if (!relation->tuples) {
        perror("memory must be allocated first");
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

    pagesize        = getpagesize();
    npages          = (num_tuples * sizeof(tuple_t)) / pagesize + 1;
    npages_perthr   = npages / nthreads;
    ntuples_perthr  =  npages_perthr * (pagesize/sizeof(tuple_t));

    if(npages_perthr == 0) 
       ntuples_perthr = num_tuples / nthreads;

    ntuples_lastthr = num_tuples - ntuples_perthr * (nthreads-1);

    pthread_attr_init(&attr);

    rv = pthread_barrier_init(&barrier, NULL, nthreads);
    if(rv != 0){
        printf("[ERROR] Couldn't create the barrier\n");
        exit(EXIT_FAILURE);
    }


    volatile void * locks = (volatile void *)calloc(num_tuples, sizeof(char));

    for( i = 0; i < nthreads; i++ ) {
        int cpu_idx = get_cpu_id(i);
        
        CPU_ZERO(&set);
        CPU_SET(cpu_idx, &set);
        pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &set);

        args[i].firstkey       = (offset + 1) % maxid;
        args[i].maxid          = maxid;
        args[i].rel.tuples     = relation->tuples + offset;
        args[i].rel.num_tuples = (i == nthreads-1) ? ntuples_lastthr 
                                 : ntuples_perthr;

        args[i].fullrel = relation;
        args[i].locks   = locks;
        args[i].barrier = &barrier;

        offset += ntuples_perthr;

        rv = pthread_create(&tid[i], &attr, random_unique_gen_thread, 
                            (void*)&args[i]);
        if (rv){
            fprintf(stderr, "[ERROR] pthread_create() return code is %d\n", rv);
            exit(-1);
        }
    }

    for(i = 0; i < nthreads; i++){
        pthread_join(tid[i], NULL);
    }

    /* randomly shuffle elements */
    /* knuth_shuffle(relation); */

    /* clean up */
    free((char*)locks);
    pthread_barrier_destroy(&barrier);

#ifdef PERSIST_RELATIONS
    char * const tables[] = {"R.tbl", "S.tbl"};
    static int rs = 0;
    write_relation(relation, tables[(rs++)%2]);
#endif

    return 0;
}

int 
numa_localize(tuple_t * relation, int64_t num_tuples, uint32_t nthreads) 
{
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

    pagesize        = getpagesize();
    npages          = (num_tuples * sizeof(tuple_t)) / pagesize + 1;
    npages_perthr   = npages / nthreads;
    ntuples_perthr  = npages_perthr * (pagesize/sizeof(tuple_t));
    ntuples_lastthr = num_tuples - ntuples_perthr * (nthreads-1);

    pthread_attr_init(&attr);

    for( i = 0; i < nthreads; i++ ) {
        int cpu_idx = get_cpu_id(i);
        
        CPU_ZERO(&set);
        CPU_SET(cpu_idx, &set);
        pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &set);

        args[i].firstkey       = offset + 1;
        args[i].rel.tuples     = relation + offset;
        args[i].rel.num_tuples = (i == nthreads-1) ? ntuples_lastthr 
                                 : ntuples_perthr;
        offset += ntuples_perthr;

        rv = pthread_create(&tid[i], &attr, numa_localize_thread, 
                            (void*)&args[i]);
        if (rv){
            fprintf(stderr, "[ERROR] pthread_create() return code is %d\n", rv);
            exit(-1);
        }
    }

    for(i = 0; i < nthreads; i++){
        pthread_join(tid[i], NULL);
    }

    return 0;
}


int 
create_relation_fk(relation_t *relation, int64_t num_tuples, const int64_t maxid)
{
    int32_t i, iters;
    int64_t remainder;
    relation_t tmp;

    check_seed();

    relation->num_tuples = num_tuples;
      
    if (!relation->tuples) { 
        perror("memory must be allocated first");
        return -1; 
    }
  
    /* alternative generation method */
    iters = num_tuples / maxid;
    for(i = 0; i < iters; i++){
        tmp.num_tuples = maxid;
        tmp.tuples = relation->tuples + maxid * i;
        random_unique_gen(&tmp);
    }

    /* if num_tuples is not an exact multiple of maxid */
    remainder = num_tuples % maxid;
    if(remainder > 0) {
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
                           int64_t num_tuples) 
{
    int i, iters;
    int64_t remainder;

    if (!fkrel->tuples) {
        perror("memory must be allocated first");
        return -1; 
    }

    fkrel->num_tuples = num_tuples;

    /* alternative generation method */
    iters = num_tuples / pkrel->num_tuples;
    for(i = 0; i < iters; i++){
        memcpy(fkrel->tuples + i * pkrel->num_tuples, pkrel->tuples, 
               pkrel->num_tuples * sizeof(tuple_t));
    }

    /* if num_tuples is not an exact multiple of pkrel->num_tuples */
    remainder = num_tuples % pkrel->num_tuples;
    if(remainder > 0) {
        memcpy(fkrel->tuples + i * pkrel->num_tuples, pkrel->tuples, 
               remainder * sizeof(tuple_t));
    }

    knuth_shuffle(fkrel);

    return 0;
}

int create_relation_nonunique(relation_t *relation, int64_t num_tuples,
                              const int64_t maxid) 
{
    check_seed();

    relation->num_tuples = num_tuples;
    
    if (!relation->tuples) { 
        perror("memory must be allocated first");
        return -1; 
    }

    random_gen(relation, maxid);

    return 0;
}

double 
zipf_ggl(double * seed) 
{
    double t, d2=0.2147483647e10;
    t = *seed;
    t = fmod(0.16807e5*t, d2);
    *seed = t;
    return (t-1.0e0)/(d2-1.0e0);
}

int 
create_relation_zipf(relation_t * relation, int64_t num_tuples,
                     const int64_t maxid, const double zipf_param) 
{
    check_seed();

    relation->num_tuples = num_tuples;
    
    if (!relation->tuples) {
        perror("memory must be allocated first");
        return -1; 
    }

    gen_zipf(num_tuples, maxid, zipf_param, &relation->tuples);
    
    /* write_relation(relation, "S128M-skew1.tbl"); */
    return 0;
}
