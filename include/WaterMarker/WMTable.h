//
// Created by tony on 31/12/22.
//

#ifndef INTELLISTREAM_INCLUDE_WATERMARKER_WMTABLE_H_
#define INTELLISTREAM_INCLUDE_WATERMARKER_WMTABLE_H_

#include <WaterMarker/AbstractWaterMarker.h>
#include <map>
#include <string>

namespace OoOJoin {
/**
 * @ingroup ADB_WATERMARKER The watermark generator
 *@class WMTable WaterMarker/WMTable.h
 * @brief The table to indexing all possible watermark generators
 * @note default tags:
 * - "lateness" The @ref LatenessWM
 * - "arrival" The @ref ArrivalWM
 */
class WMTable {
 protected:
  std::map<std::string, AbstractWaterMarkerPtr> wmMap;
 public:
  WMTable();

  ~WMTable() {}

  /**
 * @brief To register a new watermarker
 * @param onew The new watermarker
 * @param tag THe name tag
 */
  void registerNewWM(AbstractWaterMarkerPtr onew, std::string tag) {
    wmMap[tag] = onew;
  }

  /**
   * @brief find an operator in the table according to its name
   * @param name The name of operator
   * @return The operator, nullptr if not found
   */
  AbstractWaterMarkerPtr findWM(std::string name) {
    if (wmMap.count(name)) {
      return wmMap[name];
    }
    return nullptr;
  }
};

/**
 * @ingroup ADB_WATERMARKER
 * @typedef WMTablePtr
 * @brief The class to describe a shared pointer to @ref WMTable
 */
typedef std::shared_ptr<class WMTable> WMTablePtr;
/**
 * @ingroup ADB_WATERMARKER
 * @def newWMTable
 * @brief (Macro) To creat a new @ref WMTable under shared pointer.
 */
#define newWMTable std::make_shared<OoOJoin::WMTable>
}
#endif //INTELLISTREAM_INCLUDE_WATERMARKER_WMTABLE_H_
