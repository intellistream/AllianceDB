cmake_minimum_required(VERSION 3.9)
project(AllianceDB)
set(CMAKE_CXX_STANDARD 11)

add_executable(AllianceDB main.cpp util/types.h
        affinity/affinity.h util/config.h affinity/cpu_mapping.cpp affinity/cpu_mapping.h
        joins/sortmergejoin_multipass.h joins/sortmergejoin_multiway.h
        joins/sortmergejoin_multiway_skewhandling.h joins/sortmergejoin_mpsm.h
        datagen/generator.h
        util/memalloc.cpp util/memalloc.h util/params.h
        util/numa_shuffle.cpp util/numa_shuffle.h joins/joincommon.cpp joins/joincommon.h
        util/generic_task_queue.cpp util/generic_task_queue.h util/barrier.h)