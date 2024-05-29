/*! \file   IAWJOperator.h*/
//
// Created by tony on 22/11/22.
//

#ifndef INTELLISTREAM_INCLUDE_OPERATOR_IAWJOPERATOR_H_
#define INTELLISTREAM_INCLUDE_OPERATOR_IAWJOPERATOR_H_

#include <Operator/AbstractOperator.h>
#include <Common/Window.h>
#include <atomic>
#include <WaterMarker/LatenessWM.h>

namespace OoOJoin {
/**
 * @class IAWJOperator
 * @ingroup ADB_OPERATORS
 * @class IAWJOperator Operator/IAWJOperator.h
 * @brief The intra window join (IAWJ) operator, only considers a single window
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
class IAWJOperator : public AbstractOperator {
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

  void conductComputation();

 public:
  IAWJOperator() = default;

  ~IAWJOperator() = default;

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

};

/**
 * @ingroup ADB_OPERATORS
 * @typedef IAWJOperatorPtr
 * @brief The class to describe a shared pointer to @ref IAWJOperator
 */
typedef std::shared_ptr<class IAWJOperator> IAWJOperatorPtr;
/**
 * @ingroup ADB_OPERATORS
 * @def newIAWJOperator
 * @brief (Macro) To creat a new @ref IAWJOperator under shared pointer.
 */
#define newIAWJOperator std::make_shared<OoOJoin::IAWJOperator>
}
#endif //INTELLISTREAM_INCLUDE_OPERATOR_IAWJOPERATOR_H_
