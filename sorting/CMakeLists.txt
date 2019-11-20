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
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${./bin})
set (CMAKE_STATIC_LINKER_FLAGS -lnuma)
set (CMAKE_STATIC_LINKER_FLAGS -lrt)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2 -mavx")
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_executable(sorting
        datagen/generator.h datagen/generator.cpp datagen/genzipf.cpp datagen/genzipf.h
        util/types.h config.h util/rdtsc.h
        affinity/cpu_mapping.cpp affinity/cpu_mapping.h
        joins/sortmergejoin_multipass.h joins/sortmergejoin_multiway.h
        joins/sortmergejoin_multiway_skewhandling.h joins/sortmergejoin_mpsm.h
        affinity/memalloc.cpp affinity/memalloc.h util/params.h
        affinity/numa_shuffle.cpp affinity/numa_shuffle.h joins/joincommon.cpp joins/joincommon.h
        util/generic_task_queue.cpp util/generic_task_queue.h util/barrier.h util/lock.h
        joins/avxcommon.h joins/merge.cpp joins/merge.h joins/scalar_multiwaymerge.cpp joins/scalar_multiwaymerge.h
        joins/partition.cpp joins/partition.h joins/avxsort.cpp joins/avxsort.h joins/avxintrin_emu.h joins/avxsort_core.h
        joins/scalarsort.cpp joins/scalarsort.h joins/avx_multiwaymerge.cpp joins/avx_multiwaymerge.h joins/avxsort_multiway.cpp
        joins/avxsort_multiway.h joins/sortmergejoin_multiway.cpp
        joins/sortmergejoin_multipass.cpp joins/sortmergejoin_mpsm.cpp joins/sortmergejoin_multiway_skewhandling.cpp
        test/check_avxsort.cpp test/testutil.cpp test/testutil.h params.h
        main.cpp)

target_link_libraries(sorting numa)
target_link_libraries(sorting check)
target_link_libraries(sorting rt)