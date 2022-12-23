# Copyright (C) 2021 by the INTELLI team (https://github.com/intellistream)
cmake_minimum_required(VERSION 3.14)
project(AllianceDB CXX)
# Judge the os system

#gcc options
if (DEFINED ENV{CC})
    message(STATUS "you have set CC= " $ENV{CC})
    set(CMAKE_C_COMPILER $ENV{CC})
endif ()
if (DEFINED ENV{CXX})
    message(STATUS "you have set CXX= " $ENV{CXX})
    set(CMAKE_CXX_COMPILER $ENV{CXX})
endif ()

# Custom CMake find instructions and macros
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")
include(cmake/macros.cmake)
include(cmake/default.cmake)

message(STATUS ${CMAKE_CXX_COMPILER})
# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set Optimization Flags
# -Werror=return-type -Werror=unused-variable -Werror=unused-parameter
set(CMAKE_CXX_FLAGS "-std=c++20 -lstdc++fs -Wall -g")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DNO_RACE_CHECK -DAllianceDB_DEBUG_MODE=1")
set(CMAKE_CXX_FLAGS_RELEASE "-Wno-ignored-qualifiers -Wno-sign-compare -O3")
# Set LOGGING_LEVEL Flag
if (AllianceDB_LOGGING_LEVEL)
    get_log_level_value(AllianceDB_LOGGING_VALUE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DAllianceDB_LOGGING_LEVEL=${AllianceDB_LOGGING_VALUE}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DAllianceDB_LOGGING_LEVEL=${AllianceDB_LOGGING_VALUE}")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DAllianceDB_LOGGING_LEVEL=${AllianceDB_LOGGING_VALUE}")
else (AllianceDB_LOGGING_LEVEL)
    message("---Everything will be logged")
endif (AllianceDB_LOGGING_LEVEL)

message(STATUS "CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE}")

option(ENABLE_UNIT_TESTS "Enable unit tests" ON)
message(STATUS "Enable testing: ${ENABLE_UNIT_TESTS}")

if (ENABLE_UNIT_TESTS)
    enable_testing()
    # Google Test
    include(FetchContent)

    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG release-1.11.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    include(GoogleTest)
endif ()

# Print all used include directories
message(STATUS "INCLUDE_DIRS:")
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach (dir ${dirs})
    message(STATUS "  - ${dir}")
endforeach ()

message(STATUS " Libraries: ${LIBRARIES}")

# Add Source Code
add_subdirectory(src)
add_subdirectory(third_party/gflags)

# Add Library
get_sources(AllianceDB_SOURCE_FILES)
get_headers(AllianceDB_HEADER_FILES)
add_library(AllianceDB SHARED ${AllianceDB_SOURCE_FILES} ${AllianceDB_HEADER_FILES} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(AllianceDB PUBLIC "include")
target_include_directories(AllianceDB PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(AllianceDB PUBLIC ${LIBRARIES} -lpthread)

#Add benchmarks with command
#Main Entry.
add_subdirectory(benchmark)

#Add tests (based on GTest suits)
if (ENABLE_UNIT_TESTS)
    add_subdirectory(test)
endif ()
install(DIRECTORY "include" DESTINATION "/AllianceDB" COMPONENT AllianceDB)

