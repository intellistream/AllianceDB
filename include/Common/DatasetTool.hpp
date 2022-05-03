/*! \file DatasetTool.h*/
#ifndef _COMMON_DATASETTOOL_H_
#define _COMMON_DATASETTOOL_H_
#include <Common/Types.hpp>
#include <Common/Tuple.hpp>
#include <Common/Relations.hpp>

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
   * @brief To load Relation from local text data
   * @param relation The output Relation to store loaded data
   * @param fileName The name of txt file
   * @note make sure each line of the txt follows [key,value,subkey] format
   */
  void LoadData(Relation &relation, const std::string &fileName);
};
}
#endif //ALIANCEDB_INCLUDE_COMMON_DATASETTOOL_H_
