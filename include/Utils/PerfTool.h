

#ifndef UTILS_PERFTOOL_H
#define UTILS_PERFTOOL_H
#include <Utils/Logger.hpp>
#define PERF_ERROR(n) ROOFLINE_ERROR(n)

#include <Utils/UtilityFunctions.hpp>

#include <assert.h>
#include <fcntl.h>
#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <linux/perf_event.h>
#include <signal.h>
using namespace std;
namespace ROOFLINE {
#define __LIBPERF_MAX_COUNTERS 32
#define __LIBPERF_ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))
enum perfTrace {
  /* sw tracepoints */
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
};
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
/*class:PerfTool
description: pack the perf in cpp style, remain safe if some std linux perf failed
warnning: confict with the c version libperf

date:202001013
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

/*class:PerfPair
description: perf together with tags

date:202001016
*/
class PerfPair {
 public:
  int ref;
  std::string name;
  uint64_t record;
  PerfPair(int _ref, string _name) {
    ref = _ref;
    name = _name;
    record = 0;
  }
  ~PerfPair() {}
};

/*class:threadPerf
description: The all-in-on perf class for thread

date:202001016
*/
class ThreadPerf {
 protected:
  string getChValueAsString(size_t idx);

  PerfToolPtr myTool;
  vector<PerfPair> pairs;
  struct timeval tstart, tend;
  uint64_t latency;
 public:
  ThreadPerf() {}
  ThreadPerf(int cpu);
  uint64_t getResultById(size_t idx);
  uint64_t getResultByName(string name);
  uint64_t getRunTime(void) {
    return UtilityFunctions::getRunningUs(tstart, tend);
  }
  ~ThreadPerf() {

  }
  //start all registered perf
  void start();
  //end all registered perf
  void end();
  //the headString for printf
  string headStringPrintf(void);
  //for csv
  string headStringCsv(void);
  //for printf
  string resultStringPrintf(void);
  //for csv
  string resultStringCsv(void);

};
}

#endif
