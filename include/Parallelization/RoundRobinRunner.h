/*! \file   RoundRobinRunner.h*/

//
// Created by tony on 27/06/23.
//

#ifndef INTELLISTREAM_INCLUDE_PARALLELIZATION_RRRUNNER_H_
#define INTELLISTREAM_INCLUDE_PARALLELIZATION_RRRUNNER_H_
#include <Parallelization/KeyPartitionRunner.h>
namespace OoOJoin {
/**
 * @class RoundRobinWorker Parallelization/RoundRobinRunner.h
 * @ingroup Parallelization
 * @brief the working thread class of key partition parallelization
 */
class RoundRobinWorker : public OoOJoin::KeyPartitionWorker {
 protected:
  /**
   * @brief the time struct of the whole system
   */
  virtual void inlineMain();
  /**
   * @brief the inline main function of decentralized mode, assume there is no central distributor of datastream
   */
  void decentralizedMain();

  /**
   * @brief the inline main function of centralized mode, assume there is a central distributor of datastream
   */
  void centralizedMain();

 public:

  RoundRobinWorker() {}
  ~RoundRobinWorker() {}

};
/**
 * @ingroup Parallelization
 * @typedef RoundRobinWorkerPtr
 * @brief The class to describe a shared pointer to @ref RoundRobinWorker
 */
typedef std::shared_ptr<class RoundRobinWorker> RoundRobinWorkerPtr;
/**
 * @ingroup Parallelization
 * @def newRoundRobinWorker
 * @brief (Macro) To creat a new @ref newRoundRobinWorker under shared pointer.
 */
#define newRoundRobinWorker std::make_shared<OoOJoin::RoundRobinWorker>
/**
 * @class RoundRobinRunner Parallelization/RoundRobinRunner.h
 * @ingroup Parallelization
 * @brief the top class of key partition parallelization, which manages the @ref RoundRobinWorker
 * @note default behaviors
 * - create
 * - call @ref setConfig
 * - call @ref setDataSet
 * -call @ref runStreaming
 * -call @ref getResult, @ref getAQPResult etc for the results
 */
class RoundRobinRunner : public KeyPartitionRunner {
 protected:
  //std::vector<OoOJoin::RoundRobinWorkerPtr> myWorker;

 public:
  RoundRobinRunner() {}
  ~RoundRobinRunner() {}
  /**
 * @brief set the dataset to feed
 * @param _r The r tuples
 * @param _s The s tuples
 */
  void setDataSet(std::vector<TrackTuplePtr> _r, std::vector<TrackTuplePtr> _s);
  void setConfig(INTELLI::ConfigMapPtr _cfg);
  /**
   * @brief to run the multithread streaming process
   */
  virtual void runStreaming(void);
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
 * @typedef RoundRobinRunnerPtr
 * @brief The class to describe a shared pointer to @ref RoundRobin
 */
typedef std::shared_ptr<class OoOJoin::RoundRobinRunner> RoundRobinRunnerPtr;
/**
 * @ingroup Parallelization
 * @def newRoundRobinRunner
 * @brief (Macro) To creat a new @ref  RoundRobinRunner under shared pointer.
 */
#define newRoundRobinRunner std::make_shared<OoOJoin::RoundRobinRunner>
} // OoOJoin

#endif //INTELLISTREAM_INCLUDE_PARALLELIZATION_KEYPARTITIONRUNNER_H_
