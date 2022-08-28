/*! \file ThreadPerf.h*/
//Copyright (C) 2022 by the IntelliStream team (https://github.com/intellistream)
#ifndef UTILS_ThreadPerf_H
#define UTILS_ThreadPerf_H
#define PERF_ERROR(n) printf(n)
#include <sys/syscall.h>
#include <linux/perf_event.h>
#include <assert.h>
#include <fcntl.h>
#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if __linux
#include <sys/syscall.h>
#include <linux/perf_event.h>
#elif defined(_WIN32) || defined(_WIN64)
#include <windows.h>       // Or something like it.
#endif
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <string>
#include <memory>
#include <vector>
using namespace std;
namespace AllianceDB {
#define __LIBPERF_MAX_COUNTERS 32
#define __LIBPERF_ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

/*class:PerfEntry
description:the entry record in PerfTool
warnning: confict with the c version libperf

date:202001013
*/
class PerfEntry {
 public:
  //struct perf_event_attr attr;
  int fds;
  bool addressable;
  uint64_t prevVale;
  PerfEntry() { addressable = false; }
  ~PerfEntry() {}
};
/**
 * @class PerfTool Utils/ThreadPerf.h
 * @brief pack the perf event in cpp style, remain safe if some std linux perf failed
 * @warning conflict with the c version libperf
*/
class PerfTool {
 private:
  /* data */
  std::vector<PerfEntry> entries;
  pid_t myPid;
  int myCpu;
  uint64_t prevValue;
 public:
  PerfTool() {

  }
  PerfTool(pid_t pid, int cpu);
  // reading result from a perf trace on [ch], will return 0 if the channel is invaild
  uint64_t readPerf(size_t ch);
  // start the perf trace on [ch]
  int startPerf(size_t ch);
  // st the perf trace on [ch]
  int stopPerf(size_t ch);
  //check the addressability of [ch]
  bool isValidChannel(size_t ch);
  ~PerfTool();
};
typedef std::shared_ptr<PerfTool> PerfToolPtr;

/**
 * @class PerfPair Utils/ThreadPerf.h
 * @brief Combine @ref PerfTool together with tags
*/
class PerfPair {
 public:
  /** The number to find pair*/
  int ref;
  /** The string to find pair*/
  std::string name;
  /** The read result*/
  uint64_t record;
  /**
   * @brief The construction function
   * @param _ref number index, one in @ref perfTrace
   * @param _name string index
   */
  PerfPair(int _ref, string _name) {
    ref = _ref;
    name = _name;
    record = 0;
  }
  ~PerfPair() {}
};

/**
 * @defgroup INTELLI_UTIL Shared Utils with other Intelli Stream programs
 * @{
 * This group provides common functions to support the Intelli Stream programs.
 */
/**
 * @defgroup INTELLI_UTIL_ThreadPerf The ThreadPerf tool
 * @{
 * Use this tool to perf your thread in an easy way. Organized as @ref INTELLI_UTIL_ThreadPerf_Init and @ref NTELLI_UTIL_ThreadPerf_Results.
 * The following is an example to use
 * @verbatim
 ThreadPerf tp(-1, {PerfPair(COUNT_HW_CPU_CYCLES, "Cycles"), \
                    PerfPair(COUNT_HW_INSTRUCTIONS, "instructions")
                }
                    ); // creat a ThreadPerf with specific events
 tp.start(); //start it
 // run your program here
  tp.end();
  cout<<tp.headStringPrintf()<<endl;
  cout<<tp.resultStringPrintf()<<endl;
  //print the results
 @endverbatim
 */
/**
 * @ingroup INTELLI_UTIL_ThreadPerf
* @class ThreadPerf Utils/ThreadPerf.h
* @brief The all-in-on perf class for thread
 *  @note You should enable perf in linux kernel and have root right to access it. For events not supported, the tool will mark an "NA" in string
*/
class ThreadPerf {
 protected:
  string getChValueAsString(size_t idx);
  uint64_t getRunTime(void) {
    // gettimeofday(&te, NULL);
    int64_t s0, e0, s1, e1;
    s0 = ts.tv_sec;
    s1 = ts.tv_usec;
    e0 = te.tv_sec;
    e1 = te.tv_usec;
    return 1000000 * (e0 - s0) + (e1 - s1);
  }
  PerfToolPtr myTool;
  vector<PerfPair> pairs;
  struct timeval ts, te;
  uint64_t latency;
 public:

  ThreadPerf() {}
  /**
   * @defgroup INTELLI_UTIL_ThreadPerf_Init Initialize
   * @{
   * To init and register the perf events
   * */
  /**
  *
  * @brief To start perf this thread at specific cpu, with default perf events
  * @param cpu The cpu to be perfed, -1 for all that runs this thread
  */
  ThreadPerf(int cpu);
  /**
   * @brief To start perf this thread at specific cpu, with customized perf events
   * @param cpu The cpu to be perfed, -1 for all that runs this thread
   * @param customPair The customized vector of PerfPair to define perf events
   * @see @ref PerfPair
   */
  ThreadPerf(int cpu, vector<PerfPair> customPair);
  /**
    * @brief start all registered perf
    * @note The previous results will NOT be cleared after this function
   */
  void start();
  /**
   * @}
   */
  /**
   * @defgroup NTELLI_UTIL_ThreadPerf_Results Get the perf results
   * @{
   * We can get thr perf result by 1) search their index and 2) search their tags,
   * The csv and printing sheet are also supported
   */
  /**
   * @brief Get the perf result by event id
   * @param idx The event id
   * @return perf result
   */
  uint64_t getResultById(int idx);
  /**
   * @brief Get the perf result by event tag string
   * @param name The name tag
   * @return perf result
   */
  uint64_t getResultByName(string name);

  ~ThreadPerf() {

  }

  /**
    * @brief end all registered perf
    * @note The results will be valid after this function
   */
  void end();
  //the headString for printf
  /**
   * @brief get the head string for print a result sheet
   * @return the head string
   */
  string headStringPrintf(void);
  /**
   * @brief get the head string for csv file of a result sheet
   * @return the head string
   */
  string headStringCsv(void);
  /**
  * @brief get the result string for print a result sheet
  * @return the result string
  */
  string resultStringPrintf(void);
  /**
  * @brief get the result string for csv file a result sheet
  * @return the result string
  */
  string resultStringCsv(void);
  /**
   * @}
   */
};
/**
 * @enum perfTrace Utils/ThreadPerf.h
 * @brief The valid idx for perf
 */
typedef
enum perfTrace {
  /* sw tracepoints */
  TRACE_RUNNING_TIME = -1,
  COUNT_SW_CPU_CLOCK = 0,
  COUNT_SW_TASK_CLOCK = 1,
  COUNT_SW_CONTEXT_SWITCHES = 2,
  COUNT_SW_CPU_MIGRATIONS = 3,
  COUNT_SW_PAGE_FAULTS = 4,
  COUNT_SW_PAGE_FAULTS_MIN = 5,
  COUNT_SW_PAGE_FAULTS_MAJ = 6,

  /* hw counters */
  COUNT_HW_CPU_CYCLES = 7,
  COUNT_HW_INSTRUCTIONS = 8,
  COUNT_HW_CACHE_REFERENCES = 9,
  COUNT_HW_CACHE_MISSES = 10,
  COUNT_HW_BRANCH_INSTRUCTIONS = 11,
  COUNT_HW_BRANCH_MISSES = 12,
  COUNT_HW_BUS_CYCLES = 13,

  /* cache counters */

  /* L1D - data cache */
  COUNT_HW_CACHE_L1D_LOADS = 14,
  COUNT_HW_CACHE_L1D_LOADS_MISSES = 15,
  COUNT_HW_CACHE_L1D_STORES = 16,
  COUNT_HW_CACHE_L1D_STORES_MISSES = 17,
  COUNT_HW_CACHE_L1D_PREFETCHES = 18,

  /* L1I - instruction cache */
  COUNT_HW_CACHE_L1I_LOADS = 19,
  COUNT_HW_CACHE_L1I_LOADS_MISSES = 20,

  /* LL - last level cache */
  COUNT_HW_CACHE_LL_LOADS = 21,
  COUNT_HW_CACHE_LL_LOADS_MISSES = 22,
  COUNT_HW_CACHE_LL_STORES = 23,
  COUNT_HW_CACHE_LL_STORES_MISSES = 24,

  /* DTLB - data translation lookaside buffer */
  COUNT_HW_CACHE_DTLB_LOADS = 25,
  COUNT_HW_CACHE_DTLB_LOADS_MISSES = 26,
  COUNT_HW_CACHE_DTLB_STORES = 27,
  COUNT_HW_CACHE_DTLB_STORES_MISSES = 28,

  /* ITLB - instructiont translation lookaside buffer */
  COUNT_HW_CACHE_ITLB_LOADS = 29,
  COUNT_HW_CACHE_ITLB_LOADS_MISSES = 30,

  /* BPU - branch prediction unit */
  COUNT_HW_CACHE_BPU_LOADS = 31,
  COUNT_HW_CACHE_BPU_LOADS_MISSES = 32,

  /* Special internally defined "counter" */
  /* this is the _only_ floating point value */
  //LIB_SW_WALL_TIME = 33
} perfIdx;
}
/**
 * @}
 * @}
 */
#endif
