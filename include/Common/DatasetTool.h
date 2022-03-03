/*! \file DatasetTool.h*/
//
// Created by tony on 02/03/22.
//

#ifndef COMMON_DATASETTOOL_H_
#define COMMON_DATASETTOOL_H_
#include <Common/Types.h>


namespace INTELLI {
/**
 * @ingroup Common
 * @{
 * @defgroup DSTool Tools for loading/creating dataset
 * @{
 * @note
 * \li We can load .txt in [key,value,subkey] format
 * \li We can creat zipf data to [key,value,subkey] format
 */

/**
 * @class DatasetTool Common/DatasetTool.h
 * @brief The class containing methods to process dataset
 */
class DatasetTool{
 public:
  DatasetTool(){}
  ~DatasetTool(){}


  /**
 * @brief To store TupleQueuePtrLocal to local text data
 * @param relationPtr The input TupleQueuePtr
 * @param fileName The name of txt file
 * @note The txt follows [key,value,subkey] format
 */
  void store3VText(TupleQueuePtr &relationPtr, const std::string &fileName);
  /**
   * @brief To load TupleQueuePtrLocal from local text data
   * @param relationPtr The output TupleQueuePtrLocal to store loaded data
   * @param fileName The name of txt file
   * @note make sure each line of the txt follows [key,value,subkey] format
   */
  void load3VText(TupleQueuePtrLocal &relationPtr, const std::string &fileName);
  /**
   * @brief To load TupleQueuePtrLocal from local text data
   * @param relationPtr The output TupleQueuePtr to store loaded data
   * @param fileName The name of txt file
   * @note make sure each line of the txt follows [key,value,subkey] format
   */
  void load3VText(TupleQueuePtr &relationPtr, const std::string &fileName);
  /**
   * @brief To load TupleQueuePtrLocal from vector of corresponding vector
   * @param relationPtr The output TupleQueuePtrLocal to store loaded data
   * @param vk Vector of key
   * @param vv Vector of value
   * @param vs vector of subkey
   * @note make sure the vector have equal size
   */
  void combine3VVector(TupleQueuePtrLocal &relationPtr, vector<keyType>vk,vector<valueType>vv,vector<size_t> vs);
  /**
   * @brief To load TupleQueuePtr from vector of corresponding vector
   * @param relationPtr The output TupleQueuePtr to store loaded data
   * @param vk Vector of key
   * @param vv Vector of value
   * @param vs vector of subkey
   * @note make sure the vector have equal size
   */
  void combine3VVector(TupleQueuePtr &relationPtr, vector<keyType>vk,vector<valueType>vv,vector<size_t> vs);
};
/**
 * @}
 * @}
 */
}
#endif //ALIANCEDB_INCLUDE_COMMON_DATASETTOOL_H_
