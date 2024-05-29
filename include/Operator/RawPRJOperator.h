/*! \file   RawPRJOperator.h*/
//
// Created by tony on 22/11/22.
//

#ifndef INTELLISTREAM_INCLUDE_OPERATOR_RawPRJOPERATOR_H_
#define INTELLISTREAM_INCLUDE_OPERATOR_RawPRJOPERATOR_H_

#include <Operator/AbstractOperator.h>
#include <Common/Window.h>
#include <atomic>
#include <WaterMarker/LatenessWM.h>

namespace OoOJoin {
/**
 * @class RawPRJOperator
 * @ingroup ADB_OPERATORS
 * @class RawPRJOperator Operator/RawPRJOperator.h
 * @brief The raw PRJ operator, without OoO mechanism
 * @note require configurations:
 * - "windowLen" U64: The length of window
 * - "slideLen" U64: The length of slide
 * - "sLen" U64: The length of S buffer
 * - "rLen" U64: The length of R buffer
 * - "algo" String: The specific join algorithm (optional, default nested loop)
 * - "joinSum" U64, whether or not conduct join sum, default=0
 * @warning This is from leagacy version of alliancedb, no ooo support in fact!!!
 * @note In current version, the computation will block feeding
 * @note operator tag = "NPJ"
 */
class RawPRJOperator : public AbstractOperator {
 protected:
  Window myWindow;
  size_t intermediateResult = 0;
  string algoTag = "NestedLoopJoin";
  uint64_t joinThreads = 1;
  /**
   * @brief if operator is locked by watermark, it will never accept new incoming
   * @note For this one, there is no really water mark
   */
  atomic_bool lockedByWaterMark = false;
  AbstractWaterMarkerPtr wmGen = nullptr;
  uint64_t lazyRunningTime=0;
  void conductComputation();

 public:
  RawPRJOperator() = default;

  ~RawPRJOperator() = default;

  /**
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
    * @brief get the throughput under lazy running
    * @param waitTimeUs the waiting time for lazily gather data in us
    * @return the throughput
    */
   double getLazyRunningThroughput() override;
};

/**
 * @ingroup ADB_OPERATORS
 * @typedef RawPRJOperatorPtr
 * @brief The class to describe a shared pointer to @ref RawPRJOperator
 */
typedef std::shared_ptr<class RawPRJOperator> RawPRJOperatorPtr;
/**
 * @ingroup ADB_OPERATORS
 * @def newRawPRJOperator
 * @brief (Macro) To creat a new @ref RawPRJOperator under shared pointer.
 */
#define newRawPRJOperator std::make_shared<OoOJoin::RawPRJOperator>
}
#endif //INTELLISTREAM_INCLUDE_OPERATOR_IAWJOPERATOR_H_
