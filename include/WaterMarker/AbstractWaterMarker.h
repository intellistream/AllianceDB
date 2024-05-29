/*! \file AbstractWaterMarker.h*/
//
// Created by tony on 24/11/22.
//

#ifndef _INCLUDE_WATERMARKER_ABSTRACTWATERMARKER_H_
#define _INCLUDE_WATERMARKER_ABSTRACTWATERMARKER_H_

#include <Common/Window.h>
#include <Common/Tuples.h>
#include <Common/Window.h>
#include <Common/Tuples.h>
#include <Utils/UtilityFunctions.hpp>
#include <assert.h>
#include <Utils/ConfigMap.hpp>

#include <Utils/IntelliLog.h>

#define WM_INFO INTELLI_INFO
#define WM_ERROR INTELLI_ERROR
#define WM_WARNNING INTELLI_WARNING
using namespace INTELLI;
namespace OoOJoin {
/**
 * @ingroup ADB_WATERMARKER The watermark generator
 *@class AbstractWaterMarker WaterMarker/AbstractWaterMarker.h
 * @brief The abstraction to describe a watermark generator, providing virtual function of deciding watermark generation
 * @todo multi window support is not done yet, left for future, but we do preserve the interfaces
 * @note require configurations:
 * - "errorBound" Double The assigned error bound
 */
class AbstractWaterMarker {
 protected:
  /**
   * @brief The assigned error bound, read from configuration
   */
  double errorBound;
  //tsType timeStep;
  /**
   * @brief the base structure for time
   */
  struct timeval timeBaseStruct;

  /**
   * @brief estimate the error of a window
   * @param wid The window id
   * @return The estimated error
   */
  virtual double estimateError(size_t wid = 1);

 public:
  ConfigMapPtr config = nullptr;

  AbstractWaterMarker() {}

  ~AbstractWaterMarker() {}

  /**
   * @brief Separately set the errorBound
   * @param err The errorBound
   */
  void setErrorBound(double err) {
    errorBound = err;
  }

  /**
   * @brief Synchronize the time structure with outside setting
   * @param tv The outside time structure
   */
  void syncTimeStruct(struct timeval tv) {
    timeBaseStruct = tv;
  }

  /**
  * @brief Set the config map related to this operator
  * @param cfg The config map
   * @return bool whether the config is successfully set
  */
  virtual bool setConfig(ConfigMapPtr cfg);

  /**
  * @brief init the generator
   * @return bool whether the generator is successfully inited
   * @note call before using
  */
  virtual bool init(void);

  /**
  * @brief creat a window
   * @param tBegin The begin event time of the window
   * @param tEnd The end event time of the window
   * @return the id of created window
  */
  virtual size_t creatWindow(tsType tBegin, tsType tEnd);

  /**
   * @brief delete the window and recycle its id
   * @return bool, whether the window is released
   */
  virtual bool deleteWindow(size_t wid);

  /**
  * @brief report a tuple s into the watermark generator
  * @param ts The tuple
   * @param wid The id of window
   * @return bool, whether generate watermark after receiving this Tuple
  */
  virtual bool reportTupleS(TrackTuplePtr ts, size_t wid = 1);

  /**
    * @brief Report a tuple R into the watermark generator
    * @param tr The tuple
    * @param wid The id of window
    *  @return bool, bool, whether generate watermark after receiving this Tuple
    */
  virtual bool reportTupleR(TrackTuplePtr tr, size_t wid = 1);

};

/**
 * @cite AbstractWaterMarkerPtr
 * @brief The class to describe a shared pointer to @ref AbstractWaterMarker
 */
typedef std::shared_ptr<class AbstractWaterMarker> AbstractWaterMarkerPtr;
/**
 * @cite newAbstractWaterMarker
 * @brief (Macro) To creat a new @ref AbstractWaterMarker under shared pointer.
 */
#define newAbstractWaterMarker std::make_shared<OoOJoin::AbstractWaterMarker>
}
#endif //INTELLISTREAM_INCLUDE_WATERMARKER_ABSTRACTWATERMARKER_H_
