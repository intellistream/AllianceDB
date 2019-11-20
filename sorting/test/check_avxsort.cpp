//#include <stdint.h>
//#include <stdlib.h>
//#include <check.h>
//#include <stdio.h>
//#include <time.h>
///* #include <limits.h> */
//
//#include "testutil.h"
//#include "../joins/avxsort.h"
//#include "../joins/avxsort_multiway.h"
//
///* #define CK_FORK "no" */
//#define MAXTESTSIZE (1<<22)
//
//START_TEST (test_avxsort_int32)
//    {
//        int64_t sz = rand() % MAXTESTSIZE;
//        int32_t *in = generate_rand_int32(sz);
//        int32_t *out = calloc(sz, sizeof(int32_t));
//        avxsort_int32(&in, &out, sz);
//        ck_assert_int_eq (is_sorted_int32(out, sz), 1);
//        free(in);
//        free(out);
//    }
//
//END_TEST

//START_TEST(test_avxsort_int64)
//    {
//        int64_t sz = rand() % MAXTESTSIZE;
//        int64_t *in = generate_rand_int64(sz);
//        int64_t *out = calloc(sz, sizeof(int64_t));
//        avxsort_int64(&in, &out, sz);
//        ck_assert_int_eq(is_sorted_int64(out, sz), 1);
//        free(in);
//        free(out);
//    }
//
//END_TEST
//
//START_TEST(test_avxsort_tuples)
//    {
//        int64_t sz = rand() % MAXTESTSIZE;
//        tuple_t *in = generate_rand_tuples(sz);
//        tuple_t *out = malloc(sz * sizeof(tuple_t));;
//        avxsort_tuples(&in, &out, sz);
//        ck_assert_int_eq(is_sorted_tuples(out, sz), 1);
//        free(in);
//        free(out);
//
//        /* To count number of skipped keys due to NaN pattern
//        int64_t NaNExpMask = (0x7FFL << 52U);
//        uint32_t count = 0;
//        for(int32_t j = 0; j < INT_MAX; j++) {
//            tuple_t tmp;
//            tmp.key = j;
//            tmp.payload = 0;
//
//            // avoid NaN in tuples
//            int64_t * bitpattern = (int64_t *)&(tmp);
//            if(((*bitpattern) & NaNExpMask) == NaNExpMask){
//                count ++;
//            }
//        }
//        fprintf (stderr, "[INFO ] Number of keys skipped = %d\n", count);
//        */
//    }
//
//END_TEST
//
//START_TEST(test_avxsort_int32_ordered)
//    {
//        int64_t sz = rand() % MAXTESTSIZE;
//        int32_t *in = generate_rand_ordered_int32(sz);
//        int32_t *out = calloc(sz, sizeof(int32_t));
//        avxsort_int32(&in, &out, sz);
//        ck_assert_int_eq(is_sorted_int32(out, sz), 1);
//        free(in);
//        free(out);
//    }
//
//END_TEST
//
//START_TEST(test_avxsort_int64_ordered)
//    {
//        int64_t sz = rand() % MAXTESTSIZE;
//        int64_t *in = generate_rand_ordered_int64(sz);
//        int64_t *out = calloc(sz, sizeof(int64_t));
//        avxsort_int64(&in, &out, sz);
//        ck_assert_int_eq(is_sorted_int64(out, sz), 1);
//        free(in);
//        free(out);
//    }
//
//END_TEST
//
//START_TEST(test_avxsort_tuples_ordered)
//    {
//        int64_t sz = rand() % MAXTESTSIZE;
//        tuple_t *in = generate_rand_ordered_tuples(sz);
//        tuple_t *out = malloc(sz * sizeof(tuple_t));;
//        avxsort_tuples(&in, &out, sz);
//        ck_assert_int_eq(is_sorted_tuples(out, sz), 1);
//        free(in);
//        free(out);
//    }
//
//END_TEST
//
//START_TEST(test_avxsortmultiway_int64)
//    {
//        int64_t sz = MAXTESTSIZE + (rand() % (MAXTESTSIZE * 4));
//        int64_t *in = generate_rand_int64(sz);
//        int64_t *out = calloc(sz, sizeof(int64_t));
//        fprintf(stderr, "[INFO ] avxsortmultiway_int64(size=%"PRId64")\n", sz);
//        avxsortmultiway_int64(&in, &out, sz);
//        ck_assert_int_eq(is_sorted_int64(out, sz), 1);
//        free(in);
//        free(out);
//    }
//
//END_TEST
//
//START_TEST(test_avxsortmultiway_tuples)
//    {
//        int64_t sz = MAXTESTSIZE + (rand() % (MAXTESTSIZE * 4));
//        tuple_t *in = generate_rand_tuples(sz);
//        tuple_t *out = calloc(sz, sizeof(tuple_t));
//        fprintf(stderr, "[INFO ] avxsortmultiway_tuples(size=%"PRId64")\n", sz);
//        avxsortmultiway_tuples(&in, &out, sz);
//        ck_assert_int_eq(is_sorted_tuples(out, sz), 1);
//        free(in);
//        free(out);
//    }
//
//END_TEST
//
//Suite
//*
//
//avxsort_suite(void) {
//    Suite *s = suite_create("avxsort");
//
//    int seed = time(NULL);
//    fprintf(stderr, "[INFO ] avxsort_suit seed value is %d\n", seed);
//    srand(seed);
//
//    /* Core test case */
//    TCase *tc_int32 = tcase_create("int32");
//    tcase_add_test(tc_int32, test_avxsort_int32);
//    tcase_add_test(tc_int32, test_avxsort_int32_ordered);
//    tcase_set_timeout(tc_int32, 0);
//
//    TCase *tc_int64 = tcase_create("int64");
//    tcase_add_test(tc_int64, test_avxsort_int64);
//    tcase_add_test(tc_int64, test_avxsort_int64_ordered);
//    tcase_add_test(tc_int64, test_avxsortmultiway_int64);
//    tcase_set_timeout(tc_int64, 0);
//
//    TCase *tc_tuples = tcase_create("tuples");
//    tcase_add_test(tc_tuples, test_avxsort_tuples);
//    tcase_add_test(tc_tuples, test_avxsort_tuples_ordered);
//    tcase_add_test(tc_tuples, test_avxsortmultiway_tuples);
//    tcase_set_timeout(tc_tuples, 0);
//
//    suite_add_tcase(s, tc_int32);
//    suite_add_tcase(s, tc_int64);
//    suite_add_tcase(s, tc_tuples);
//
//    return s;
//}
//
//int
//main(void) {
//    int number_failed;
//    Suite *s = avxsort_suite();
//    SRunner *sr = srunner_create(s);
//    //srunner_set_fork_status (sr, CK_NOFORK);
//    srunner_run_all(sr, CK_NORMAL);
//    number_failed = srunner_ntests_failed(sr);
//    srunner_free(sr);
//    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
//}
//
