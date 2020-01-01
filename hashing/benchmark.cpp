//
// Created by root on 11/29/19.
//

#include "benchmark.h"

int check_avx() {
    unsigned int eax, ebx, ecx, edx;
    if (!__get_cpuid(1, &eax, &ebx, &ecx, &edx))
        return 1;

    /* Run AVX test only if host has AVX runtime support.  */
    if ((ecx & AVXFlag) != AVXFlag)
        return 0; /* missing feature */

    return 1; /* has AVX support! */
}

void createRelation(relation_t *rel, relation_payload_t *relPl, int32_t key, const param_t &cmd_params,
                    char *loadfile, uint64_t rel_size, uint32_t seed) {
    fprintf(stdout,
            "[INFO ] %s relation with size = %.3lf MiB, #tuples = %llu : ",
            (loadfile != NULL) ? ("Loading") : ("Creating"),
            (double) sizeof(tuple_t) * rel_size / 1024.0 / 1024.0, rel_size);
    fflush(stdout);

    seed_generator(seed);

    /* to pass information to the create_relation methods */
    numalocalize = cmd_params.basic_numa;
    nthreads = cmd_params.nthreads;


    /** first allocate the memory for relations (+ padding based on numthreads) : */
    rel->num_tuples = cmd_params.r_size;
    size_t relRsz = rel->num_tuples * sizeof(tuple_t)
                    + RELATION_PADDING(cmd_params.nthreads, cmd_params.part_fanout);

    rel->tuples = (tuple_t *) malloc_aligned(relRsz);

    /** second allocate the memory for relation payload **/
    // TODO: not sure whether is correct
    relPl->num_tuples = cmd_params.r_size;
    size_t relPlRsz = relPl->num_tuples * sizeof(table_t)
                    + RELATION_PADDING(cmd_params.nthreads, cmd_params.part_fanout);
    relPl->rows = (table_t *) malloc_aligned(relPlRsz);

    //    /* NUMA-localize the input: */
    //    if(!nonumalocalize){
    //        numa_localize(relS.tuples, relS.num_tuples, cmd_params.nthreads);
    //    }

    if (loadfile != NULL) {
        /* load relation from file */
        load_relation(rel, relPl, key, loadfile, rel_size);
    } else if (cmd_params.fullrange_keys) {
        create_relation_nonunique(rel, rel_size, INT_MAX);
    } else if (cmd_params.nonunique_keys) {
        create_relation_nonunique(rel, rel_size, rel_size);
    } else {
        //create_relation_pk(&rel, rel_size);
        parallel_create_relation(rel, rel_size,
                                 nthreads,
                                 rel_size);
    }
    printf("OK \n");
}

/**
 *
 * @param cmd_params
 */
void
benchmark(const param_t cmd_params) {
    relation_t relR;
    relation_t relS;

    relR.payload = new relation_payload_t();
    relS.payload = new relation_payload_t();

    result_t *results;
    // TODO: generate dataset
    /* create relation R */
    createRelation(&relR, relR.payload, cmd_params.rkey, cmd_params, cmd_params.loadfileR, cmd_params.r_size,
                   cmd_params.r_seed);
    DEBUGMSG("relR [aligned:%d]: %s", is_aligned(relR.tuples, CACHE_LINE_SIZE),
             print_relation(relR.tuples, cmd_params.r_size).c_str())

    /* create relation S */
    createRelation(&relS, relS.payload, cmd_params.skey, cmd_params, cmd_params.loadfileS, cmd_params.s_size,
                   cmd_params.s_seed);
    DEBUGMSG("relS [aligned:%d]: %s", is_aligned(relS.tuples, CACHE_LINE_SIZE),
             print_relation(relS.tuples, cmd_params.s_size).c_str())

    // TODO: Execute query with dataset, need to submit a join function

    /* Run the selected join algorithm */
    printf("[INFO ] Running join algorithm %s ...\n", cmd_params.algo->name);

    results = cmd_params.algo->joinAlgo(&relR, &relS, cmd_params.nthreads);

    printf("[INFO ] Results = %ld. DONE.\n", results->totalresults);

    /* clean-up */
    delete_relation(&relR);
    delete_relation(&relS);
    delete_relation_payload(relR.payload);
    delete_relation_payload(relS.payload);
    free(results);

//    results = join_from_file(cmd_params, cmd_params.loadfileR, cmd_params.loadfileS,
//            cmd_params.rkey, cmd_params.skey, cmd_params.r_size, cmd_params.s_size);
}

