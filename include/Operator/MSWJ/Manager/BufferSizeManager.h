//
// Created by 86183 on 2023/1/5.
//

#ifndef DISORDERHANDLINGSYSTEM_BUFFER_SIZE_MANAGER_H
#define DISORDERHANDLINGSYSTEM_BUFFER_SIZE_MANAGER_H

#include <Operator/MSWJ/Manager/StatisticsManager.h>
#include <Operator/MSWJ/Profiler/TupleProductivityProfiler.h>

namespace MSWJ {
class BufferSizeManager {
 public:
  explicit BufferSizeManager(StatisticsManager *statisticsManager, TupleProductivityProfiler *profiler);

  ~BufferSizeManager() = default;

  // Adaptive K algorithm
  auto kSearch(int stream_id) -> int;

  auto setConfig(INTELLI::ConfigMapPtr config) -> void;

 private:
  // Function γ(L,T) in the paper
  auto y(int K) -> double;

  INTELLI::ConfigMapPtr cfg = nullptr;

  // Data statistics manager
  StatisticsManager *statisticsManager;

  // Tuple productivity profiler
  TupleProductivityProfiler *productivityProfiler;
};
}

#endif //DISORDERHANDLINGSYSTEM_BUFFER_SIZE_MANAGER_H
