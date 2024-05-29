/*! \file FileDataLoader.h*/
//
// Created by tony on 29/12/22.
//

#ifndef _INCLUDE_TESTBENCH_FileDataLoader_H_
#define _INCLUDE_TESTBENCH_FileDataLoader_H_

#include <Utils/ConfigMap.hpp>
#include <Common/Tuples.h>
#include <assert.h>
#include <TestBench/AbstractDataLoader.h>

using namespace INTELLI;

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
 * @class FileDataLoader TestBench/FileDataLoader.h
 * @brief The dataloader which loads data from external csv file
 * @ingroup ADB_TESTBENCH_DATALOADERS
 * @note:
 * - Must have a global config by @ref setConfig
 * - Can also have a modification config by
 *@note  Default behavior
* - create
* - setConfig and setModConfig (optional), generate R and S internally
* - call getTupleVectorS/R
* @note Require configs
 * - "fileDataLoader_rFile" String The file name of r tuple
 * - "fileDataLoader_sFile" String The file name of s tuple
 */
class FileDataLoader : public AbstractDataLoader {
 protected:

  void spilt(const std::string s, const std::string &c, vector<std::string> &v) {
    std::string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while (std::string::npos != pos2) {
      v.push_back(s.substr(pos1, pos2 - pos1));

      pos1 = pos2 + c.size();
      pos2 = s.find(c, pos1);
    }
    if (pos1 != s.length())
      v.push_back(s.substr(pos1));
  }

  /**
  * @brief load a dataset from csv file
  * @param fname The name of file
  * @param separator The separator in .csv, default is ","
  * @param newLine THe indicator of a new line. default is "\n"
  * @return The vector of TrackTuplePtr
  */
  std::vector<TrackTuplePtr> loadDataFromCsv(std::string fname,
                                             std::string separator = ",",
                                             std::string newLine = "\n");

 public:
  ConfigMapPtr cfgGlobal;
  vector<TrackTuplePtr> sTuple, rTuple;

  FileDataLoader() = default;

  ~FileDataLoader() = default;

  /**
 * @brief Set the GLOBAL config map related to this loader
 * @param cfg The config map
  * @return bool whether the config is successfully set
 */
  bool setConfig(ConfigMapPtr cfg) override;

  /**
* @brief Set the modification config map related to this loader
* @param cfg The config map
* @return bool whether the config is successfully set
*/
  bool setModConfig(ConfigMapPtr cfg) override {
    assert(cfg);
    return true;
  }

  /**
   * @brief get the vector of s tuple
   * @return the vector
   */
  vector<TrackTuplePtr> getTupleVectorS() override;

  /**
  * @brief get the vector of R tuple
  * @return the vector
  */
  vector<TrackTuplePtr> getTupleVectorR() override;

};

/**
 * @ingroup ADB_TESTBENCH_DATALOADERS
 * @typedef FileDataLoaderPtr
 * @brief The class to describe a shared pointer to @ref FileDataLoader

 */
typedef std::shared_ptr<class FileDataLoader> FileDataLoaderPtr;
/**
 * @ingroup ADB_TESTBENCH_DATALOADERS
 * @def newFileDataLoader
 * @brief (Macro) To creat a new @ref  FileDataLoader under shared pointer.
 */
#define newFileDataLoader std::make_shared<OoOJoin::FileDataLoader>
/**
 * @}
 */
/**
 *
 */
}
#endif //INTELLISTREAM_INCLUDE_TESTBENCH_FileDataLoader_H_
