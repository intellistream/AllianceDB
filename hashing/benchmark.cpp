//
// Created by root on 11/29/19.
//

#include "benchmark.h"
#include "joins/prj_params.h"
#include "utils/generator.h"
#include <unistd.h>
#include <sys/resource.h>

/**
 * Put an odd number of cache lines between partitions in pass-2:
 * Here we put 3 cache lines.
 */

#define NO_MARTERIAL_SAMPLE

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

    MSG("[INFO ] %s relation with size = %.3lf MiB, #tuples = %lu : ",
        (loadfile != NULL) ? ("Loading") : ("Creating"),
        (double) sizeof(tuple_t) * rel_size / 1024.0 / 1024.0, rel_size)

    //    size_t relRsz = rel->num_tuples * sizeof(tuple_t)
    //                    + RELATION_PADDING(cmd_params.nthreads, cmd_params.part_fanout);
    rel->tuples = (tuple_t *) MALLOC(rel_size * sizeof(tuple_t));

    //    size_t relPlsz = relPl->num_tuples * sizeof(relation_payload_t)
    //                     + RELATION_PADDING(cmd_params.nthreads, cmd_params.part_fanout);
    //    rel->payload = (relation_payload_t *) malloc_aligned(relPlsz);

    /** second allocate the memory for relation payload **/
    //    size_t relPlRsz = relPl->num_tuples * sizeof(table_t)
    //                      + RELATION_PADDING(cmd_params.nthreads, cmd_params.part_fanout);
    //    relPl->rows = (table_t *) malloc_aligned(relPlRsz);

    //    size_t relTssz = relPl->num_tuples * sizeof(milliseconds)
    //                     + RELATION_PADDING(cmd_params.nthreads, cmd_params.part_fanout);
    relPl->ts = (uint64_t *) MALLOC(relPl->num_tuples * sizeof(uint64_t));

    //    /* NUMA-localize the input: */
    //    if(!nonumalocalize){
    //        numa_localize(relS.tuples, relS.num_tuples, cmd_params.nthreads);
    //    }

    if (loadfile != NULL && loadfile != "") {
        /* load relation from file */
        load_relation(rel, relPl, key, tsKey, loadfile, rel_size, partitions);
    } else if (cmd_params.fullrange_keys) {
        create_relation_nonunique(rel, rel_size, INT_MAX);
    } else if (cmd_params.nonunique_keys) {
        create_relation_nonunique(rel, rel_size, rel_size);
    } /*else if (cmd_params.gen_with_ts) {
        parallel_create_relation_with_ts(rel, relPl, rel->num_tuples, nthreads, rel->num_tuples, cmd_params.step_size, cmd_params.interval);
    } */ else if (cmd_params.ts) {
        // TODO: add a key duplicate function
        // idea: 1. generate a size = rel_size/duplicate_num key set. 2. duplicate key set to rel_size. 3. this generation is nothing to do with ts.

        // check params 1, window_size, 2. step_size, 3. interval, 4. distribution, 5. zipf factor, 6. nthreads
        switch (cmd_params.key_distribution) {
            case 0: // unique
                parallel_create_relation(rel, rel_size,
                                         nthreads,
                                         rel_size, cmd_params.duplicate_num);
                //                parallel_create_relation_with_ts(rel, relPl, rel->num_tuples, nthreads, rel->num_tuples,
                //                                                 cmd_params.step_size, cmd_params.interval);
                break;
            case 2: // zipf with zipf factor
                create_relation_zipf(rel, rel_size, rel_size, cmd_params.skew, cmd_params.duplicate_num);
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
                                 rel_size, cmd_params.duplicate_num);
        add_ts(rel, relPl, step_size, 0, partitions);
    }
}

void writefile(relation_payload_t *relPl, const param_t cmd_params) {
    string path = EXP_DIR "/datasets/Kim/data_distribution_zipf" + std::to_string(cmd_params.zipf_param) + ".txt";
    ofstream outputFile(path, std::ios::trunc);
    for (auto i = 0; i < relPl->num_tuples; i++) {
        outputFile << (std::to_string(relPl->ts[i]) + "\n");
    }
    outputFile.close();
}


uint32_t rand4sample(int &que_head, uint32_t *&rand_que)
{
#ifdef AVX_RAND
    using rand_t = uint32_t;
    static rand_t mod = int(1e9)+6, quelen = RANDOM_BUFFER_SIZE;
    // static rand_t *rand_que = NULL;
    static pxart::simd256::mt19937 rng{random_device{}};
    static constexpr auto rand_stp = sizeof(rng()) / sizeof(rand_t);

    if (rand_que == NULL)
    {
        rand_que = (rand_t *)malloc_aligned(sizeof(rand_t)*quelen+256);
    }
    if (que_head == quelen)
        que_head = 0;
    if (que_head == 0)
    {
        for (int i = 0; i < quelen; i += rand_stp)
        {
            // const auto vrnd = pxart::simd256::uniform<type>(rng, -9, 9);
            const auto vrnd = pxart::uniform<rand_t>(rng, 0, mod);
            // const auto srnd = pxart::pun_cast<array<rand_t, rand_stp>>(vrnd);
            // for (int j = 0; j < rand_stp; ++j) {
            //     rand_que[i+j] = srnd[j];
            memcpy(rand_que+i, &vrnd, sizeof(vrnd));
        }
    }
    return rand_que[que_head++];
#else
    static uint32_t mod = int(1e9)+7;
    return rand()%mod;
#endif
}


void setSampleParam(relation_t *relR, relation_t *relS, double epsilon_r, double epsilon_s, double &identical_Universal_p)
{
    std::map<intkey_t, int> hist[2];
    long long gm[3][3];
    memset(gm, 0, sizeof(gm));
    for (int i = 0; i < relR->num_tuples; ++i)
    {
        tuple_t tuple = relR->tuples[i];
        int tmp = ((hist[0]))[tuple.key];
        int tt = ((hist[1]))[tuple.key];
        ((hist[0]))[tuple.key] = tmp + 1;
        gm[1][1] += tt;
        gm[1][2] += 1ll*tt*tt;
        gm[2][1] += 2ll*tmp*tt + tt;
        gm[2][2] += 2ll*tmp*tt*tt + 1ll*tt*tt;
    }
    for (int i = 0; i < relS->num_tuples; ++i)
    {
        tuple_t tuple = relS->tuples[i];
        int tmp = ((hist[1]))[tuple.key];
        int tt = ((hist[0]))[tuple.key];
        ((hist[1]))[tuple.key] = tmp + 1;
        gm[1][1] += tt;
        gm[1][2] += 2ll*tmp*tt + tt;
        gm[2][1] += 1ll*tt*tt;
        gm[2][2] += 2ll*tmp*tt*tt + 1ll*tt*tt;
    }
    double p = (gm[2][2]*epsilon_r*epsilon_s - gm[2][1] - gm[1][2])/gm[1][1] + 1;
    p = sqrt(p);
    p = max(p, epsilon_r);
    p = max(p, epsilon_s);
    p = min(1.0, p);
    identical_Universal_p = p;
    // cout << epsilon_r <<" " << epsilon_s << endl;
    // cout << identical_Universal_p << endl;
    // fflush(stdout);
}

void sampleRelation(relation_t *rel, relation_payload_t *relPl, const
                    param_t &cmd_params, double epsilon, double identical_Universal_p,
                    double Universal_p, double Bernoulli_q, int hash_a, int hash_b)
{
    static int mod = 1e9+7;
    int que_head = 0;
    uint32_t *rand_que = NULL;
    uint32_t B, U;
    int cnt = 0;

#ifdef SET_PR
    B = Bernoulli_q * mod;
    U = Universal_p * mod;
    for (int i = 0; i < rel->num_tuples; ++i)
    {
      tuple_t tuple = rel->tuples[i];
      if( !(rand4sample(que_head, rand_que) >= B || (1ll*tuple.key*tuple.key%mod*hash_a%mod + hash_b)%mod >= U ) )
      {
        rel->tuples[cnt] = tuple;
        rel->payload->ts[cnt] = rel->payload->ts[i];
        cnt ++;
      }
    }
#else
    B = epsilon / identical_Universal_p * mod;
    U = identical_Universal_p * mod;
    for (int i = 0; i < rel->num_tuples; ++i)
    {
      tuple_t tuple = rel->tuples[i];
      if( !(rand4sample(que_head, rand_que) >= B || (1ll*tuple.key*tuple.key%mod*hash_a%mod + hash_b)%mod >= U ) )
      {
        rel->tuples[cnt] = tuple;
        rel->payload->ts[cnt] = rel->payload->ts[i];
        cnt ++;
      }

    }
#endif
    // cerr << Bernoulli_q <<" "<< identical_Universal_p << endl;
    // cerr << rel->num_tuples << " "<<cnt << endl;
    rel->num_tuples = rel->payload->num_tuples = cnt;
    // cerr << cnt << endl;
    if (rand_que != NULL)
        free(rand_que);
}

void *
memory_calculator_thread(void *args) {
    uint64_t exp_id = (uint64_t) args;
    struct rusage r_usage;
    int counter = 0;
    string path = EXP_DIR "/results/breakdown/mem_stat_" + std::to_string(exp_id) + ".txt";
    auto fp = fopen(path.c_str(), "w");
    setbuf(fp, NULL);

    while (counter < 1000000) {
        int ret = getrusage(RUSAGE_SELF, &r_usage);
        if (ret == 0) {
            fprintf(fp, "Memory usage: %ld kilobytes\n", r_usage.ru_maxrss);
            fflush(fp);
        } else
            printf("Error in getrusage. errno = %d\n", errno);
        counter++;
        usleep(10000);
    }
    return 0;
}

/**
 *
 * @param cmd_params
 */
void
benchmark(const param_t cmd_params) {

    srand(time(NULL));
    relation_t relR;
    relation_t relS;

    relR.payload = new relation_payload_t();
    relS.payload = new relation_payload_t();

    result_t *results;
    /* create relation R */


    if (cmd_params.old_param) {
        relR.num_tuples = cmd_params.r_size;
    } else {
        relR.num_tuples =
                (cmd_params.window_size / cmd_params.interval) * cmd_params.step_sizeR;
    }
    // check which fetcher is used, to decide whether need to partition ts.
    int partitions = cmd_params.nthreads;
    if (strstr(cmd_params.algo->name, "JM") != NULL) {
        partitions = 1;
    }

    createRelation(&relR, relR.payload, cmd_params.rkey, cmd_params.rts, cmd_params, cmd_params.loadfileR,
                   cmd_params.r_seed, cmd_params.step_sizeR, partitions);

    /* create relation S */
    if (cmd_params.old_param) {
        relS.num_tuples = cmd_params.s_size;
    } else {
        if (cmd_params.fixS)
            relS.num_tuples =
                    (cmd_params.window_size / cmd_params.interval) * cmd_params.step_sizeS;
        else
            relS.num_tuples = relR.num_tuples;
    }

    createRelation(&relS, relS.payload, cmd_params.skey, cmd_params.sts, cmd_params, cmd_params.loadfileS,
                   cmd_params.s_seed, cmd_params.step_sizeS, cmd_params.nthreads);

#ifdef MARTERIAL_SAMPLE
/*

    double epsilon_r;
    double epsilon_s;
    double Universal_p;
    double Bernoulli_q;
    int reservior_size;
    int rand_buffer_size;
    int presample_size;
*/
  double identical_Universal_p;
  int hash_a, hash_b;
  srand(time(NULL));
  hash_a = rand();
  hash_b = rand();
  setSampleParam(&relR, &relS, cmd_params.epsilon_r, cmd_params.epsilon_s, identical_Universal_p);
  
  // std::cout << "\n\n\n\n\n\n\n\n SET PARAM DONE \n\n\n\n\n\n\n\n\n";
  // fflush(stdout);

  sampleRelation(&relR, relR.payload, cmd_params, cmd_params.epsilon_r, identical_Universal_p, cmd_params.Universal_p,
                 cmd_params.Bernoulli_q, hash_a, hash_b);
  sampleRelation(&relS, relS.payload, cmd_params, cmd_params.epsilon_s, identical_Universal_p, cmd_params.Universal_p,
                 cmd_params.Bernoulli_q, hash_a, hash_b);

#endif

    DEBUGMSG("relR [aligned:%d]: %s", is_aligned(relR.tuples, CACHE_LINE_SIZE),
             print_relation(relR.tuples, min((uint64_t) 1000, relR.num_tuples)).c_str());
    DEBUGMSG("relS [aligned:%d]: %s", is_aligned(relS.tuples, CACHE_LINE_SIZE),
             print_relation(relS.tuples, min((uint64_t) 1000, relS.num_tuples)).c_str());

    //    string path = EXP_DIR "/datasets/Kim/data_distribution_zipf" + std::to_string(cmd_params.zipf_param) + ".txt";
    //    writefile(relR.payload, cmd_params);

#ifdef PROFILE_MEMORY_CONSUMPTION
    // create a thread for memory consumption
    pthread_t thread_id;
    uint32_t rv = pthread_create(&thread_id, nullptr, memory_calculator_thread, (void *) cmd_params.exp_id);
    if (rv) {
        fprintf(stderr, "[ERROR] pthread_create() return code is %d\n", rv);
        exit(-1);
    }

    if (strcmp(cmd_params.algo->name, "NPO") == 0 || strcmp(cmd_params.algo->name, "PRO") == 0) {
        sleep(1);
    }
#endif


    /* Run the selected join algorithm */
    MSG("[INFO ] Running join algorithm %s ...", cmd_params.algo->name);

    //    uint64_t ts = 0;
    //    for (auto i = 0; i < relS.num_tuples; i++) {
    //        auto read = &relS.tuples[i];
    //        auto read_ts = relS.payload->ts[read->payloadID];
    //        if (read_ts >= ts) {
    //            ts = read_ts;
    //        } else {
    //            printf("\nts is not monotonically increasing since:%d, "
    //                   " S:%lu\n", i,   read_ts);
    //            break;
    //        }
    //    }
    //    fflush(stdout);

    if (cmd_params.ts == 0)
        results = cmd_params.algo->joinAlgo(&relR, &relS, cmd_params);//no window to wait.
    else
        results = cmd_params.algo->joinAlgo(&relR, &relS, cmd_params);

    MSG("[INFO ] Results = %ld. DONE.", results->totalresults);

    /* clean-up */
    delete_relation(&relR);
    delete_relation(&relS);
    delete_relation_payload(relR.payload);
    delete_relation_payload(relS.payload);
    free(results);

    //    results = join_from_file(cmd_params, cmd_params.loadfileR, cmd_params.loadfileS,
    //            cmd_params.rkey, cmd_params.skey, cmd_params.r_size, cmd_params.s_size);


#if (defined(PERSIST_RELATIONS) && defined(JOIN_RESULT_MATERIALIZE))
    MSG("[INFO ] Persisting the join result to \"Out.tbl\" ...\n");
    write_result_relation(results, "Out.tbl");
#endif

#ifdef JOIN_RESULT_MATERIALIZE
    free(results->resultlist);
#endif
}

