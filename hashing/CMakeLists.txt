cmake_minimum_required(VERSION 3.5.1)
project(AllianceDB)
set(CMAKE_CXX_STANDARD 11)

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
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${./bin})
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -o3 -pthread -msse4.1")

add_executable(hashing main.cpp
        utils/xxhash64.h utils/timer.cpp utils/timer.h utils/types.h utils/generator.cpp utils/generator.h utils/perf_counters.cpp utils/perf_counters.h utils/cpu_mapping.cpp utils/cpu_mapping.h utils/genzipf.cpp utils/genzipf.h utils/lock.h utils/rdtsc.h utils/barrier.h utils/task_queue.h utils/config.h
        joins/npj_types.h joins/npj_params.h joins/no_partitioning_join.cpp joins/no_partitioning_join.h joins/shj.cpp joins/shj.h
        joins/parallel_radix_join.cpp joins/parallel_radix_join.h joins/prj_params.h joins/common_functions.cpp joins/common_functions.h utils/xxhash64.h)
target_link_libraries(hashing numa)

