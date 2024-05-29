/*! \file DataLoaderTable.h*/
//
// Created by tony on 29/12/22.
//

#ifndef _INCLUDE_TESTBENCH_DATALOADERTABLE_H_
#define _INCLUDE_TESTBENCH_DATALOADERTABLE_H_

#include <map>
#include <TestBench/AbstractDataLoader.h>

using namespace INTELLI;
namespace OoOJoin {


/**
* @ingroup ADB_TESTBENCH The test bench to feed data into operators
* @{
 *
 */
/**
 * @class DataLoaderTable TestBench/DataLoaderTable.h
 * @ingroup ADB_TESTBENCH
 *  @brief The entity of DataLoaderTable, can used for searching all available DataLoaders
 *  @note please edit corresponding .cpp if adding a new data loader
 *  @note Default tags
 *  - "random" @ref RandomDataLoader
 *  - "file" @ref FileDataLoader
 *  - "zipf" @ref ZipfDataLoader
 */
class DataLoaderTable {
 protected:
  std::map<std::string, AbstractDataLoaderPtr> loaderMap;
 public:
/**
   * @brief The constructing function
   * @note  If new DataLoader is added, please change this file and its .cpp
   */
  DataLoaderTable();

  ~DataLoaderTable() {
  }

  /**
 * @brief To register a new loader
 * @param onew The new operator
 * @param tag THe name tag
 */
  void registerNewDataLoader(AbstractDataLoaderPtr dnew, std::string tag) {
    loaderMap[tag] = dnew;
  }

  /**
   * @brief find a dataloader in the table according to its name
   * @param name The name of operator
   * @return The DataLoader, nullptr if not found
   */
  AbstractDataLoaderPtr findDataLoader(std::string name) {
    if (loaderMap.count(name)) {
      return loaderMap[name];
    }
    return nullptr;
  }
};

/**
 * @ingroup ADB_TESTBENCH
 * @typedef DataLoaderTable
 * @brief The class to describe a shared pointer to @ref DataLoaderTable
 */
typedef std::shared_ptr<class DataLoaderTable> DataLoaderTablePtr;
/**
 * @ingroup ADB_OPERATORS
 * @def newDataLoaderTable
 * @brief (Macro) To creat a new @ref DataLoaderTable under shared pointer.
 */
#define newDataLoaderTable std::make_shared<OoOJoin::DataLoaderTable>
/**
 * @}
 */
}
#endif //INTELLISTREAM_INCLUDE_TESTBENCH_DATALOADERTABLE_H_
