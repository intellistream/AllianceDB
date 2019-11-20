cmake_minimum_required(VERSION 3.13.4)
project(AllianceDB)
set(CMAKE_CXX_STANDARD 17)

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

add_executable(hashing
        utils/xxhash64.h utils/t_timer.h utils/t_timer.cpp utils/types.h utils/generator.cpp utils/generator.h utils/perf_counters.cpp utils/perf_counters.h utils/cpu_mapping.cpp utils/cpu_mapping.h utils/genzipf.cpp utils/genzipf.h utils/lock.h utils/rdtsc.h utils/barrier.h utils/task_queue.h utils/config.h
        joins/npj_types.h joins/npj_params.h joins/no_partitioning_join.cpp joins/no_partitioning_join.h joins/onlinejoins.cpp joins/onlinejoins.h
        joins/parallel_radix_join.cpp joins/parallel_radix_join.h joins/prj_params.h joins/common_functions.cpp joins/common_functions.h helper/launcher.cpp helper/launcher.h helper/thread_task.cpp helper/thread_task.h helper/localjoiner.cpp helper/localjoiner.h helper/shuffler.cpp helper/shuffler.h helper/fetcher.cpp helper/fetcher.h joins/shj_struct.h lib/types.h lib/pci.h lib/msr.h lib/msr.cpp lib/cpucounters.cpp lib/cpucounters.h lib/utils.cpp lib/utils.h lib/client_bw.cpp lib/client_bw.h lib/mmio.cpp lib/mmio.h lib/width_extender.h lib/mutex.h lib/exceptions/unsupported_processor_exception.hpp lib/pci.cpp helper/concurrentqueue.h helper/maps/robin_growth_policy.h helper/maps/robin_hash.h helper/maps/robin_map.h helper/maps/robin_set.h helper/readerwriterqueue.h helper/atomicops.h helper/avxsort.cpp helper/avxsort.h helper/avxsort_core.h helper/avxcommon.h utils/params.h helper/avxintrin_emu.h helper/sort_common.cpp helper/sort_common.h
        main.cpp)

target_link_libraries(hashing ${CMAKE_DL_LIBS}) ##used by boost

target_link_libraries(hashing numa)

