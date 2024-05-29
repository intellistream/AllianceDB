/*! \file   RawSHJOperator.h
.h*/
//
// Created by tony on 03/12/22.
//

#ifndef INTELLISTREAM_INCLUDE_OPERATOR_MEANAQPRawSHJOperator_H_
#define INTELLISTREAM_INCLUDE_OPERATOR_MEANAQPRawSHJOperator_H_

#include <Operator/MeanAQPIAWJOperator.h>
#include <Common/Window.h>
#include <atomic>
#include <WaterMarker/LatenessWM.h>
#include <Common/StateOfKey.h>

namespace OoOJoin {

/**
 * @ingroup ADB_OPERATORS
 * @class RawSHJOperator Operator/RawSHJOperator.h
 * \image html MeanAQP.png
 * @brief The raw SHJ ported from legacy alliancedb, no OoO support in fact
 * exponential weighted moving average for prediction. This one is EAGER join in fact
 * @note require configurations:
 * - "windowLen" U64: The length of window
 * - "slideLen" U64: The length of slide
 * - "sLen" U64: The length of S buffer
 * - "rLen" U64: The length of R buffer
 * @warning This implementation is putting rotten, just to explore a basic idea of AQP by using historical mean to predict future
 * @warning The predictor and watermarker are currently NOT seperated in this operator, split them in the future!
 * @note In current version, the computation will block feeding
 * @note follows the assumption of linear independent arrival and skewness
 * @note operator tag = "SHJ"
 */
class RawSHJOperator : public MeanAQPIAWJOperator {
 protected:
  void conductComputation();
 public:
  RawSHJOperator() = default;

  ~RawSHJOperator() = default;

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
   * @brief get the joined sum result under AQP
   * @return The result
   */
  size_t getAQPResult() override;

};

/**
 * @ingroup ADB_OPERATORS
 * @typedef RawSHJOperatorPtr
 * @brief The class to describe a shared pointer to @ref RawSHJOperator

 */
typedef std::shared_ptr<class RawSHJOperator> RawSHJOperatorPtr;
/**
 * @ingroup ADB_OPERATORS
 * @def newRawSHJOperator

 * @brief (Macro) To creat a new @ref RawSHJOperator
 under shared pointer.
 */
#define newRawSHJOperator std::make_shared<OoOJoin::RawSHJOperator>

}
#endif //INTELLISTREAM_INCLUDE_OPERATOR_MEANAQPRawSHJOperator_H_
