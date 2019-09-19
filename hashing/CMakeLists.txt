cmake_minimum_required(VERSION 3.9)
project(AllianceDB)
set(CMAKE_CXX_STANDARD 11)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${./bin})
set (CMAKE_STATIC_LINKER_FLAGS -lnuma)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -msse4.1")

add_executable(hashing
        main.cpp no_partitioning_join.cpp no_partitioning_join.h types.h parallel_radix_join.cpp parallel_radix_join.h generator.cpp generator.h perf_counters.cpp perf_counters.h affinity.h cpu_mapping.cpp cpu_mapping.h genzipf.cpp genzipf.h lock.h  npj_types.h npj_params.h rdtsc.h barrier.h prj_params.h task_queue.h config.h)

target_link_libraries(hashing numa)

