#ifndef INTELLISTREAM_INCLUDE_OPERATOR_MSWJOPERATOR_H_
#define INTELLISTREAM_INCLUDE_OPERATOR_MSWJOPERATOR_H_

#include <Operator/AbstractOperator.h>
#include <Common/Window.h>
#include <atomic>
#include <utility>
#include <WaterMarker/LatenessWM.h>
#include <Operator/MSWJ/KSlack/KSlack.h>
#include <Operator/MeanAQPIAWJOperator.h>
#include <WaterMarker/LatenessWM.h>
#include <Common/StateOfKey.h>

namespace OoOJoin {
/**
* @ingroup ADB_OPERATORS
* @typedef MSWJOperatorPtr
* @brief The class to describe a shared pointer to @ref MSWJOperator
*/

typedef std::shared_ptr<class MSWJOperator> MSWJOperatorPtr;
typedef std::shared_ptr<class MSWJ::KSlack> KSlackPtr;
typedef std::shared_ptr<class MSWJ::BufferSizeManager> BufferSizeManagerPtr;
typedef std::shared_ptr<class MSWJ::StatisticsManager> StatisticsManagerPtr;
typedef std::shared_ptr<class MSWJ::TupleProductivityProfiler> TupleProductivityProfilerPtr;
typedef std::shared_ptr<class MSWJ::Synchronizer> SynchronizerPtr;
typedef std::shared_ptr<class MSWJ::StreamOperator> StreamOperatorPtr;

/**
 * @class MSWJOperator
 * @ingroup ADB_OPERATORS
 * @class MSWJOperator Operator/MSWJOperator.h
 * @brief The intra window join (MSWJ) operator, only considers a single window, following yuanzhen's work ICDE2016
 * @note require configurations:
 * - "windowLen" U64: The length of window
 * - "slideLen" U64: The length of slide
 * - "sLen" U64: The length of S buffer,
 * - "rLen" U64: The length of R buffer
 * - "algo" String: The specific join algorithm (optional, default nested loop)
 * - "threads" U64: The threads to conduct intra window join (optional, default 1)
 * - "wmTag" String: The tag of watermarker, default is arrival for @ref ArrivalWM
 * = "mswjCompensation" U64, whether or not use linear compensation in mswj, default 0
 * @note In current version, the computation will block feeding
 * @note operator tag = "MSWJ"
 */
class MSWJOperator : public MeanAQPIAWJOperator {
 protected:

  class IMAStateOfKey : public MeanStateOfKey {
   public:
    // size_t arrivedTupleCnt = 0;
    //  double arrivalSkew = 0, sigmaArrivalSkew = 0;
    // TrackTuplePtr lastEventTuple = nullptr, lastArrivalTuple = nullptr;
    // tsType  lastSeenTime=0;
    //size_t lastEstimateAllTuples=0;
    double lastUnarrivedTuples = 0;

    // size_t lastAdded=0;
    IMAStateOfKey() = default;

    ~IMAStateOfKey() = default;
  };

  typedef std::shared_ptr<IMAStateOfKey> IMAStateOfKeyPtr;
#define newIMAStateOfKey std::make_shared<IMAStateOfKey>

 private:
  bool created = false;

  //save rTuple record
  std::vector<TrackTuplePtr> rTupleRecord;

  KSlackPtr kSlackS = nullptr;
  KSlackPtr kSlackR = nullptr;

  //bufferSizeManager,  is used to update K(bufferSize)
  BufferSizeManagerPtr bufferSizeManager;

  //statisticsManager, is used to statistic
  StatisticsManagerPtr statisticsManager;

  //TupleProductivityProfiler,is used to get productivity of tuple at join stage.
  TupleProductivityProfilerPtr tupleProductivityProfiler;

  SynchronizerPtr synchronizer;

  StreamOperatorPtr streamOperator = nullptr;

 public:

  MSWJOperator() = default;

  MSWJOperator(BufferSizeManagerPtr bufferSizeManager, TupleProductivityProfilerPtr tupleProductivityProfiler,
               SynchronizerPtr synchronizer, StreamOperatorPtr streamOperator,
               StatisticsManagerPtr statisticsManager, KSlackPtr kSlackR, KSlackPtr kSlackS) : bufferSizeManager(
      std::move(bufferSizeManager)),
                                                                                               statisticsManager(
                                                                                                   std::move(
                                                                                                       statisticsManager)),
                                                                                               tupleProductivityProfiler(
                                                                                                   std::move(
                                                                                                       tupleProductivityProfiler)),
                                                                                               synchronizer(
                                                                                                   std::move(
                                                                                                       synchronizer)),
                                                                                               streamOperator(
                                                                                                   std::move(
                                                                                                       streamOperator)),
                                                                                               kSlackR(std::move(
                                                                                                   kSlackR)),
                                                                                               kSlackS(std::move(
                                                                                                   kSlackS)) {}

  ~MSWJOperator() = default;

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
* @def newMSWJOperator
* @brief (Macro) To creat a new @ref MSWJOperator under shared pointer.
*/
#define newMSWJOperator std::make_shared<OoOJoin::MSWJOperator>

}
#endif //INTELLISTREAM_INCLUDE_OPERATOR_IAWJOPERATOR_H_
