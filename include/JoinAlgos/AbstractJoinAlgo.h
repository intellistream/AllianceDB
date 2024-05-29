/*! \file AbstractJoinAlgo.h*/
//
// Created by tony on 11/03/22.
//

#ifndef _JOINALGO_ABSTRACTJOINALGO_H_
#define _JOINALGO_ABSTRACTJOINALGO_H_

#include <Common/Window.h>
#include <string>
#include <memory>
#include <assert.h>
#include <Utils/ConfigMap.hpp>
#include <utility>
#include "Operator/MSWJ/Profiler/TupleProductivityProfiler.h"

//#include <Utils/Logger.hpp>/
using namespace INTELLI;
#define ALGO_INFO INTELLI_INFO
#define ALGO_ERROR INTELLI_ERROR
#define ALGO_WARNNING INTELLI_WARNING
namespace OoOJoin {
/**
* @defgroup ADB_JOINALGOS The specific join algorithms
* @{
 * @todo Add latency tracking and throughput evaluation for each algorithms, we use the @ref TrackTuple so just fill in the processedTIme is enough
* State-of-art joins algorithms. We use a register to table called @ref JoinAlgoTable to manage and access different algos in an unified way, and user-defined
 * new algos should also be registered in that table.
 *
 * */
/**
 * @defgroup ADB_JOINALGOS_ABSTRACT Common Abstraction and Interface
 * @{
 */
/**
 * @ingroup ADB_JOINALGOS_ABSTRACT
 * @class AbstractJoinAlgo JoinAlgos/AbstractJoinAlgo.h
 * @brief The abstraction to describe a join algorithm, providing virtual function of join
 */
class AbstractJoinAlgo {
 protected:
  std::string nameTag;
  uint64_t joinSum = 0;
  struct timeval timeBaseStruct{};
  uint64_t processedTime=0;
  //tsType timeStep;
 public:
  //元组生产力监视器
  MSWJ::TupleProductivityProfiler *productivity_profiler_;

  ConfigMapPtr config;

  AbstractJoinAlgo() {
    setAlgoName("NULL");
  }

  ~AbstractJoinAlgo() = default;

  /**
  * @brief Set the config map related to this Algorithm
  * @param cfg The config map
   * @return bool whether the config is successfully set
  */
  virtual bool setConfig(ConfigMapPtr cfg);

  /**
  * @brief The function to execute join,
  * @param windS The window of S tuples
   * @param windR The window of R tuples
   * @param threads The threads for executing this join
  * @return The joined tuples
   * @note Please at least mark the final processed time at rTuples
  */
  virtual size_t join(C20Buffer<OoOJoin::TrackTuplePtr> windS,
                      C20Buffer<OoOJoin::TrackTuplePtr> windR,
                      int threads = 1);
  virtual void labelProceesedTime( C20Buffer<OoOJoin::TrackTuplePtr> windR);
  /**
   * @brief set the name of algorithm
   * @param name Algorithm name
   */
  void setAlgoName(std::string name) {
    nameTag = std::move(name);
  }

  /**
  * @brief get the name of algorithm
  * @return The name
  */
  std::string getAlgoName() {
    return nameTag;
  }

  /**
   * @brief Synchronize the time structure with outside setting
   * @param tv The outside time structure
   */
  void syncTimeStruct(struct timeval tv) {
    timeBaseStruct = tv;
  }
};

typedef std::shared_ptr<AbstractJoinAlgo> AbstractJoinAlgoPtr;
#define  newAbstractJoinAlgo() std::make_shared<AbstractJoinAlgo>()
/**
 * @}
 * @}
 */
}

#endif //ALIANCEDB_INCLUDE_JOINALGO_ABSTRACTJOINALGO_H_
