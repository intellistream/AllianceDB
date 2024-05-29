/*! \file AbstractDataLoader.h*/
//
// Created by tony on 29/12/22.
//

#ifndef _INCLUDE_TESTBENCH_ABSTRACTDATALOADER_H_
#define _INCLUDE_TESTBENCH_ABSTRACTDATALOADER_H_

#include <Utils/ConfigMap.hpp>
#include <Common/Tuples.h>
#include <assert.h>
#include <Utils/IntelliLog.h>

using namespace INTELLI;
#define DATA_INFO INTELLI_INFO
#define DATA_ERROR INTELLI_ERROR
#define DATA_WARNNING INTELLI_WARNING
namespace OoOJoin {

/**
* @ingroup ADB_TESTBENCH The test bench to feed data into operators
* @{
 *
 */
/**
 * @defgroup ADB_TESTBENCH_DATALOADERS The classes of dataloader
 * @{
 */
/**
 * @class AbstractDataLoader TestBench/AbstractDataLoader.h
 * @brief The abstract class of dataloader
 * @ingroup ADB_TESTBENCH_DATALOADERS
 * @note:
 * - Must have a global config by @ref setConfig
 * - Can also have a modification config by
 *@note  Default behavior
* - create
* - setConfig and setModConfig (optional), generate R and S internally
* - call getTupleVectorS/R
 */
class AbstractDataLoader {
 public:
  AbstractDataLoader() = default;

  ~AbstractDataLoader() = default;

  /**
 * @brief Set the GLOBAL config map related to this loader
 * @param cfg The config map
  * @return bool whether the config is successfully set
 */
  virtual bool setConfig(ConfigMapPtr cfg) {
    assert(cfg);
    return true;
  }

  /**
* @brief Set the modification config map related to this loader
* @param cfg The config map
* @return bool whether the config is successfully set
*/
  virtual bool setModConfig(ConfigMapPtr cfg) {
    assert(cfg);
    return true;
  }

  /**
   * @brief get the vector of s tuple
   * @return the vector
   */
  virtual vector<TrackTuplePtr> getTupleVectorS() {

    vector<TrackTuplePtr> ru;
    return ru;

  }

  /**
  * @brief get the vector of R tuple
  * @return the vector
  */
  virtual vector<TrackTuplePtr> getTupleVectorR() {
    vector<TrackTuplePtr> ru;
    return ru;

  }
};

/**
 * @ingroup ADB_TESTBENCH_DATALOADERS
 * @typedef AbstractDataLoaderPtr
 * @brief The class to describe a shared pointer to @ref AbstractDataLoader

 */
typedef std::shared_ptr<class AbstractDataLoader> AbstractDataLoaderPtr;
/**
 * @ingroup ADB_TESTBENCH_DATALOADERS
 * @def newAbstractDataLoader
 * @brief (Macro) To creat a new @ref  AbstractDataLoader under shared pointer.
 */
#define newAbstractDataLoader std::make_shared<OoOJoin::AbstractDataLoader>
/**
 * @}
 */
/**
 *
 */
}
#endif //INTELLISTREAM_INCLUDE_TESTBENCH_ABSTRACTDATALOADER_H_
