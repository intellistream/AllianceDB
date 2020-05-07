//
// Created by Shuhao Zhang on 22/11/19.
//

#include "gtest/gtest.h"
#include "../utils/types.h"
#include "../utils/generator.h"
#include "../utils/params.h"
#include "../joins/common_functions.h"
#include "../joins/onlinejoins.h"

int size = 1280;

relation_t relR;
relation_t relS;
result_t *results;

tuple_t *
generate_rand_tuples(int num) {

    relation_t rel;
    rel.tuples = (tuple_t *) malloc(sizeof(tuple_t) * num);
    rel.num_tuples = num;
    uint64_t i;

    for (i = 0; i < num; i++) {
        rel.tuples[i].key = (i + 1);
        rel.tuples[i].payloadID = i;
    }

    /* randomly shuffle elements */
    knuth_shuffle(&rel);

    return rel.tuples;
}

void setup() {
    /* start initially on CPU-0 */
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(0, &set);
    if (sched_setaffinity(0, sizeof(set), &set) < 0) {
        perror("sched_setaffinity");
    }

    /* create relation R */
    relR.num_tuples = size;
    relR.tuples = generate_rand_tuples(size);
    DEBUGMSG("relR [aligned:%d]: %s", is_aligned(relR.tuples, CACHE_LINE_SIZE),
             print_relation(relR.tuples, size).c_str())

    /* create relation S */
    relS.num_tuples = size;
    relS.tuples = generate_rand_tuples(size);
    DEBUGMSG("relS [aligned:%d]: %s", is_aligned(relS.tuples, CACHE_LINE_SIZE),
             print_relation(relS.tuples, size).c_str())
}

void cleanup() {
    /* clean-up */
    delete_relation(&relR);
    delete_relation(&relS);
    free(results);
}

TEST(JOIN_TEST, RPJ_JB_NP) {
    setup();
    /* Run the selected join algorithm */
//    MSG("[INFO ] Running join algorithm %s ...\n", "RPJ_JB_NP");
    results = RPJ_JB_NP(&relR, &relS, 2);
//    MSG("[INFO ] Results = %ld. DONE.\n", results->totalresults);
    EXPECT_EQ(results->totalresults, size);
    cleanup();
}

TEST(JOIN_TEST, SHJ_ST) {
    setup();
    /* Run the selected join algorithm */
//    MSG("[INFO ] Running join algorithm %s ...\n", "SHJ");
    results = SHJ_st(&relR, &relS, 2);
//    MSG("[INFO ] Results = %ld. DONE.\n", results->totalresults);
    EXPECT_EQ(results->totalresults, size);
    cleanup();
}

GTEST_API_ int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}