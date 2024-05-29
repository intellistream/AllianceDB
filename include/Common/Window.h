/*! \file Window.h*/
//
// Created by tony on 22/11/22.
//

#ifndef INTELLISTREAM_INCLUDE_COMMON_WINDOW_H_
#define INTELLISTREAM_INCLUDE_COMMON_WINDOW_H_

#include <Common/Tuples.h>
#include <Utils/C20Buffers.hpp>

namespace OoOJoin {

/**
* @ingroup INTELLI_COMMON_BASIC Basic Definitions and Data Structures
* @{
 */
/**
 * @class Window Common/Window.h
 * @brief The class to describe a simplest window
 * @ingroup INTELLI_COMMON_BASIC
 * @note Must init before feed
 */
class Window {
 protected:
  /**
   * @brief The unique ID in system, used only for multiple window case
   */
  size_t windowID = 0;
  /**
   * @brief The start time (event) of this window
   */
  tsType startTime = 0;
  /**
   * @brief The end time (event) of this window
   */
  tsType endTime = 0;
 public:
  /**
   * @brief default creator
   */
  Window() = default;

  /**
   * @brief vreator with range, call @ref setRange
   * @param ts The start time
   * @param te  The end time
   */
  Window(tsType ts, tsType te);

  /**
  * @brief To set the range of this window
  * @param ts The start time
  * @param te The end time
  */
  void setRange(tsType ts, tsType te);

  ~Window() = default;

  INTELLI::C20Buffer<OoOJoin::TrackTuplePtr> windowS;
  INTELLI::C20Buffer<OoOJoin::TrackTuplePtr> windowR;

  /**
    * @brief init window with buffer/queue length and id
    * @param sLen The length of S queue and/or buffer
    * @param rLen The length of S queue and/or buffer
    * @param _sysId The system id
    */
  void init(size_t sLen, size_t rLen, size_t _sysId);

  /**
  * @brief feed a tuple s into the window
  * @param ts The tuple
   * @warning The current version is simplified and assuming only used in SINGLE THREAD!
   * @return bool, whether tuple is fed.
  */
  bool feedTupleS(TrackTuplePtr ts);

  /**
    * @brief feed a tuple R into the window
    * @param tr The tuple
    * @warning The current version is simplified and assuming only used in SINGLE THREAD!
    *  @return bool, whether tuple is fed.
    */
  bool feedTupleR(TrackTuplePtr tr);

  /**
   * @brief reset the window, clear all tuples
   *  @return bool, whether reset is done
   */
  bool reset(void);

  /**
   * @brief get the start time of window
   * @return the start time
   */
  tsType getStart() const {
    return startTime;
  }

  /**
   * @brief get the end time of window
   * @return the end time
   */
  tsType getEnd() const {
    return endTime;
  }
};
/**
 * @}
 */
}
#endif //INTELLISTREAM_INCLUDE_COMMON_WINDOW_H_
