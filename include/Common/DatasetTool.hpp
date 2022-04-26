/*! \file DatasetTool.h*/
#ifndef _COMMON_DATASETTOOL_H_
#define _COMMON_DATASETTOOL_H_
#include <Common/Types.hpp>
#include <Common/Tuple.hpp>

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
class DatasetTool {
 public:
  DatasetTool() {}
  ~DatasetTool() {}

  /**
 * @brief To store TuplePtrQueueIn to local text data
 * @param relationPtr The input TuplePtrQueue
 * @param fileName The name of txt file
 * @note The txt follows [key,value,subkey] format
 */
  void store3VText(TuplePtrQueue &relationPtr, const std::string &fileName);
  /**
   * @brief To load TuplePtrQueueIn from local text data
   * @param relationPtr The output TuplePtrQueueIn to store loaded data
   * @param fileName The name of txt file
   * @note make sure each line of the txt follows [key,value,subkey] format
   */
  void load3VText(TuplePtrQueueIn &relationPtr, const std::string &fileName);
  /**
   * @brief To load TuplePtrQueueIn from local text data
   * @param relationPtr The output TuplePtrQueue to store loaded data
   * @param fileName The name of txt file
   * @note make sure each line of the txt follows [key,value,subkey] format
   */
  void load3VText(TuplePtrQueue &relationPtr, const std::string &fileName);
  /**
   * @brief To load TuplePtrQueueIn by combining 3 stand-alone vectors of key, value,subkey
   * @param relationPtr The output TuplePtrQueueIn to store loaded data
   * @param vk Vector of key
   * @param vv Vector of value
   * @param vs vector of subkey
   * @note make sure the vector have equal size
   */
  void combine3VVector(TuplePtrQueueIn &relationPtr, vector<keyType> vk, vector<valueType> vv, vector<size_t> vs);
  /**
   * @brief To load TuplePtrQueue by combining 3 stand-alone vectors of key, value,subkey
   * @param relationPtr The output TuplePtrQueue to store loaded data
   * @param vk Vector of key
   * @param vv Vector of value
   * @param vs vector of subkey
   * @note make sure the vector have equal size
   */
  void combine3VVector(TuplePtrQueue &relationPtr, vector<keyType> vk, vector<valueType> vv, vector<size_t> vs);

};
}
#endif //ALIANCEDB_INCLUDE_COMMON_DATASETTOOL_H_
