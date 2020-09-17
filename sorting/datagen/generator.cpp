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
#include <assert.h>

#include "generator.h"
#include "../affinity/cpu_mapping.h"        /* get_cpu_id() */
//#include "../affinity/affinity.h"           /* pthread_attr_setaffinity_np */
#include "../affinity/memalloc.h"
#include "../joins/common_functions.h"
#include "../utils/barrier.h"
#include "../utils/lock.h"
#include "genzipf.h" /* gen_zipf() */

/* return a random number in range [0,N] */
#define RAND_RANGE(N) ((double)rand() / ((double)RAND_MAX + 1) * (N))
#define RAND_RANGE48(N, STATE) ((double)nrand48(STATE)/((double)RAND_MAX+1)*(N))
#define FREE(X, SZ) free(X)

#include <iostream>
#include <sstream>
#include <vector>
#include <chrono>
#include <assert.h>

using namespace std;
using namespace std::chrono;

#ifndef BARRIER_ARRIVE
/** barrier wait macro */
#define BARRIER_ARRIVE(B, RV)                           \
    RV = pthread_barrier_wait(B);                       \
    if(RV !=0 && RV != PTHREAD_BARRIER_SERIAL_THREAD){  \
        printf("Couldn't wait on barrier\n");           \
        exit(EXIT_FAILURE);                             \
    }
#endif

/* Uncomment the following to persist input relations to disk. */
//#define PERSIST_RELATIONS 1

/** An experimental feature to allocate input relations numa-local */
int numalocalize;
int nthreads;

static int seeded = 0;
static unsigned int seedValue;


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

inline bool last_thread(int i, int nthreads) {
    return i == (nthreads - 1);
}

// TODO: going ot
void ts_shuffle(relation_t *relation, relation_payload_t *relationPayload, uint32_t partitions) {
    int numthr = relation->num_tuples / partitions;//replicate R, partition S.

    int *tid_offsets = new int[partitions];
    int *tid_start_idx = new int[partitions];
    int *tid_end_idx = new int[partitions];

    for (auto partition = 0; partition < partitions; partition++) {
        tid_offsets[partition] = 0;
        tid_start_idx[partition] = numthr * partition;
        tid_end_idx[partition] = (last_thread(partition, partitions)) ? relation->num_tuples : numthr * (partition + 1);

        printf("partition %d start idx: %d end idx: %d\n", partition, tid_start_idx[partition], tid_end_idx[partition]);
    }

    for (auto i = 0; i < relation->num_tuples; i++) {

    }
}

void
add_ts(relation_t *relation, relation_payload_t *relationPayload, int step_size, int interval, uint32_t partitions) {
    int ts = 0;

    int numthr = relation->num_tuples / partitions;//replicate R, partition S.

    int *tid_offsets = new int[partitions];
    int *tid_start_idx = new int[partitions];
    int *tid_end_idx = new int[partitions];

    for (auto partition = 0; partition < partitions; partition++) {
        tid_offsets[partition] = 0;
        tid_start_idx[partition] = numthr * partition;
        tid_end_idx[partition] = (last_thread(partition, partitions)) ? relation->num_tuples : numthr * (partition + 1);

        DEBUGMSG(1, "partition %d start idx: %d end idx: %d\n", partition, tid_start_idx[partition],
                 tid_end_idx[partition]);
    }

    for (auto i = 0; i < relation->num_tuples; i++) {
        if (i % (step_size) == 0) {
            ts += interval * 2.1 * 1E6;//2.1GHz, 1 milliseconds to ticks.
        }
        // round robin to assign ts to each thread.
        auto partition = i % partitions;
        if (tid_start_idx[partition] + tid_offsets[partition] == tid_end_idx[partition]) {
            partition = partitions - 1; // if reach the maximum size, append the tuple to the last partition
        }
        // record cur index in partition
        int cur_index = tid_start_idx[partition] + tid_offsets[partition];
        relationPayload->ts[cur_index] = (uint64_t) ts;
        tid_offsets[partition]++;
    }
#ifdef DEBUG
    for (auto i = 0; i < relation->num_tuples; i++) {
        DEBUGMSG(0, "ts: %ld\n", relationPayload->ts[i]);
    }
#endif
//    assert(interval == 0 || ts == window_size);
}

/**
 * @param numThr
 * @param relation
 * @param relationPayload
 * @param window_size
 * @param zipf_param
 */
void add_zipf_ts(relation_t *relation, relation_payload_t *relationPayload, int window_size, const double zipf_param,
                 uint32_t partitions) {

    int small = 0;
    int32_t *timestamps = gen_zipf_ts(relation->num_tuples, window_size, zipf_param);

    int numthr = relation->num_tuples / partitions;//replicate R, partition S.

    int *tid_offsets = new int[partitions];
    int *tid_start_idx = new int[partitions];
    int *tid_end_idx = new int[partitions];

    for (auto partition = 0; partition < partitions; partition++) {
        tid_offsets[partition] = 0;
        tid_start_idx[partition] = numthr * partition;
        tid_end_idx[partition] = (last_thread(partition, partitions)) ? relation->num_tuples : numthr * (partition + 1);
    }

    for (auto i = 0; i < relation->num_tuples; i++) {
        // round robin to assign ts to each thread.
        int partition = i % partitions;
        if (tid_start_idx[partition] + tid_offsets[partition] == tid_end_idx[partition]) {
            partition = partitions - 1; // if reach the maximum size, append the tuple to the last partition
        }

        // record cur index in partition
        int cur_index = tid_start_idx[partition] + tid_offsets[partition];
        relationPayload->ts[cur_index] = (uint64_t) timestamps[i] * 2.1 * 1E6;//ms to cycle
        tid_offsets[partition]++;

        DEBUGMSG(0, "%d, %ld\n", relation->tuples[i].key, relationPayload->ts[i]);
    }
    for (auto i = 0; i < relation->num_tuples; i++) {
        if (relationPayload->ts[i] < 0.25 * window_size * 2.1 * 1E6) {
            small++;
        }
    }
    printf("small ts %f\n", (double) small / relation->num_tuples);
}

/**
 * @param relation relation
 * @param step_size number of tuples should be generated each interval
 * @param interval interval of each timestamp generate step
 * @param numThr numeber of threads
 */
void
random_gen_with_ts(relation_t *rel, relation_payload_t *relPl, int64_t maxid, int step_size, int interval, int numThr) {
    uint64_t i;

    for (i = 0; i < rel->num_tuples; i++) {
        rel->tuples[i].key = RAND_RANGE(maxid);
        rel->tuples[i].payloadID = i;//payload is simply the id of the tuple.
    }

    add_ts(rel, relPl, step_size, interval, 0);
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
        rel->tuples[i].payloadID = i;
    }

    /* randomly shuffle elements */
    knuth_shuffle(rel);

    // timestamp append on shuffled data

}

struct create_arg_t {
    relation_t rel;
    int64_t firstkey;
    int64_t maxid;
    uint64_t ridstart;
    relation_t *fullrel;
    volatile void *locks;
    pthread_barrier_t *barrier;
    uint64_t offset;
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
    auto offset = arg->offset;

    uint64_t i;

//    value_t randstart = 5; /* rand() % 1000; */

    /* for randomly seeding nrand48() */
    unsigned short state[3] = {0, 0, 0};
    unsigned int seed = seedValue;//time(NULL) + *(unsigned int *) pthread_self(); //TODO: Not sure why Ca's original implementation use time as the seed.
    memcpy(state, &seed, sizeof(seed));

    for (i = 0; i < rel->num_tuples; i++) {
        rel->tuples[i].key = firstkey;
        rel->tuples[i].payloadID = firstkey;
        if (rel->tuples[i].payloadID < 0) {
            printf("Something is wrong.");
        }
        if (firstkey == maxid)
            firstkey = 0;

        firstkey++;
    }

    /* randomly shuffle elements */
    /* knuth_shuffle48(rel, state); */

    /* wait at a barrier until all threads finish initializing data */
    int rv;
    BARRIER_ARRIVE(arg->barrier, rv);

//    /* parallel synchronized knuth-shuffle */
//    volatile char *locks = (volatile char *) (arg->locks);
//    relation_t *fullrel = arg->fullrel;
//
//    uint64_t rel_offset_in_full = rel->tuples - fullrel->tuples;
//    uint64_t k = rel_offset_in_full + rel->num_tuples - 1;
//    for (i = rel->num_tuples - 1; i > 0; i--, k--) {
//        int64_t j = RAND_RANGE48(k, state);
//        lock(locks + k);  /* lock this rel-idx=i, fullrel-idx=k */
//        lock(locks + j);  /* lock full rel-idx=j */
//
//        intkey_t tmp = fullrel->tuples[k].key;
//        fullrel->tuples[k].key = fullrel->tuples[j].key;
//        fullrel->tuples[j].key = tmp;
//
//        unlock(locks + j);
//        unlock(locks + k);
//    }

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
read_relation(relation_t *rel, relation_payload_t *relPl, int32_t keyby, int32_t tsKey, char *filename,
              uint32_t partitions);

/**
 * Write relation to a file.
 */
void
write_relation(relation_t *rel, char *filename) {
    FILE *fp = fopen(filename, "w");
    uint64_t i;

    fprintf(fp, "#KEY, VAL\n");

    for (i = 0; i < rel->num_tuples; i++) {
        fprintf(fp, "%d %d\n", rel->tuples[i].key, rel->tuples[i].payloadID);
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
        rel->tuples[i].payloadID = i;//payload is simply the id of the tuple.
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
parallel_create_relation(relation_t *reln, uint64_t ntuples, uint32_t nthreads, uint64_t maxid,
                         const int duplicate_num) {
    int rv;
    uint32_t i;
    uint64_t offset = 0;

    check_seed();

    // only generate a segment tuples first
    reln->num_tuples = ntuples / duplicate_num;


    if (!reln->tuples) {
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

    pagesize = getpagesize();
    npages = (reln->num_tuples * sizeof(tuple_t)) / pagesize + 1;
    npages_perthr = npages / nthreads;
    ntuples_perthr = npages_perthr * (pagesize / sizeof(tuple_t));

    if (npages_perthr == 0)
        ntuples_perthr = reln->num_tuples / nthreads;

    ntuples_lastthr = reln->num_tuples - ntuples_perthr * (nthreads - 1);
    pthread_attr_init(&attr);

    rv = pthread_barrier_init(&barrier, NULL, nthreads);
    if (rv != 0) {
        printf("[ERROR] Couldn't create the barrier\n");
        exit(EXIT_FAILURE);
    }


    volatile void *locks = (volatile void *) calloc(reln->num_tuples, sizeof(char));

    for (i = 0; i < nthreads; i++) {
        int cpu_idx = get_cpu_id(i);
        CPU_ZERO(&set);
        CPU_SET(cpu_idx, &set);
        rv = pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &set);
        if (rv) {
            fprintf(stderr, "[ERROR] set affinity error return code is %d\n", rv);
            exit(-1);
        }
        args[i].firstkey = (offset + 1) % maxid;
        args[i].maxid = maxid;
        args[i].rel.tuples = reln->tuples + offset;
        args[i].rel.num_tuples = (i == nthreads - 1) ? ntuples_lastthr
                                                     : ntuples_perthr;

        args[i].fullrel = reln;
        args[i].locks = locks;
        args[i].barrier = &barrier;

        offset += ntuples_perthr;

        rv |= pthread_create(&tid[i], &attr, random_unique_gen_thread,
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
    fprintf(stdout, "writing out relations\n");
    char * const tables[] = {"R.tbl", "S.tbl"};
    static int rs = 0;
    write_relation(relation, tables[(rs++)%2]);
#endif

    duplicate(reln, ntuples, duplicate_num);
    return 0;
}

int
parallel_create_relation_with_ts(relation_t *relation, relation_payload_t *relationPayload, uint64_t num_tuples,
                                 uint32_t nthreads, uint64_t maxid, int step_size, int interval) {
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
        rv = pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &set);
        if (rv) {
            fprintf(stderr, "[ERROR] set affinity error return code is %d\n", rv);
            exit(-1);
        }
        args[i].firstkey = (offset + 1) % maxid;
        args[i].maxid = maxid;
        args[i].rel.tuples = relation->tuples + offset;
        args[i].rel.num_tuples = (i == nthreads - 1) ? ntuples_lastthr
                                                     : ntuples_perthr;
        args[i].offset = offset;
        args[i].fullrel = relation;
        args[i].locks = locks;
        args[i].barrier = &barrier;

        offset += ntuples_perthr;

        rv |= pthread_create(&tid[i], &attr, random_unique_gen_thread,
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
    fprintf(stdout, "writing out relations\n");
    char * const tables[] = {"R.tbl", "S.tbl"};
    static int rs = 0;
    write_relation(relation, tables[(rs++)%2]);
#endif

    add_ts(relation, relationPayload, step_size, interval, 0);

//    add_zipf_ts(relation, relationPayload, num_tuples/step_size*interval, nthreads, 1);

    return 0;
}

int
load_relation(relation_t *relation, relation_payload_t *relation_payload, int32_t keyby, int32_t tsKey, char *filename,
              uint64_t num_tuples, uint32_t partitions) {
    relation->num_tuples = num_tuples;

    /* we need aligned allocation of items */
    relation->tuples = (tuple_t *) MALLOC(num_tuples * sizeof(tuple_t));

    relation_payload->num_tuples = num_tuples;
//    relation_payload->rows = (table_t *) MALLOC(num_tuples * sizeof(table_t));

    if (!relation->tuples /*|| !relation_payload->rows*/) {
//    if (!relation->tuples) {
        perror("out of memory");
        return -1;
    }

    MSG("load from the given input file %s", filename)
    read_relation(relation, relation_payload, keyby, tsKey, filename, partitions);

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


int
create_relation_nonunique(relation_t *relation, int64_t num_tuples,
                          const int64_t maxid) {
    check_seed();

    relation->num_tuples = num_tuples;

    if (!relation->tuples) {
        perror("memory must be allocated first");
        return -1;
    }

    random_gen(relation, maxid);

    return 0;
}

int
create_relation_nonunique_with_ts(relation_t *relation, relation_payload_t *relationPayload, int64_t num_tuples,
                                  const int numThr,
                                  const int64_t maxid, const int step_size, const int interval) {
    check_seed();

    relation->num_tuples = num_tuples;

    if (!relation->tuples) {
        perror("memory must be allocated first");
        return -1;
    }

    random_gen_with_ts(relation, relationPayload, maxid, step_size, interval, numThr);

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

void duplicate(relation_t *reln, uint64_t ntuples, const int duplicate_num) {// duplicate generated tuples
    auto num_tuple_perseg = reln->num_tuples;
    if (duplicate_num > 1) {
//        for (auto i = 0; i < num_tuple_perseg; i++) {
//            for (auto j = 1; j < duplicate_num; j++) {
//                auto index = j * num_tuple_perseg + i;
//                reln->tuples[index] = reln->tuples[i];
//            }
//        }
        for (auto i = 1; i < duplicate_num; i++) {
            for (auto j = 0; j < num_tuple_perseg; j++) {
                reln->tuples[num_tuple_perseg * i + j] = reln->tuples[j];
            }
        }
    }
    reln->num_tuples = ntuples;

//    for (auto i=0; i < reln->num_tuples; i++) {
//        DEBUGMSG("%dth: %d\n", i, reln->tuples[i].key);
//    }
    /* randomly shuffle elements */
    knuth_shuffle(reln);
}

int
create_relation_zipf(relation_t *reln, int64_t ntuples, const int64_t maxid, const double zipfparam,
                     const int duplicate_num) {
    check_seed();

    // only generate a segment tuples first
    reln->num_tuples = ntuples / duplicate_num;

    reln->tuples = (tuple_t *) MALLOC(reln->num_tuples * sizeof(tuple_t));

    if (!reln->tuples) {
        perror("out of memory");
        return -1;
    }

    gen_zipf(ntuples, maxid, zipfparam, &reln->tuples);
    duplicate(reln, ntuples, duplicate_num);
    return 0;
}

void
delete_relation(relation_t *rel) {
    /* clean up */
    FREE(rel->tuples, rel->num_tuples * sizeof(tuple_t));
}

void
delete_relation_payload(relation_payload_t *relPl) {
    /* clean up */
    FREE(relPl->ts, relPl->num_tuples * sizeof(milliseconds));
}

// for string delimiter
vector<string> split(string s, string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    string token;
    vector<string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != string::npos) {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }

    res.push_back(s.substr(pos_start));
    return res;
}

void
read_relation(relation_t *rel, relation_payload_t *relPl, int32_t keyby, int32_t tsKey, char *filename,
              uint32_t partitions) {
    printf("reading file: %s", filename);
    FILE *fp = fopen(filename, "r");
    /* skip the header line */
    char c;
//    do {
//        c = fgetc(fp);
//    } while (c != '\n');

    /* search for a whitespace for "key payload" format */
    int fmtspace = 0;
    int fmtcomma = 0;
    int fmtbar = 0;
    do {
        c = fgetc(fp);
//        if (c == ' ') {
//            fmtspace = 1;
//            break;
//        }
        if (c == ',') {
            fmtcomma = 1;
            break;
        }
        if (c == '|') {
            fmtbar = 1;
            break;
        }
    } while (c != '\n');

    char *line;
    size_t len;
    ssize_t read;

    /* rewind back to the beginning and start parsing again */
    rewind(fp);
    /* skip the header line */
//    do {
//        c = fgetc(fp);
//    } while (c != '\n');

    uint64_t ntuples = rel->num_tuples;
    intkey_t key;
    table_t row = table_t();
    uint64_t timestamp = (uint64_t) 0;

    // add a index field, here payload is index field, row is real payload
    int32_t payload = 0;

    int warn = 1;
    int i = 0;

    int numthr = rel->num_tuples / partitions;//replicate R, partition S.

    int *tid_offsets = new int[partitions];
    int *tid_start_idx = new int[partitions];
    int *tid_end_idx = new int[partitions];

    for (auto partition = 0; partition < partitions; partition++) {
        tid_offsets[partition] = 0;
        tid_start_idx[partition] = numthr * partition;
        tid_end_idx[partition] = (last_thread(partition, partitions)) ? rel->num_tuples : numthr * (partition + 1);
    }
    int small = 0;
    u_int64_t maxTS = 0;

    std::ifstream file(filename);
    std::string str;
    while (std::getline(file, str) && i < ntuples) /*read = getline(&line, &len, fp)) != -1 && */ {
//        printf("Retrieved line of length %zu:\n", read);
//        printf("%s", str.c_str());
        if (fmtcomma) {
            key = stoi(split(str, ",")[keyby]);
            strcpy(row.value, line);
            payload = i;
            if (tsKey != 0) {
                timestamp = stol(split(str, ",")[tsKey]) * 2.1 * 1E6;
//                if (timestamp != 0)
//                    printf("%lld \n", timestamp);
            }
        } else if (fmtbar) {
            key = stoi(split(str, "|")[keyby]);
            strcpy(row.value, line);
            payload = i;
            if (tsKey != 0) {
                timestamp = stol(split(str, "|")[tsKey]) * 2.1 * 1E6;
//                if (timestamp != 0)
//                    printf("%lld \n", timestamp);
            }
        } else {
            printf("error!!\n");
            return;
        }
//        relPl->rows[i] = row;
//        relPl->ts[i] = timestamp;
        if (timestamp < 0.5 * 10500000000) {
            small++;
        }
        if (timestamp > maxTS) {
            maxTS = timestamp;
        }
        // round robin to assign ts to each thread.
        auto partition = i % partitions;
        int cur_index = tid_start_idx[partition] + tid_offsets[partition];

        if (cur_index == tid_end_idx[partition]) {
            partition = partitions - 1; // if reach the maximum size, append the tuple to the last partition
            cur_index = tid_start_idx[partition] + tid_offsets[partition];
        }
        // record cur index in partition
//        if (cur_index == 80049) {
//            printf("??");
//        }
        relPl->ts[cur_index] = timestamp;
        rel->tuples[cur_index].key = key;
        rel->tuples[cur_index].payloadID = payload;
        tid_offsets[partition]++;

        i++;//id of record being read.
    }
    printf("small%d, ts %f\n", small, (double) small / rel->num_tuples);
    printf("maxts:%f\n", maxTS / (2.1 * 1E6));
    fclose(fp);
}

void *alloc_aligned(size_t size) {
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



