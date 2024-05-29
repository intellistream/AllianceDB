/*! \file   LazyIAWJSelOperator.h*/
//
// Created by tony on 22/11/22.
//

#ifndef INTELLISTREAM_INCLUDE_OPERATOR_LazyIAWJSelOPERATOR_H_
#define INTELLISTREAM_INCLUDE_OPERATOR_LazyIAWJSelOPERATOR_H_

#include <Operator/AbstractOperator.h>
#include <Common/Window.h>
#include <atomic>
#include <WaterMarker/LatenessWM.h>

namespace OoOJoin {
/**
 * @class LazyIAWJSelOperator
 * @ingroup ADB_OPERATORS
 * @class LazyIAWJSelOperator Operator/LazyIAWJSelOperator.h
 * @brief The sel-based pecj join under lazy evaluation
 * @note require configurations:
 * - "windowLen" U64: The length of window
 * - "slideLen" U64: The length of slide
 * - "sLen" U64: The length of S buffer
 * - "rLen" U64: The length of R buffer
 * - "algo" String: The specific join algorithm (optional, default nested loop)
 * - "threads" U64: The threads to conduct intra window join (optional, default 1)
 * - "wmTag" String: The tag of watermarker, default is arrival for @ref ArrivalWM
 * - "joinSum" U64, whether or not conduct join sum, default=0
 * @note In current version, the computation will block feeding
 * @note operator tag = "IAWJ"
 */
class LazyIAWJSelOperator : public AbstractOperator {
 protected:
  Window myWindow;
  size_t intermediateResult = 0;
  string algoTag = "NestedLoopJoin";
  uint64_t joinThreads = 1;
  /**
   * @brief if operator is locked by watermark, it will never accept new incoming
   * @todo current implementation is putting rotten, fix later
   */
  atomic_bool lockedByWaterMark = false;
  AbstractWaterMarkerPtr wmGen = nullptr;
  tsType lastArriveTime=0;
  double avgSkewS=0,avgSkewR=0;
  size_t compensatedRu=0;
  uint64_t lazyRunningTime=0;
  void conductComputation();

 public:
  LazyIAWJSelOperator() = default;

  ~LazyIAWJSelOperator() = default;
  /**
   * @brief do the statistics in an lazy way
   */
  void LazyStatistics(void);
  double LazyPredictFutureTuples(double avgSkew,uint64_t arrivedCnt);
  /**
   * @todo Where this operator is conducting join is still putting rotten, try to place it at feedTupleS/R
  * @brief Set the config map related to this operator
  * @param cfg The config map
   * @return bool whether the config is successfully set
  */
  bool setConfig(ConfigMapPtr cfg) override;

  /**
 * @brief feed a tuple s into the Operator
 * @param ts The tuple
  * @warning The current version is simplified and assuming only used in SINGLE THREAD!
  * @return bool, whether tuple is fed.
 */
  bool feedTupleS(TrackTuplePtr ts) override;

  /**
    * @brief feed a tuple R into the Operator
    * @param tr The tuple
    * @warning The current version is simplified and assuming only used in SINGLE THREAD!
    *  @return bool, whether tuple is fed.
    */
  bool feedTupleR(TrackTuplePtr tr) override;

  /**
   * @brief start this operator
   * @return bool, whether start successfully
   */
  bool start() override;

  /**
   * @brief stop this operator
   * @return bool, whether start successfully
   */
  bool stop() override;

  /**
   * @brief get the joined sum result
   * @return The result
   */
  size_t getResult() override;
  /**
   * @brief get the joined  result under AQP
   * @return The result
   */
  size_t getAQPResult() override;
  /**
  * @brief get the throughput under lazy running
  * @param waitTimeUs the waiting time for lazily gather data in us
  * @return the throughput
  */
   double getLazyRunningThroughput() override;
};

/**
 * @ingroup ADB_OPERATORS
 * @typedef LazyIAWJSelOperatorPtr
 * @brief The class to describe a shared pointer to @ref LazyIAWJSelOperator
 */
typedef std::shared_ptr<class LazyIAWJSelOperator> LazyIAWJSelOperatorPtr;
/**
 * @ingroup ADB_OPERATORS
 * @def newLazyIAWJSelOperator
 * @brief (Macro) To creat a new @ref LazyIAWJSelOperator under shared pointer.
 */
#define newLazyIAWJSelOperator std::make_shared<OoOJoin::LazyIAWJSelOperator>
}
#endif //INTELLISTREAM_INCLUDE_OPERATOR_IAWJOPERATOR_H_
