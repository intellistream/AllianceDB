/*! \file   MeanAQPIAWJOperator.h*/
//
// Created by tony on 03/12/22.
//

#ifndef INTELLISTREAM_INCLUDE_OPERATOR_MEANAQPMeanAQPIAWJOPERATOR_H_
#define INTELLISTREAM_INCLUDE_OPERATOR_MEANAQPMeanAQPIAWJOPERATOR_H_

#include <Operator/AbstractOperator.h>
#include <Common/Window.h>
#include <atomic>
#include <WaterMarker/LatenessWM.h>
#include <Common/StateOfKey.h>

namespace OoOJoin {

/**
 * @ingroup ADB_OPERATORS
 * @class MeanAQPIAWJOperator Operator/MeanAQPIAWJOperator.h
 * \image html MeanAQP.png
 * @brief The intra window join (MeanAQPIAWJ) operator under the simplest AQP strategy, only considers a single window and uses
 * exponential weighted moving average for prediction
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
 * @note operator tag = "MeanAQP"
 */
class MeanAQPIAWJOperator : public AbstractOperator {
 protected:
  //predicted param
  static constexpr double MIN_ALPHA = 0.01;
  static constexpr double MAX_ALPHA = 0.99;
  static constexpr double MIN_MAX_RATIO = 0.1;
  static constexpr double MAX_MAX_RATIO = 100.0;

  double max_ratio = 10.0;
  double alpha = 0.5;

  Window myWindow;
  size_t intermediateResult = 0;
  size_t confirmedResult = 0;
  size_t confirmedResultJS = 0;
  uint64_t windowBound = 0;
  // double alphaArrivalRate=0.125;
  double alphaArrivalSkew = 0.125;
  double betaArrivalSkew = 0.25;
//  tsType lastTimeS = 0, lastTimeR = 0;
  double aqpScale = 0.1;

  void conductComputation();

  atomic_bool lockedByWaterMark = false;
  AbstractWaterMarkerPtr wmGen = nullptr;
  StateOfKeyHashTablePtr stateOfKeyTableR, stateOfKeyTableS;
  tsType lastTimeOfR = 0;
  /**
   * @brief for time breakdown of searching index
   */
  tsType timeBreakDownIndex{};
  /**
   * @brief for time breakdown of prediction
   */
  tsType timeBreakDownPrediction{};
  /**
   * @brief for time breakdown of join
   */
  tsType timeBreakDownJoin{};
  /**
   * @brief adaptive linear filter for prediction
   */
  class AEWMAPredictor {
   protected:
    double alpha = 0.2;
    double minAlpha = 0.01;        // Minimum allowed alpha
    double maxAlpha = 0.99;        // Maximum allowed alpha
    double previousValue;   // Previous observed value
   public:
    AEWMAPredictor() = default;
    ~AEWMAPredictor() = default;
    void reset(void) {
      alpha = 0.2;
      previousValue = 0.0;
    }
    double update(double observedValue) {
      double smoothedValue = alpha * observedValue + (1 - alpha) * previousValue;
      previousValue = observedValue;

      // Adjust alpha based on the deviation between observed and smoothed values
      double deviation = std::abs(observedValue - smoothedValue);
      if (deviation > 0) {
        double adjustment = 1.0 / deviation;
        alpha = std::max(minAlpha, std::min(maxAlpha, alpha * adjustment));
      }

      return smoothedValue;
    }

  };
  class MeanStateOfKey : public AbstractStateOfKey {
   public:
    size_t arrivedTupleCnt = 0;
    double arrivalSkew = 0, sigmaArrivalSkew = 0;
    TrackTuplePtr lastEventTuple = nullptr, lastArrivalTuple = nullptr;
    AEWMAPredictor joinedRValuePredictor;
    double joinedRValueSum = 0, joinedRValueAvg = 0;
    int64_t joinedRValueCnt = 0;
    double rvAvgPrediction = 0;
    // tsType  lastSeenTime=0;
    MeanStateOfKey() {
      joinedRValuePredictor.reset();
    }

    ~MeanStateOfKey() = default;
  };

  typedef std::shared_ptr<MeanStateOfKey> MeanStateOfKeyPtr;
#define newMeanStateOfKey std::make_shared<MeanStateOfKey>

  void updateStateOfKey(MeanStateOfKeyPtr sk, TrackTuplePtr tp);

  // void updateStateOfKeyR(MeanStateOfKeyPtr sk,TrackTuplePtr tp);
  void lazyComputeOfAQP();

  double predictUnarrivedTuples(MeanStateOfKeyPtr px);

 public:

  MeanAQPIAWJOperator() = default;

  ~MeanAQPIAWJOperator() = default;

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

  /**
 * @brief get the break down information of processing time
 * @warning should check the nullptr of output
 * @return The ConfigMapPtr which contains breakdown information, null if no breakdown supported
  * @note provided breakdown:
  * - "index" U64, processing time on prediction for indexing the tuple state in hhash table
  * - "prediction" U64, processing time on prediction
  * - "join" U64, processing time on prediction
  * The total processing time=index+prediction+join
 */
  ConfigMapPtr getTimeBreakDown() override;
};

/**
 * @ingroup ADB_OPERATORS
 * @typedef MeanAQPIAWJOperatorPtr
 * @brief The class to describe a shared pointer to @ref MeanAQPIAWJOperator
 */
typedef std::shared_ptr<class MeanAQPIAWJOperator> MeanAQPIAWJOperatorPtr;
/**
 * @ingroup ADB_OPERATORS
 * @def newMeanAQPIAWJOperator
 * @brief (Macro) To creat a new @ref MeanAQPIAWJOperator under shared pointer.
 */
#define newMeanAQPIAWJOperator std::make_shared<OoOJoin::MeanAQPIAWJOperator>
}
#endif //INTELLISTREAM_INCLUDE_OPERATOR_MEANAQPMeanAQPIAWJOPERATOR_H_
