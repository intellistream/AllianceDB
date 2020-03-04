//
// Created by root on 11/29/19.
//

#include "benchmark.h"
#include "joins/prj_params.h"
#include "utils/generator.h"

/**
 * Put an odd number of cache lines between partitions in pass-2:
 * Here we put 3 cache lines.
 */
#define SMALL_PADDING_TUPLES (3 * CACHE_LINE_SIZE/sizeof(tuple_t))
#define PADDING_TUPLES (SMALL_PADDING_TUPLES*(FANOUT_PASS2+1))
#define RELATION_PADDING (PADDING_TUPLES*FANOUT_PASS1*sizeof(tuple_t))

/* return a random number in range [0,N] */
#define RAND_RANGE(N) ((double)rand() / ((double)RAND_MAX + 1) * (N))
#define RAND_RANGE48(N, STATE) ((double)nrand48(STATE)/((double)RAND_MAX+1)*(N))
#define MALLOC(SZ) alloc_aligned(SZ+RELATION_PADDING) /*malloc(SZ+RELATION_PADDING)*/
#define FREE(X, SZ) free(X)


int check_avx() {
    unsigned int eax, ebx, ecx, edx;
    if (!__get_cpuid(1, &eax, &ebx, &ecx, &edx))
        return 1;

    /* Run AVX test only if host has AVX runtime support.  */
    if ((ecx & AVXFlag) != AVXFlag)
        return 0; /* missing feature */

    return 1; /* has AVX support! */
}

// TODO: why need so many parameters, only need cmd_params inside.
void createRelation(relation_t *rel, relation_payload_t *relPl, int32_t key, int32_t tsKey, const param_t &cmd_params,
                    char *loadfile, uint32_t seed, const int step_size, int partitions) {
    seed_generator(seed);
    /* to pass information to the create_relation methods */
    numalocalize = cmd_params.basic_numa;
    nthreads = cmd_params.nthreads;

//    if (cmd_params.kim) {
    // calculate num of tuples by params
//    if (cmd_params.step_size < nthreads) {
//        perror("step size should be bigger than the number of threads!");
//        return;
//    }
    uint64_t rel_size = rel->num_tuples;
    relPl->num_tuples = rel->num_tuples;

    fprintf(stdout,
            "[INFO ] %s relation with size = %.3lf MiB, #tuples = %llu : ",
            (loadfile != NULL) ? ("Loading") : ("Creating"),
            (double) sizeof(tuple_t) * rel_size / 1024.0 / 1024.0, rel_size);
    fflush(stdout);

//    size_t relRsz = rel->num_tuples * sizeof(tuple_t)
//                    + RELATION_PADDING(cmd_params.nthreads, cmd_params.part_fanout);
    rel->tuples = (tuple_t *) MALLOC(rel->num_tuples * sizeof(tuple_t));

//    size_t relPlsz = relPl->num_tuples * sizeof(relation_payload_t)
//                     + RELATION_PADDING(cmd_params.nthreads, cmd_params.part_fanout);
//    rel->payload = (relation_payload_t *) malloc_aligned(relPlsz);

    /** second allocate the memory for relation payload **/
//    size_t relPlRsz = relPl->num_tuples * sizeof(table_t)
//                      + RELATION_PADDING(cmd_params.nthreads, cmd_params.part_fanout);
//    relPl->rows = (table_t *) malloc_aligned(relPlRsz);

//    size_t relTssz = relPl->num_tuples * sizeof(milliseconds)
//                     + RELATION_PADDING(cmd_params.nthreads, cmd_params.part_fanout);
    relPl->ts = (milliseconds *) MALLOC(relPl->num_tuples * sizeof(tuple_t));

    //    /* NUMA-localize the input: */
    //    if(!nonumalocalize){
    //        numa_localize(relS.tuples, relS.num_tuples, cmd_params.nthreads);
    //    }

    if (loadfile != NULL && loadfile != "") {
        /* load relation from file */
        load_relation(rel, relPl, key, tsKey, loadfile, rel_size);
    } else if (cmd_params.fullrange_keys) {
        create_relation_nonunique(rel, rel_size, INT_MAX);
    } else if (cmd_params.nonunique_keys) {
        create_relation_nonunique(rel, rel_size, rel_size);
    } /*else if (cmd_params.gen_with_ts) {
        parallel_create_relation_with_ts(rel, relPl, rel->num_tuples, nthreads, rel->num_tuples, cmd_params.step_size, cmd_params.interval);
    } */ else if (cmd_params.kim) {
        // check params 1, window_size, 2. step_size, 3. interval, 4. distribution, 5. zipf factor, 6. nthreads
        switch (cmd_params.key_distribution) {
            case 0: // unique
                parallel_create_relation(rel, rel_size,
                                         nthreads,
                                         rel_size);
//                parallel_create_relation_with_ts(rel, relPl, rel->num_tuples, nthreads, rel->num_tuples,
//                                                 cmd_params.step_size, cmd_params.interval);
                break;
            case 2: // zipf with zipf factor
                create_relation_zipf(rel, rel_size, rel_size, cmd_params.skew);
                break;
            default:
                break;
        }

        switch (cmd_params.ts_distribution) {
            case 0: // uniform
                add_ts(rel, relPl, step_size, cmd_params.interval, partitions);
                break;
            case 2: // zipf
                add_zipf_ts(rel, relPl, cmd_params.window_size, cmd_params.zipf_param, partitions);
                break;
            default:
                break;
        }
    } else {
        //create_relation_pk(&rel, rel_size);
        parallel_create_relation(rel, rel_size,
                                 nthreads,
                                 rel_size);
        add_ts(rel, relPl, step_size, 0, 0);
    }
    printf("OK \n");
}

void writefile(relation_payload_t *relPl, const param_t cmd_params) {
    string path = "/data1/xtra/datasets/Kim/data_distribution_zipf" + std::to_string(cmd_params.zipf_param) + ".txt";
    ofstream outputFile(path, std::ios::trunc);
    for (auto i=0; i<relPl->num_tuples; i++) {
        outputFile << (std::to_string(relPl->ts[i].count()) + "\n");
    }
    outputFile.close();
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
    /* create relation R */

    if (cmd_params.old_param) {
        relR.num_tuples = cmd_params.r_size;
    } else {
        relR.num_tuples = (cmd_params.window_size / cmd_params.interval) * cmd_params.step_sizeR;
    }
    createRelation(&relR, relR.payload, cmd_params.rkey, cmd_params.rts, cmd_params, cmd_params.loadfileR,
                   cmd_params.r_seed, cmd_params.step_sizeR, nthreads);
    DEBUGMSG("relR [aligned:%d]: %s", is_aligned(relR.tuples, CACHE_LINE_SIZE),
             print_relation(relR.tuples, min((uint64_t) 1000, cmd_params.r_size)).c_str());


    /* create relation S */
    if (cmd_params.old_param) {
        relS.num_tuples = cmd_params.s_size;
    } else {

        if (cmd_params.fixS)
            relS.num_tuples = cmd_params.r_size;
        else
            relS.num_tuples = (cmd_params.window_size / cmd_params.interval) * cmd_params.step_sizeS;
    }

    // check which fetcher is used, to decide whether need to partition ts.
    int partitions = nthreads;
    if (strstr(cmd_params.algo->name, "JM") != NULL) {
        partitions = 1;
    }

    createRelation(&relS, relS.payload, cmd_params.skey, cmd_params.sts, cmd_params, cmd_params.loadfileS,
                   cmd_params.s_seed, cmd_params.step_sizeS, partitions);
    DEBUGMSG("relS [aligned:%d]: %s", is_aligned(relS.tuples, CACHE_LINE_SIZE),
             print_relation(relS.tuples, min((uint64_t) 1000, cmd_params.s_size)).c_str());

//    string path = "/data1/xtra/datasets/Kim/data_distribution_zipf" + std::to_string(cmd_params.zipf_param) + ".txt";
//    writefile(relR.payload, cmd_params);

    /* Run the selected join algorithm */
    printf("[INFO ] Running join algorithm %s ...\n", cmd_params.algo->name);

    results = cmd_params.algo->joinAlgo(&relR, &relS, cmd_params.nthreads, cmd_params.exp_id, cmd_params.group_size);

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

