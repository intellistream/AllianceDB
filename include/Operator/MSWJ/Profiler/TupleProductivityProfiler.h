//
// Created by 86183 on 2023/1/9.
//

#ifndef DISORDERHANDLINGSYSTEM_TUPLE_PRODUCTIVITY_PROFILER_H
#define DISORDERHANDLINGSYSTEM_TUPLE_PRODUCTIVITY_PROFILER_H

#include <unordered_map>
#include <map>
#include <mutex>
#include <Utils/ConfigMap.hpp>
#include <Operator/MSWJ/Common/MSWJDefine.h>

namespace MSWJ {

class TupleProductivityProfiler {
 public:

  // Constructor
  explicit TupleProductivityProfiler(INTELLI::ConfigMapPtr config);

  // Destructor
  ~TupleProductivityProfiler() = default;

  // Get join_record_map
  auto getJoinRecordMap() -> std::vector<int>;

  // Add join record
  auto addJoinRecord(int stream_id, int count) -> void;

  // Update cross-join result size
  auto updateCrossJoin(int Di, size_t res) -> void;

  // Update join result size
  auto updateJoinRes(int Di, size_t res) -> void;

  // Get select ratio
  auto getSelectRatio(int K) -> double;

  // Get requirement recall
  auto getRequirementRecall() -> double;

  // Set configuration
  auto setConfig(INTELLI::ConfigMapPtr config) -> void;

  // Get the value of the discrete random variable D
  // If delay(ei) ∈(kg,(k+1)g], then Di=k+1
  static auto inline getD(int delay) -> int {
    return delay % g == 0 ? delay / g : delay / g + 1;
  }

 private:

  INTELLI::ConfigMapPtr cfg{};

  // Records the number of tuples that arrive at the join operator
  std::vector<int> joinRecordMap{};

  // Records the size of the cross-join result
  std::vector<size_t> crossJoinMap{0};
  std::vector<int> crossJoinPos{};

  // Records the size of the join result
  std::vector<size_t> joinResultMap{0};
  std::vector<int> joinResultPos{};

};

}

#endif //DISORDERHANDLINGSYSTEM_TUPLE_PRODUCTIVITY_PROFILER_H
