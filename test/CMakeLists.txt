# adding the Google_Tests_run target
add_executable(Google_Tests_run
        SystemTest/SimpleTest.cpp
        )

# linking Google_Tests_run with IntelliStream_lib which will be tested
target_link_libraries(Google_Tests_run gtest_main AllianceDB)

if (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/datasets)
add_custom_target(test_datasets ALL
            COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/datasets ${CMAKE_CURRENT_BINARY_DIR}/datasets)
add_dependencies(Google_Tests_run test_datasets)
endif (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/datasets)

gtest_discover_tests(Google_Tests_run)