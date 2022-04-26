// Copyright (C) 2021 by the IntelliStream team (https://github.com/intellistream)

#include <Utils/UtilityFunctions.hpp>
#include <numeric>
#include <Common/Result.hpp>
static unsigned long mt[N]; /* the array for the state vector  */
static int mti; /* mti==N+1 means mt[N] is not initialized */

long INTELLI::UtilityFunctions::genrand_int31() {
  return long(genrand_int32() >> 1);
}

unsigned long INTELLI::UtilityFunctions::genrand_int32() {
  unsigned long y;
  static unsigned long mag01[2] = {0x0UL, MATRIX_A};
  /* mag01[x] = x * MATRIX_A  for x=0,1 */

  if (mti >= N) { /* generate N words at one time */
    int kk;

    if (mti == N + 1)   /* if init_genrand() has not been called, */
      init_genrand(5489UL); /* a default initial seed is used */

    for (kk = 0; kk < N - M; kk++) {
      y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
      mt[kk] = mt[kk + M] ^ (y >> 1) ^ mag01[y & 0x1UL];
    }
    for (; kk < N - 1; kk++) {
      y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
      mt[kk] = mt[kk + (M - N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
    }
    y = (mt[N - 1] & UPPER_MASK) | (mt[0] & LOWER_MASK);
    mt[N - 1] = mt[M - 1] ^ (y >> 1) ^ mag01[y & 0x1UL];

    mti = 0;
  }

  y = mt[mti++];

  /* Tempering */
  y ^= (y >> 11);
  y ^= (y << 7) & 0x9d2c5680UL;
  y ^= (y << 15) & 0xefc60000UL;
  y ^= (y >> 18);

  return y;
}

INTELLI::UtilityFunctions::UtilityFunctions() {
  mti = N + 1; /* mti==N+1 means mt[N] is not initialized */
}

/**
 * initializes mt[N] with a seed
 * @param s
 */
void INTELLI::UtilityFunctions::init_genrand(unsigned long s) {
  /* initializes mt[N] with a seed */
  mt[0] = s & 0xffffffffUL;
  for (mti = 1; mti < N; mti++) {
    mt[mti] =
        (1812433253UL * (mt[mti - 1] ^ (mt[mti - 1] >> 30)) + mti);
    /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
    /* In the previous versions, MSBs of the seed affect   */
    /* only MSBs of the array mt[].                        */
    /* 2002/01/09 modified by Makoto Matsumoto             */
    mt[mti] &= 0xffffffffUL;
    /* for >32 bit machines */
  }
}

/**
 *  generates a random number on (0,1)-real-interval
 * @return
 */
double INTELLI::UtilityFunctions::genrand_real3() {
  return (((double) genrand_int32()) + 0.5) * (1.0 / 4294967296.0);
  /* divided by 2^32 */
}

std::shared_ptr<std::barrier<>> INTELLI::UtilityFunctions::createBarrier(int count) {
  return std::make_shared<std::barrier<>>(count);
}

void INTELLI::UtilityFunctions::timerStart(Result &result) {
  //result.timeTaken = clock();
  gettimeofday(&result.timeBegin, NULL);
}

void INTELLI::UtilityFunctions::timerEnd(Result &result) {
  // double start = result.timeTaken;
  result.timeTaken = timeLastUs(result.timeBegin);
  result.timeTaken /= 1000.0;
}

void INTELLI::UtilityFunctions::printTest(char const *name, int context) {
  std::cout << name << ":" << context << "\n";
}
int INTELLI::UtilityFunctions::bind2Core(int id) {
  if (id == -1) //OS scheduling
  {
    return -1;
  }
  int maxCpu = std::thread::hardware_concurrency();
  int cpuId = id % maxCpu;
  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(cpuId, &mask);
  return cpuId;
}

vector<size_t> INTELLI::UtilityFunctions::avgPartitionSizeFinal(size_t inS, std::vector<size_t> partitionWeight) {
  size_t partitions = partitionWeight.size();
  vector<size_t> partitionSizeFinals = vector<size_t>(partitions);
  size_t divideLen = inS / partitions;
  size_t tEnd = 0;

  for (size_t i = 0; i < partitions - 1; i++) {
    tEnd += divideLen;
    partitionSizeFinals[i] = divideLen;
  }
  partitionSizeFinals[partitions - 1] = inS - tEnd;
  return partitionSizeFinals;
}
vector<size_t> INTELLI::UtilityFunctions::weightedPartitionSizeFinal(size_t inS, std::vector<size_t> partitionWeight) {
  vector<size_t> partitionSizes;
  vector<size_t> partitionSizeFinals;
  size_t fraction = accumulate(partitionWeight.begin(), partitionWeight.end(), 0);
  size_t tsize = 0;
  for (size_t i = 0; i < partitionWeight.size() - 1; i++) {
    tsize = inS * partitionWeight[i] / fraction;
    partitionSizes.push_back(tsize);
  }

  //check if the partition is vaild
  size_t tEnd = 0;
  for (size_t i = 0; i < partitionSizes.size() - 1; i++) {
    if (partitionSizes[i] != 0) {
      tEnd += partitionSizes[i];
      partitionSizeFinals.push_back(partitionSizes[i]);
    }
  }
  partitionSizeFinals[partitionSizes.size() - 1] = inS - tEnd;
  return partitionSizeFinals;
}
size_t INTELLI::UtilityFunctions::timeLast(size_t past, size_t unitTime) {
  size_t te = clock();
  return (te - past) * unitTime / CLOCKS_PER_SEC;
}

size_t INTELLI::UtilityFunctions::timeLastUs(struct timeval ts) {
  struct timeval te;
  gettimeofday(&te, NULL);
  int64_t s0, e0, s1, e1;
  s0 = ts.tv_sec;
  s1 = ts.tv_usec;
  e0 = te.tv_sec;
  e1 = te.tv_usec;
  return 1000000 * (e0 - s0) + (e1 - s1);
}
