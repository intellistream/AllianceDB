//
// Created by 86183 on 2023/1/8.
//

#ifndef DISORDERHANDLINGSYSTEM_STREAM_OPERATOR_H
#define DISORDERHANDLINGSYSTEM_STREAM_OPERATOR_H

#include <queue>
#include <unordered_map>
#include <list>
#include <mutex>
#include <Operator/MSWJ/Profiler/TupleProductivityProfiler.h>
#include <Operator/MSWJ/Common/MSWJDefine.h>
#include <Common/Window.h>
#include <WaterMarker/AbstractWaterMarker.h>
#include <WaterMarker/LatenessWM.h>
#include <Common/StateOfKey.h>
#include <Operator/MeanAQPIAWJOperator.h>
#include <JoinAlgos/NPJ/MultiThreadHashTable.h>

using namespace OoOJoin;

namespace MSWJ {
class StreamOperator : public MeanAQPIAWJOperator {
 public:
  // Whether r tuple is in window
  bool rIsInWindow = false;
  // Whether s tuple is in window
  bool sIsInWindow = false;

  explicit StreamOperator(TupleProductivityProfiler *profiler, INTELLI::ConfigMapPtr config);

  ~StreamOperator() = default;

  // The main execution function of MSWJ
  auto mswjExecution(const TrackTuplePtr &tuple) -> bool;

  auto setConfig(INTELLI::ConfigMapPtr config) -> bool;

  // Get the value of discrete random variable D. If delay(ei) ∈(kg,(k+1)g], Di=k+1
  static auto inline get_D(int delay) -> int {
    return delay % g == 0 ? delay / g : delay / g + 1;
  }

  /**
   * @brief Start this operator
   * @return bool, whether start successfully
   */
  bool start() override;

  /**
   * @brief Stop this operator
   * @return bool, whether stop successfully
   */
  bool stop() override;

  /**
   * @brief Get the joined sum result
   * @return The result
   */
  size_t getResult() override;

  /**
   * @brief Get the joined sum result under AQP
   * @return The result
   */
  size_t getAQPResult() override;

  /**
   * @brief Get the joinSum result
   * @return The result
   */
  size_t getAQPJoinSumResult(keyType key);

  /**
   * @brief Get the joinSum result
   * @return The result
   */
  size_t getConfirmJoinSumResult(keyType key);

  class MSWJStateOfKey : public MeanStateOfKey {
   public:
    // The number of arrived tuples
    // double arrivalSkew = 0, sigmaArrivalSkew = 0;
    // The pointer to the last event tuple
    // The pointer to the last arrival tuple
    // The last seen time
    // The last estimate all tuples
    double lastUnarrivedTuples = 0;
    valueType value = 0;

    // The last added number
    // The default constructor of IMAStateOfKey
    MSWJStateOfKey() = default;

    // The default destructor of IMAStateOfKey
    ~MSWJStateOfKey() = default;
  };

  // The shared pointer to IMAStateOfKey
  typedef std::shared_ptr<MSWJStateOfKey> MSWJStateOfKeyPtr;
  // Define newIMAStateOfKey using std::make_shared
#define newMSWJStateOfKey std::make_shared<MSWJStateOfKey>

 private:
  // The tag of MSWJ algorithm
  string algoTag = " NestedLoopJoin";
  //be used to calculate joinsum
  bool joinSum{};
  //The flag of MSWJ compensation
  uint64_t mswjCompensation = 0;
  // The pointer to the configuration map
  INTELLI::ConfigMapPtr config;
  // The pointer to the tuple productivity profiler
  TupleProductivityProfiler *productivityProfiler;
  //record key-join count and value, format: key,  {{ joinCount, value}, valueCount}
  std::unordered_map<keyType, std::pair<std::pair<uint64_t, uint64_t>, uint64_t>> joinAQPCountMap{};
  //record key-join count and value, format: key,  {{ joinCount, value}, valueCount}
  std::unordered_map<keyType, std::pair<std::pair<uint64_t, uint64_t>, uint64_t>> joinConfirmCountMap{};
};
}

#endif //DISORDERHANDLINGSYSTEM_STREAM_OPERATOR_H
