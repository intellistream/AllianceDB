cmake_minimum_required(VERSION 3.5.1)
project(AllianceDB)
set(CMAKE_CXX_STANDARD 11)

find_path(LIBNUMA_INCLUDE numa.h PATHS ${SEARCH_DIRS})
find_library(LIBNUMA_LIB NAMES libnuma.so PATHS ${SEARCH_DIRS})
if(LIBNUMA_INCLUDE-NOTFOUND)
    set (LIBNUMA_INCLUDE "" CACHE PATH "Directory where libnuma header files can be found (consider installing libnuma-dev)")
else()
    include_directories(${LIBNUMA_INCLUDE})
endif()
if(LIBNUMA_LIB-NOTFOUND)
    set (LIBNUMA_LIB "" CACHE PATH "Directory where the libnuma library can be found")
else()
    link_libraries(${LIBNUMA_LIB})
endif()

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

add_executable(sorting
        datagen/generator.h datagen/generator.cpp datagen/genzipf.cpp datagen/genzipf.h
        utils/types.h config.h timer/rdtsc.h
        affinity/cpu_mapping.cpp affinity/cpu_mapping.h
        joins/sortmergejoin_multipass.h joins/sortmergejoin_multiway.h
        joins/sortmergejoin_multiway_skewhandling.h joins/sortmergejoin_mpsm.h
        affinity/memalloc.cpp affinity/memalloc.h utils/params.h
        affinity/numa_shuffle.cpp affinity/numa_shuffle.h joins/joincommon.cpp joins/joincommon.h
        utils/generic_task_queue.cpp utils/generic_task_queue.h utils/barrier.h utils/lock.h
        joins/avxcommon.h joins/merge.cpp joins/merge.h joins/scalar_multiwaymerge.cpp joins/scalar_multiwaymerge.h
        joins/partition.cpp joins/partition.h joins/avxsort.cpp joins/avxsort.h joins/avxintrin_emu.h joins/avxsort_core.h
        joins/scalarsort.cpp joins/scalarsort.h joins/avx_multiwaymerge.cpp joins/avx_multiwaymerge.h joins/avxsort_multiway.cpp
        joins/avxsort_multiway.h joins/sortmergejoin_multiway.cpp
        joins/sortmergejoin_multipass.cpp joins/sortmergejoin_mpsm.cpp joins/sortmergejoin_multiway_skewhandling.cpp
        test/check_avxsort.cpp test/testutil.cpp test/testutil.h params.h timer/t_timer.cpp timer/t_timer.h
        main.cpp utils/tuple_buffer.h)

target_link_libraries(sorting numa)
target_link_libraries(sorting check)
target_link_libraries(sorting rt)
#target_link_libraries(sorting boost_system)