add_test(SystemTest.SimpleTest /home/tony/Projects/AllianceDB/build/test/Google_Tests_run [==[--gtest_filter=SystemTest.SimpleTest]==] --gtest_also_run_disabled_tests)
set_tests_properties(SystemTest.SimpleTest PROPERTIES WORKING_DIRECTORY /home/tony/Projects/AllianceDB/build/test SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==])
set( Google_Tests_run_TESTS SystemTest.SimpleTest)
