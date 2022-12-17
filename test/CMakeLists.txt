# adding the Google_Tests_run target
add_executable(google_test
        System/Test.cpp
        )

# linking google_test with IntelliStream_lib which will be tested
target_link_libraries(google_test gtest_main AllianceDB)

if (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/datasets)
add_custom_target(test_datasets ALL
            COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/datasets ${CMAKE_CURRENT_BINARY_DIR}/datasets)
add_dependencies(google_test test_datasets)
endif (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/datasets)

gtest_discover_tests(google_test)