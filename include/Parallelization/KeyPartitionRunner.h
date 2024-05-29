/*! \file  KeyPartitionRunner.h*/

//
// Created by tony on 27/06/23.
//

#ifndef INTELLISTREAM_INCLUDE_PARALLELIZATION_KEYPARTITIONRUNNER_H_
#define INTELLISTREAM_INCLUDE_PARALLELIZATION_KEYPARTITIONRUNNER_H_
#include <Operator/OperatorTable.h>
#include <Utils/AbstractC20Thread.hpp>
#include <Utils/ConfigMap.hpp>
#include <vector>
#include <memory>
namespace OoOJoin {
/**
 * @class KeyPartitionWorker Parallelization/KeyPartitionRunner.h
 * @ingroup Parallelization
 * @brief the working thread class of key partition parallelization
 */
class KeyPartitionWorker : public INTELLI::AbstractC20Thread {
 protected:
  /**
   * @brief the time struct of the whole system
   */
  struct timeval tSystem;
  virtual void inlineMain();
  bool isLazy=false;
  uint64_t windowLen=0;
  /**
   * @brief the inline main function of decentralized mode, assume there is no central distributor of datastream
   */
  virtual void decentralizedMain();

  /**
   * @brief the inline main function of centralized mode, assume there is a central distributor of datastream
   */
  void centralizedMain();
  uint64_t myId = 0;
  uint64_t workers = 1;
  /**
   * @brief the config map of this worker
   */
  INTELLI::ConfigMapPtr cfg;
  AbstractOperatorPtr testOp{};
  virtual bool isMySTuple(TrackTuplePtr ts);
  virtual bool isMyRTuple(TrackTuplePtr tr);
 public:
  /**
  * @brief the rTuple for decentralized main
  */
  std::vector<TrackTuplePtr> rTuple{};
  /**
  * @brief the sTuple for decentralized main
  */
  std::vector<TrackTuplePtr> sTuple{};
  KeyPartitionWorker() {}
  ~KeyPartitionWorker() {}
  /**
   * @brief set the config map
   * @param _cfg the config map
   */
  void setConfig(INTELLI::ConfigMapPtr _cfg);
  void prepareRunning(void);
  /**
   * @brief to set the id of this worker
   * @param tid the id of this worker
   * @param wks the number of totoal workers
   */
  void setId(uint64_t tid, uint64_t wks) {
    myId = tid;
    workers = wks;
  }

  /**
  * @brief set the dataset to feed
  * @param _r The r tuples
  * @param _s The s tuples
  */
  void setDataSet(std::vector<TrackTuplePtr> _r, std::vector<TrackTuplePtr> _s);
  /**
 * @brief to compute the throughput after run a test
 * @return the throughput in tuples/s
 */
  virtual double getThroughput();

  /**
   *  @brief to compute the latency t such that fraction of latency is below t
   *  @param fraction The fraction you want to set
   * @return the latency in us
   */
  virtual double getLatencyPercentage(double fraction);

  /**
  * @brief get the joined sum result
  * @return The result
  */
  virtual size_t getResult();

  /**
   * @brief get the joined sum result under AQP
   * @return The result
   */
  virtual size_t getAQPResult();
};
/**
 * @ingroup Parallelization
 * @typedef KeyPartitionWorkerPtr
 * @brief The class to describe a shared pointer to @ref KeyPartitionWorker
 */
typedef std::shared_ptr<class KeyPartitionWorker> KeyPartitionWorkerPtr;
/**
 * @ingroup Parallelization
 * @def newKeyPartitionWorker
 * @brief (Macro) To creat a new @ref KeyPartitionWorker under shared pointer.
 */
#define newKeyPartitionWorker std::make_shared<OoOJoin::KeyPartitionWorker>
/**
 * @class KeyPartitionRunner Parallelization/KeyPartitionRunner.h
 * @ingroup Parallelization
 * @brief the top class of key partition parallelization, which manages the @ref KeyPartitionWorker
 * @note default behaviors
 * - create
 * - call @ref setConfig
 * - call @ref setDataSet
 * -call @ref runStreaming
 * -call @ref getResult, @ref getAQPResult etc for the results
 */
class KeyPartitionRunner {
 private:

 protected:
  uint64_t threads = 1;
  std::vector<OoOJoin::KeyPartitionWorkerPtr> myWorker;

  /**
   * @brief the time struct of the whole system
   */
  //struct timeval tSystem;
  /**
   * @brief the config map of this worker
   */
  INTELLI::ConfigMapPtr cfg;
 public:
  KeyPartitionRunner() {}
  ~KeyPartitionRunner() {}

  /**
  * @brief set the dataset to feed
  * @param _r The r tuples
  * @param _s The s tuples
  */
  virtual void setDataSet(std::vector<TrackTuplePtr> _r, std::vector<TrackTuplePtr> _s);
  /**
 * @brief to compute the throughput after run a test
 * @return the throughput in tuples/s
 */
  virtual double getThroughput();

  /**
   *  @brief to compute the latency t such that fraction of latency is below t
   *  @param fraction The fraction you want to set
   * @return the latency in us
   */
  virtual double getLatencyPercentage(double fraction);

  /**
  * @brief get the joined sum result
  * @return The result
  */
  virtual size_t getResult();

  /**
   * @brief get the joined sum result under AQP
   * @return The result
   */
  virtual size_t getAQPResult();
  /**
  * @brief set the config map
  * @param _cfg the config map
  */
  virtual void setConfig(INTELLI::ConfigMapPtr _cfg);
  /**
   * @brief to run the multithread streaming process
   */
  virtual void runStreaming(void);
};
/**
 * @ingroup Parallelization
 * @typedef KeyPartitionRunnerPtr
 * @brief The class to describe a shared pointer to @ref KeyPartitionRunner
 */
typedef std::shared_ptr<class OoOJoin::KeyPartitionRunner> KeyPartitionRunnerPtr;
/**
 * @ingroup Parallelization
 * @def newKeyPartitionRunner
 * @brief (Macro) To creat a new @ref KeyPartitionRunner under shared pointer.
 */
#define newKeyPartitionRunner std::make_shared<OoOJoin::KeyPartitionRunner>

} // OoOJoin

#endif //INTELLISTREAM_INCLUDE_PARALLELIZATION_KEYPARTITIONRUNNER_H_
