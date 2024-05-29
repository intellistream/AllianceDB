//
// Created by 86183 on 2023/5/15.
//

#ifndef INTELLISTREAM_IAWJSELOPERATOR_H
#define INTELLISTREAM_IAWJSELOPERATOR_H

#include <Operator/MeanAQPIAWJOperator.h>
#include <Common/Window.h>
#include <atomic>
#include <WaterMarker/LatenessWM.h>
#include <Common/StateOfKey.h>

namespace OoOJoin {

/**
 * @ingroup ADB_OPERATORS
 * @class IMAIAWJSelOperator Operator/IMAIAWJSelOperator.h
 * @brief The IAWJ operator under the simplest AQP strategy, but works under coarse-graind sel mode
 * exponential weighted moving average for prediction. This one is EAGER join in fact
 * @note require configurations:
 * - "windowLen" U64: The length of window
 * - "slideLen" U64: The length of slide
 * - "sLen" U64: The length of S buffer
 * - "rLen" U64: The length of R buffer
 * - "wmTag" String: The tag of watermarker, default is arrival for @ref ArrivalWM
 * @warning This implementation is putting rotten, just to explore a basic idea of AQP by using historical mean to predict future
 * @warning The predictor and watermarker are currently NOT seperated in this operator, split them in the future!
 * @note In current version, the computation will block feeding
 * @note follows the assumption of linear independent arrival and skewness
 * @note operator tag = "IAWJSel"
 */
class IAWJSelOperator : public MeanAQPIAWJOperator {
 protected:
  void conductComputation();

  class IMAStateOfKey : public MeanStateOfKey {
   public:
    double lastUnarrivedTuples = 0;

    IMAStateOfKey() = default;

    ~IMAStateOfKey() = default;
  };

#define newIMAStateOfKey std::make_shared<IMAStateOfKey>
  using IMAStateOfKeyPtr = std::shared_ptr<IMAStateOfKey>;
  /**
   * @brief trace then number of total s and r
   */
  IMAStateOfKeyPtr noSTrace, noRTrace;
  double selObservation = 0.0, selPrediction = 0.0, selPrev = 0.0;
  double noSObservation, noRObservation;
  AEWMAPredictor selPredictor;
 private:
  double selectivityR{1};
  double selectivityS{1};

  double noR{0};
  double noS{0};

 public:
  IAWJSelOperator() = default;

  ~IAWJSelOperator() = default;

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
 * @typedef IAWJSelOperatorPtr
 * @brief The class to describe a shared pointer to @ref IAWJSelOperator

 */
typedef std::shared_ptr<class IAWJSelOperator> IAWJSelOperatorPtr;
/**
 * @ingroup ADB_OPERATORS
 * @def newIAWJSelOperator

 * @brief (Macro) To creat a new @ref IAWJSelOperator
 under shared pointer.
 */
#define newIAWJSelOperator std::make_shared<OoOJoin::IAWJSelOperator>

}

#endif //INTELLISTREAM_IAWJSELOPERATOR_H
