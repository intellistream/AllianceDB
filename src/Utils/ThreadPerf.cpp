#include <Utils/ThreadPerf.h>
using namespace AllianceDB;
using namespace std;

/* gettid syscall wrapper */
static inline pid_t
myGettid() {
  return syscall(SYS_gettid);
}

/* gettid syscall wrapper */
static inline pid_t
myGetpid() {
  return syscall(SYS_getpid);
}

/* perf_event_open syscall wrapper */
static long
sys_perf_event_open(struct perf_event_attr *hw_event,
                    pid_t pid, int cpu, int group_fd,
                    unsigned long flags) {
  return syscall(__NR_perf_event_open, hw_event, pid, cpu,
                 group_fd, flags);
}
struct perf_event_attr default_attrs[] = {

    {.type = PERF_TYPE_SOFTWARE, .config = PERF_COUNT_SW_CPU_CLOCK}, //1
    {.type = PERF_TYPE_SOFTWARE, .config = PERF_COUNT_SW_TASK_CLOCK}, //2
    {.type = PERF_TYPE_SOFTWARE, .config = PERF_COUNT_SW_CONTEXT_SWITCHES},//3
    {.type = PERF_TYPE_SOFTWARE, .config = PERF_COUNT_SW_CPU_MIGRATIONS},//4
    {.type = PERF_TYPE_SOFTWARE, .config = PERF_COUNT_SW_PAGE_FAULTS},//5
    {.type = PERF_TYPE_SOFTWARE, .config = PERF_COUNT_SW_PAGE_FAULTS_MIN},//6
    {.type = PERF_TYPE_SOFTWARE, .config = PERF_COUNT_SW_PAGE_FAULTS_MAJ},//7
    {.type = PERF_TYPE_HARDWARE, .config = PERF_COUNT_HW_CPU_CYCLES},//8
    {.type = PERF_TYPE_HARDWARE, .config = PERF_COUNT_HW_INSTRUCTIONS},//9
    {.type = PERF_TYPE_HARDWARE, .config = PERF_COUNT_HW_CACHE_REFERENCES},//10
    {.type = PERF_TYPE_HARDWARE, .config = PERF_COUNT_HW_CACHE_MISSES},//11
    {.type = PERF_TYPE_HARDWARE, .config = PERF_COUNT_HW_BRANCH_INSTRUCTIONS},//12
    {.type = PERF_TYPE_HARDWARE, .config = PERF_COUNT_HW_BRANCH_MISSES},//13
    {.type = PERF_TYPE_HARDWARE, .config = PERF_COUNT_HW_BUS_CYCLES},//14
    //15
    {.type = PERF_TYPE_HW_CACHE, .config = (PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_READ << 8)
        | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16))},
    //16
    {.type = PERF_TYPE_HW_CACHE, .config = (PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_READ << 8)
        | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16))},
    //17, no x64
    {.type = PERF_TYPE_HW_CACHE, .config = (PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_WRITE << 8)
        | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16))},
    //18, no x64, no rk3399
    {.type = PERF_TYPE_HW_CACHE, .config = (PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_WRITE << 8)
        | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16))},
    //19, no x64
    {.type = PERF_TYPE_HW_CACHE, .config = (PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_PREFETCH << 8)
        | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16))},
    //20
    {.type = PERF_TYPE_HW_CACHE, .config = (PERF_COUNT_HW_CACHE_L1I | (PERF_COUNT_HW_CACHE_OP_READ << 8)
        | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16))},
    //21, no rk3399
    {.type = PERF_TYPE_HW_CACHE, .config = (PERF_COUNT_HW_CACHE_L1I | (PERF_COUNT_HW_CACHE_OP_READ << 8)
        | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16))},
    //22, no rk3399
    {.type = PERF_TYPE_HW_CACHE, .config = (PERF_COUNT_HW_CACHE_LL | (PERF_COUNT_HW_CACHE_OP_READ << 8)
        | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16))},
    //23, no rk3399
    {.type = PERF_TYPE_HW_CACHE, .config = (PERF_COUNT_HW_CACHE_LL | (PERF_COUNT_HW_CACHE_OP_READ << 8)
        | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16))},
    //24,no rk3399
    {.type = PERF_TYPE_HW_CACHE, .config = (PERF_COUNT_HW_CACHE_LL | (PERF_COUNT_HW_CACHE_OP_WRITE << 8)
        | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16))},
    //25
    {.type = PERF_TYPE_HW_CACHE, .config = (PERF_COUNT_HW_CACHE_LL | (PERF_COUNT_HW_CACHE_OP_WRITE << 8)
        | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16))},
    //26
    {.type = PERF_TYPE_HW_CACHE, .config = (PERF_COUNT_HW_CACHE_DTLB | (PERF_COUNT_HW_CACHE_OP_READ << 8)
        | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16))},
    //27, no rk3399
    {.type = PERF_TYPE_HW_CACHE, .config = (PERF_COUNT_HW_CACHE_DTLB | (PERF_COUNT_HW_CACHE_OP_READ << 8)
        | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16))},
    //28
    {.type = PERF_TYPE_HW_CACHE, .config = (PERF_COUNT_HW_CACHE_DTLB | (PERF_COUNT_HW_CACHE_OP_WRITE << 8)
        | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16))},
    //29,no rk3399
    {.type = PERF_TYPE_HW_CACHE, .config = (PERF_COUNT_HW_CACHE_DTLB | (PERF_COUNT_HW_CACHE_OP_WRITE << 8)
        | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16))},
    //30
    {.type = PERF_TYPE_HW_CACHE, .config = (PERF_COUNT_HW_CACHE_ITLB | (PERF_COUNT_HW_CACHE_OP_READ << 8)
        | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16))},
    //31
    {.type = PERF_TYPE_HW_CACHE, .config = (PERF_COUNT_HW_CACHE_ITLB | (PERF_COUNT_HW_CACHE_OP_READ << 8)
        | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16))},
    //32
    {.type = PERF_TYPE_HW_CACHE, .config = (PERF_COUNT_HW_CACHE_BPU | (PERF_COUNT_HW_CACHE_OP_READ << 8)
        | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16))},
    /* { .type = PERF_TYPE_HW_CACHE, .config = (PERF_COUNT_HW_CACHE_BPU | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16))}, */

};
PerfTool::PerfTool(pid_t pid, int cpu) {
  if (pid == -1) { pid = gettid(); }
  myPid = pid;
  myCpu = cpu;
  int nr_counters = __LIBPERF_ARRAY_SIZE(default_attrs);
  //printf("%d\r\n",nr_counters);
  for (int i = 0; i < nr_counters; i++) {
    PerfEntry entry;
    //  memcpy(&entry.attr, &default_attrs[i], sizeof(struct perf_event_attr));
    default_attrs[i].size = sizeof(struct perf_event_attr);
    /* entry.attr.inherit = 1;
     entry.attr.disabled = 1;
     entry.attr.enable_on_exec = 0;
     entry.attr.exclusive=1;
     entry.attr.pinned=1;*/

    /* entry.attr.sample_period = 100;
      entry.attr.sample_type = PERF_SAMPLE_IP;*/
    entry.fds = sys_perf_event_open(&default_attrs[i], pid, cpu, -1, 0);
    if (entry.fds < 0) {
      /* char str [64];
       sprintf(str,"perf event %d in %d is invalid",i,nr_counters);
        ROOFLINE_WARNING(str);*/
      entry.addressable = false;
    } else {
      entry.addressable = true;
      /* fcntl( entry.fds, F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
       fcntl( entry.fds, F_SETSIG, SIGIO);
       fcntl( entry.fds, F_SETOWN, pid);*/


      ioctl(entry.fds, PERF_EVENT_IOC_DISABLE);
      ioctl(entry.fds, PERF_EVENT_IOC_RESET);
      /*  int ru=read(entry.fds, &value, sizeof(uint64_t));
      printf("ru=%d,value=%ld\r\n",ru,value);*/
      // ioctl( entry.fds,PERF_EVENT_IOC_ENABLE );
      //close(entries[i].fds);
    }

    // uint64_t value;
    entries.push_back(entry);
    /*  int ru=read(entries[i].fds, &value, sizeof(uint64_t));
     printf("i=%d,ru=%d,value=%ld\r\n",i,ru,value);*/

  }
}

PerfTool::~PerfTool() {
  for (size_t i = 0; i < entries.size(); i++) {
    if (entries[i].addressable == true) {
      close(entries[i].fds);
      //printf("close perf %d\r\n",i);
    }
  }
}

uint64_t PerfTool::readPerf(size_t ch) {
  if (ch > entries.size()) {
    return 0;
  }
  if (entries[ch].addressable == false) {
    return 0;
  }
  uint64_t value;
  //ioctl(entries[ch].fds,PERF_EVENT_IOC_REFRESH);
  // ioctl(entries[ch].fds,PERF_EVENT_IOC_DISABLE );
  int ru = read(entries[ch].fds, &value, sizeof(uint64_t));
  // printf("mid ru=%d\r\n",ru);
  if (ru < 0) {
    PERF_ERROR("invalid read");
  }
  return value;
}

int PerfTool::startPerf(size_t ch) {
  /*  if(ch>entries.size())
   {
       return -1;
   }
   if(entries[ch].addressable==false)
   {
       return -1;
   }*/
  /*ioctl(entries[ch].fds,PERF_EVENT_IOC_DISABLE );
    ioctl(entries[ch].fds,PERF_EVENT_IOC_RESET);*/
  // read(entries[ch].fds, &prevValue, sizeof(uint64_t));
  ioctl(entries[ch].fds, PERF_EVENT_IOC_ENABLE);

  return 1;
}

int PerfTool::stopPerf(size_t ch) {
  if (ch > entries.size()) {
    return -1;
  }
  if (entries[ch].addressable == false) {
    return -1;
  }
  ioctl(entries[ch].fds, PERF_EVENT_IOC_DISABLE);
  ioctl(entries[ch].fds, PERF_EVENT_IOC_RESET);
  return 1;
}
bool PerfTool::isValidChannel(size_t ch) {
  if (ch > entries.size()) {
    return false;
  }
  return entries[ch].addressable;
}

//thread perfs
ThreadPerf::ThreadPerf(int cpu) {
  //PerfTool myTool;

  myTool = std::make_shared<PerfTool>(0, cpu);
  //add entries we would like to perf
  //pairs.push_back(PerfPair(COUNT_HW_CPU_CYCLES,"cpuCycle"));
  //pairs.push_back(PerfPair(COUNT_HW_INSTRUCTIONS,"instructions"));
  pairs.push_back(PerfPair(COUNT_HW_CACHE_REFERENCES, "cacheRefs"));
  pairs.push_back(PerfPair(COUNT_HW_CACHE_MISSES, "cacheMiss"));
  //pairs.push_back(PerfPair(COUNT_HW_BRANCH_INSTRUCTIONS,"branches"));
  //pairs.push_back(PerfPair(COUNT_HW_BRANCH_MISSES,"branchMiss"));
  //ll

  //pairs.push_back(PerfPair(COUNT_HW_CACHE_MISSES,"cacheMiss"));

  //pairs.push_back(PerfPair(COUNT_HW_CACHE_L1I_LOADS,"L1ILoad"));
  // pairs.push_back(PerfPair(COUNT_HW_CACHE_L1I_LOADS_MISSES,"L1ILoadMiss"));
//BPU
  /*  pairs.push_back(PerfPair(COUNT_HW_CACHE_BPU_LOADS,"BPULoad"));
   pairs.push_back(PerfPair(COUNT_HW_CACHE_BPU_LOADS_MISSES,"BPUMiss"));*/
  pairs.push_back(PerfPair(COUNT_HW_CPU_CYCLES, "Cycles"));
  pairs.push_back(PerfPair(COUNT_HW_INSTRUCTIONS, "instructions"));
  //  pairs.push_back(PerfPair(COUNT_HW_BUS_CYCLES,"BusCycles"));

}
ThreadPerf::ThreadPerf(int cpu, vector<PerfPair> customPair) {
  myTool = std::make_shared<PerfTool>(0, cpu);
  pairs = customPair;
}
void ThreadPerf::start() {
  for (size_t i = 0; i < pairs.size(); i++) {

    // pairs[i].record=myTool->readPerf(pairs[i].ref);
    myTool->startPerf(pairs[i].ref);
// printf("%d\r\n",pairs[i].ref);
  }
  gettimeofday(&ts, NULL);
}

void ThreadPerf::end() {
  gettimeofday(&te, NULL);
  for (size_t i = 0; i < pairs.size(); i++) {
    pairs[i].record = myTool->readPerf(pairs[i].ref);
    myTool->stopPerf(pairs[i].ref);
  }
}
string ThreadPerf::getChValueAsString(size_t idx) {
  string ru = "NA";
  size_t ch = pairs[idx].ref;
  if (myTool->isValidChannel(ch) == false) {
    return ru;
  }
  uint64_t val = pairs[idx].record;
  /* val=myTool->readPerf(pairs[idx].ref);
    myTool->stopPerf(pairs[idx].ref);*/
  // printf("%ld\r\n",val);

  ru = to_string(val);
  return ru;
}

uint64_t ThreadPerf::getResultById(int idx) {
  if (idx == -1) {
    return getRunTime();
  }
  for (size_t i = 0; i < pairs.size(); i++) {
    if (pairs[i].ref == idx) {
      return pairs[i].record;
    }

  }
  return 0;
}

uint64_t ThreadPerf::getResultByName(string name) {
  if (name == "time") {
    return getRunTime();
  }
  for (size_t i = 0; i < pairs.size(); i++) {
    if (pairs[i].name == name) {
      return pairs[i].record;
    }

  }
  return 0;
}
string ThreadPerf::headStringPrintf() {
  string ru = "";
  for (size_t i = 0; i < pairs.size(); i++) {
    ru += "\t|";
    ru += pairs[i].name;

  }
  ru += "\t|";
  ru += "time";
  ru += "\r\n";
  return ru;
}

string ThreadPerf::headStringCsv() {
  string ru = "";
  for (size_t i = 0; i < pairs.size(); i++) {
    ru += ",";
    ru += pairs[i].name;

  }
  ru += ",";
  ru += "time";
  ru += "\r\n";
  return ru;
}
string ThreadPerf::resultStringPrintf() {
  string ru = "";
  for (size_t i = 0; i < pairs.size(); i++) {
    ru += "\t|";
    ru += getChValueAsString(i);
  }
  ru += "\t|";
  ru += to_string(getRunTime());
  ru += "\r\n";
  return ru;
}

string ThreadPerf::resultStringCsv() {
  string ru = "";
  for (size_t i = 0; i < pairs.size(); i++) {
    ru += ",";
    ru += getChValueAsString(i);

  }
  ru += ",";
  ru += to_string(getRunTime());
  ru += "\r\n";
  return ru;
}