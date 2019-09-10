cmake_minimum_required(VERSION 3.9)
project(AllianceDB)
set(CMAKE_CXX_STANDARD 11)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_executable(AllianceDB
        datagen/generator.h datagen/generator.cpp datagen/genzipf.cpp datagen/genzipf.h
        util/types.h util/config.h util/rdtsc.h
        affinity/affinity.h affinity/cpu_mapping.cpp affinity/cpu_mapping.h
        joins/sortmergejoin_multipass.h joins/sortmergejoin_multiway.h
        joins/sortmergejoin_multiway_skewhandling.h joins/sortmergejoin_mpsm.h
        affinity/memalloc.cpp affinity/memalloc.h util/params.h
        affinity/numa_shuffle.cpp affinity/numa_shuffle.h joins/joincommon.cpp joins/joincommon.h
        util/generic_task_queue.cpp util/generic_task_queue.h util/barrier.h util/lock.h
        joins/avxcommon.h  joins/merge.cpp joins/merge.h joins/scalar_multiwaymerge.cpp joins/scalar_multiwaymerge.h
        joins/partition.cpp joins/partition.h joins/avxsort.cpp joins/avxsort.h  util/avxintrin_emu.h joins/avxsort_core.h joins/scalarsort.cpp joins/scalarsort.h joins/avx_multiwaymerge.cpp joins/avx_multiwaymerge.h joins/avxsort_multiway.cpp joins/avxsort_multiway.h  joins/sortmergejoin_multiway.cpp
        main.cpp
        joins/sortmergejoin_multipass.cpp joins/sortmergejoin_mpsm.cpp joins/sortmergejoin_multiway_skewhandling.cpp)