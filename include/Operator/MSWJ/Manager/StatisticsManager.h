//
// Created by 86183 on 2023/1/6.
//

#ifndef DISORDERHANDLINGSYSTEM_STATISTICS_MANAGER_H
#define DISORDERHANDLINGSYSTEM_STATISTICS_MANAGER_H

#include <vector>
#include <unordered_map>
#include <mutex>
#include <Operator/MSWJ/Common/MSWJDefine.h>
#include <Operator/MSWJ/Profiler/TupleProductivityProfiler.h>
#include <Utils/ConfigMap.hpp>
#include <Common/Tuples.h>

using namespace OoOJoin;
namespace MSWJ {

class StatisticsManager {
 public:

  explicit StatisticsManager(TupleProductivityProfiler *profiler, INTELLI::ConfigMapPtr config);

  ~StatisticsManager() = default;

  // Get the maximum delay of tuples in a stream
  auto getMaxD(int streamId) -> int;

  // Probability density function fDiK for discrete random variable Dik, where Dik represents the coarse-grained delay of a tuple from the corresponding stream accepted by the join operator under k setting
  auto fDk(int d, int streamId, int K) -> double;

  // Estimate the size of the sliding window for stream l
  auto wil(int l, int streamId, int K) -> int;

  // Add a tuple record to the corresponding stream's recordMap
  auto addRecord(int streamId, const TrackTuplePtr &tuple) -> void;

  // Add a T and K record to the corresponding stream's tMap and kMap
  auto addRecord(int streamId, int T, int K) -> void;

  auto setConfig(INTELLI::ConfigMapPtr config) -> void;

  // Get the value of discrete random variable Di, where delay(ei) ∈(kg,(k+1)g]，then Di=k+1
  auto inline getD(int delay) -> int {
    return delay % g == 0 ? delay / g : delay / g + 1;
  }

 private:
  // Get the value of Ksync for a stream
  auto getKsync(int streamId) -> int;

  // Get the average value of Ksync for a stream
  auto getAvgKsync(int streamId) -> int;

  // Estimate the future value of Ksync for a stream
  auto getFutureKsync(int stream_id) -> int;

  // Apply the adaptive window method described in reference [25], with the main parameters yet to be determined
  auto getRStat(int streamId) -> int;

  // Probability density function fDi for discrete random variable Di
  auto fD(int d, int stream_id) -> double;

  INTELLI::ConfigMapPtr cfg{};

  // Size of Rstat window
  std::vector<int> RStatMap{0};

  // Map recording the input history of stream Si
  std::vector<std::vector<TrackTuple>> recordMap{};

  // Map recording the T values of stream Si
  std::vector<int> tMap{0};

  // Map recording the K values of stream Si
  std::vector<int> kMap{0};

  // Map recording all K_sync values, for sampling and predicting future ksync
  std::vector<std::vector<int>> kSyncMap{};

  // Histogram map
  std::vector<std::vector<double>> histogramMap{};
  std::vector<std::vector<int>> histogramPos{};

  // Tuple productivity profiler
  TupleProductivityProfiler *productivityProfiler;
};

}

#endif //DISORDERHANDLINGSYSTEM_STATISTICS_MANAGER_H
