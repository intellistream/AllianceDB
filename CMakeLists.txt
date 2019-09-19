cmake_minimum_required(VERSION 3.9)
project(AllianceDB)
set(CMAKE_CXX_STANDARD 11)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_executable(AllianceDB
        sort-based/datagen/generator.h sort-based/datagen/generator.cpp sort-based/datagen/genzipf.cpp sort-based/datagen/genzipf.h
        sort-based/util/types.h sort-based/util/config.h sort-based/util/rdtsc.h
        sort-based/affinity/affinity.h sort-based/affinity/cpu_mapping.cpp sort-based/affinity/cpu_mapping.h
        sort-based/joins/sortmergejoin_multipass.h sort-based/joins/sortmergejoin_multiway.h
        sort-based/joins/sortmergejoin_multiway_skewhandling.h sort-based/joins/sortmergejoin_mpsm.h
        sort-based/affinity/memalloc.cpp sort-based/affinity/memalloc.h sort-based/util/params.h
        sort-based/affinity/numa_shuffle.cpp sort-based/affinity/numa_shuffle.h sort-based/joins/joincommon.cpp sort-based/joins/joincommon.h
        sort-based/util/generic_task_queue.cpp sort-based/util/generic_task_queue.h sort-based/util/barrier.h sort-based/util/lock.h
        sort-based/joins/avxcommon.h sort-based/joins/merge.cpp sort-based/joins/merge.h sort-based/joins/scalar_multiwaymerge.cpp sort-based/joins/scalar_multiwaymerge.h
        sort-based/joins/partition.cpp sort-based/joins/partition.h sort-based/joins/avxsort.cpp sort-based/joins/avxsort.h sort-based/joins/avxintrin_emu.h sort-based/joins/avxsort_core.h sort-based/joins/scalarsort.cpp sort-based/joins/scalarsort.h sort-based/joins/avx_multiwaymerge.cpp sort-based/joins/avx_multiwaymerge.h sort-based/joins/avxsort_multiway.cpp sort-based/joins/avxsort_multiway.h sort-based/joins/sortmergejoin_multiway.cpp
        sort-based/main.cpp
        sort-based/joins/sortmergejoin_multipass.cpp sort-based/joins/sortmergejoin_mpsm.cpp sort-based/joins/sortmergejoin_multiway_skewhandling.cpp)