/*! \file LatenessWM.h*/
//
// Created by tony on 25/11/22.
//

#ifndef _INCLUDE_WATERMARKER_LATENESSWM_H_
#define _INCLUDE_WATERMARKER_LATENESSWM_H_

#include <WaterMarker/AbstractWaterMarker.h>

namespace OoOJoin {
/**
 * @ingroup ADB_WATERMARKER The watermark generator
 *@class LatenessWM WaterMarker/LatenessWM.h
 * @brief The class which generates watermark according to lateness of event time. A tuple with event time earlier
 * than max (event time we have seen) - lateness will be discard and trigger a compute.
 * @note This one assert completeness by arrival time. Require configurations:
 * - "latenessMs" U64 Here used to control the lateness, in ms
 * - "earlierEmitMs" U64 Here used to allow earlier emit, in ms
 * @note
 *
  * When assign this final water mark, it "convince' the system that everything whose event time is earlier than window bound, is arrived, so we can
  * provide the final result.
  * The condition is
  * - windowUpperBound-earlierEmit+lateness<maxEventTime
 */
class LatenessWM : public AbstractWaterMarker {
 protected:
  /**
  * @brief The max allowed lateness, internal, in us
  */
  tsType lateness = 0;
  /**
   * @brief The time of earlier emit before reaching window bound, internal, in us
   */
  tsType earlierEmit = 0;

  /**
   * @note currently only support single window
   */
  tsType windowUpperBound = 0;
  tsType maxEventTime = 0;

  bool isReachWMPoint(TrackTuplePtr tp);

 public:
  LatenessWM() = default;

  ~LatenessWM() = default;

  /**
 * @brief Set the config map related to this operator
 * @param cfg The config map
  * @return bool whether the config is successfully set
 */
  bool setConfig(ConfigMapPtr cfg) override;

  /**
  * @brief creat a window
   * @param tBegin The begin event time of the window
   * @param tEnd The end event time of the window
   * @return the id of created window
  */
  size_t creatWindow(tsType tBegin, tsType tEnd) override;

  /**
 * @brief report a tuple s into the watermark generator
 * @param ts The tuple
  * @param wid The id of window
  * @return bool, whether generate watermark after receiving this Tuple
 */
  bool reportTupleS(TrackTuplePtr ts, size_t wid = 1) override;

  /**
    * @brief Report a tuple R into the watermark generator
    * @param tr The tuple
    * @param wid The id of window
    *  @return bool, bool, whether generate watermark after receiving this Tuple
    */
  bool reportTupleR(TrackTuplePtr tr, size_t wid = 1) override;
};

/**
 * @cite PeriodicalWMPtr
 * @brief The class to describe a shared pointer to @ref  PeriodicalWM
 */
typedef std::shared_ptr<class LatenessWM> LatenessWMPtr;
/**
 * @cite newPeriodicalWM
 * @brief (Macro) To creat a new @ref PeriodicalWM under shared pointer.
 */
#define newLatenessWM std::make_shared<OoOJoin::LatenessWM>
}
#endif //INTELLISTREAM_INCLUDE_WATERMARKER_PERIODICALWM_H_
