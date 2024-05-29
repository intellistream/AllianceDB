// Copyright (C) 2021 by the IntelliStream team (https://github.com/intellistream)

#include <Utils/UtilityFunctions.hpp>
#include <iostream>
#include <numeric>
#include <chrono>
using namespace std;

std::shared_ptr<std::barrier<>> INTELLI::UtilityFunctions::createBarrier(int count) {
  return std::make_shared<std::barrier<>>(count);
}

/*
void INTELLI::UtilityFunctions::timerStart(Result &result) {
  //result.timeTaken = clock();
  gettimeofday(&result.timeBegin, NULL);
}

void INTELLI::UtilityFunctions::timerEnd(Result &result) {
  // double start = result.timeTaken;
  result.timeTaken = timeLastUs(result.timeBegin);
  result.timeTaken /= 1000.0;
}*/
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
  /**
   * @brief fixed some core bind bugs
   */
  if (sched_setaffinity(0, sizeof(cpu_set_t), &mask) < 0) {
    /*AT_ERROR("Error: setaffinity()\n");
    exit(0);*/
  }
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
