cmake_minimum_required(VERSION 3.9)
project(AllianceDB)
set(CMAKE_CXX_STANDARD 11)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_executable(AllianceDB
        datagen/generator.h datagen/generator.cpp datagen/genzipf.cpp datagen/genzipf.h
        util/types.h util/config.h
        affinity/affinity.h affinity/cpu_mapping.cpp affinity/cpu_mapping.h
        joins/sortmergejoin_multipass.h joins/sortmergejoin_multiway.h
        joins/sortmergejoin_multiway_skewhandling.h joins/sortmergejoin_mpsm.h
        util/memalloc.cpp util/memalloc.h util/params.h
        util/numa_shuffle.cpp util/numa_shuffle.h joins/joincommon.cpp joins/joincommon.h
        util/generic_task_queue.cpp util/generic_task_queue.h util/barrier.h util/lock.h
        main.cpp
        )