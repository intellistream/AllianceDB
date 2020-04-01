cmake_minimum_required(VERSION 3.5.1)
project(AllianceDB)
set(CMAKE_CXX_STANDARD 11)

# Download and unpack googletest at configure time
#configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
#execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
#        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download"
#        )
#execute_process(COMMAND "${CMAKE_COMMAND}" --build .
#        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download"
#        )

#####ADD NUMA########
find_path(LIBNUMA_INCLUDE numa.h PATHS ${SEARCH_DIRS})
find_library(LIBNUMA_LIB NAMES libnuma.so PATHS ${SEARCH_DIRS})
if (LIBNUMA_INCLUDE-NOTFOUND)
    set(LIBNUMA_INCLUDE "" CACHE PATH "Directory where libnuma header files can be found (consider installing libnuma-dev)")
else ()
    include_directories(${LIBNUMA_INCLUDE})
endif ()
if (LIBNUMA_LIB-NOTFOUND)
    set(LIBNUMA_LIB "" CACHE PATH "Directory where the libnuma library can be found")
else ()
    link_libraries(${LIBNUMA_LIB})
endif ()

#####OTHERS########
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

SET (CMAKE_STATIC_LINKER_FLAGS -lnuma)
#SET (CMAKE_STATIC_LINKER_FLAGS -lboost_system)
SET (CMAKE_STATIC_LINKER_FLAGS -lrt)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${./bin})
SET(CMAKE_CXX_FLAGS "-Wall -Wextra")
SET(CMAKE_CXX_FLAGS_DEBUG "-g")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -msse4.2 -mavx2")

#####ADD GOOGLE TEST########
# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
#set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# Add googletest directly to our build. This adds the following targets:
# gtest, gtest_main, gmock and gmock_main
#enable_testing()
#add_subdirectory("${CMAKE_BINARY_DIR}/googletest-src"
#        "${CMAKE_BINARY_DIR}/googletest-build"
#        )
# The gtest/gmock targets carry header search path dependencies
# automatically when using CMake 2.8.11 or later. Otherwise we
# have to add them here ourselves.
#if(CMAKE_VERSION VERSION_LESS 2.8.11)
#    include_directories("${gtest_SOURCE_DIR}/include"
#            "${gmock_SOURCE_DIR}/include"
#            )
#endif()

ADD_LIBRARY(common OBJECT utils/xxhash64.h timer/t_timer.h timer/t_timer.cpp benchmark.cpp benchmark.h utils/types.h utils/generator.cpp utils/generator.h utils/perf_counters.cpp utils/perf_counters.h utils/cpu_mapping.cpp utils/cpu_mapping.h utils/genzipf.cpp utils/genzipf.h utils/lock.h timer/rdtsc.h utils/barrier.h utils/task_queue.h utils/config.h
        joins/npj_types.h joins/npj_params.h joins/no_partitioning_join.cpp joins/no_partitioning_join.h joins/onlinejoins.cpp joins/onlinejoins.h
        joins/parallel_radix_join.cpp joins/parallel_radix_join.h joins/prj_params.h joins/common_functions.cpp joins/common_functions.h helper/launcher.cpp helper/launcher.h helper/thread_task.cpp helper/thread_task.h helper/localjoiner.cpp helper/localjoiner.h helper/shuffler.cpp helper/shuffler.h helper/fetcher.cpp helper/fetcher.h joins/eagerjoin_struct.h lib/types.h lib/pci.h lib/msr.h lib/msr.cpp lib/cpucounters.cpp lib/cpucounters.h lib/utils.cpp lib/utils.h lib/client_bw.cpp lib/client_bw.h lib/mmio.cpp lib/mmio.h lib/width_extender.h lib/mutex.h lib/exceptions/unsupported_processor_exception.hpp lib/pci.cpp helper/concurrentqueue.h helper/maps/robin_growth_policy.h helper/maps/robin_hash.h helper/maps/robin_map.h helper/maps/robin_set.h helper/readerwriterqueue.h helper/atomicops.h helper/avxsort.cpp helper/avxsort.h helper/avxsort_core.h helper/avxcommon.h utils/params.h helper/avxintrin_emu.h helper/sort_common.cpp helper/sort_common.h
        helper/pmj_helper.cpp helper/pmj_helper.h  utils/skewness.h utils/tuple_buffer.h utils/machine.h)

# Now simply link your own targets against gtest, gmock,
# etc. as appropriate
#add_executable(runUnitTests $<TARGET_OBJECTS:common>)
#target_link_libraries(runUnitTests gtest gtest_main)
#add_test( runUnitTests runUnitTests )

#target_link_libraries(runUnitTests ${CMAKE_DL_LIBS}) ##used by boost
#target_link_libraries(runUnitTests numa)
#target_link_libraries(runUnitTests boost_system)

add_executable(hashing  main.cpp $<TARGET_OBJECTS:common> )

target_link_libraries(hashing ${CMAKE_DL_LIBS}) ##used by boost
target_link_libraries(hashing numa)
#target_link_libraries(hashing boost_system)
