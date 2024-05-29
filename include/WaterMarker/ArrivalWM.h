/*! \file ArrivalWM.h*/
//
// Created by tony on 25/11/22.
//

#ifndef _INCLUDE_WATERMARKER_ArrivalWM_H_
#define _INCLUDE_WATERMARKER_ArrivalWM_H_

#include <WaterMarker/AbstractWaterMarker.h>

namespace OoOJoin {
/**
 * @ingroup ADB_WATERMARKER The watermark generator
 *@class ArrivalWM WaterMarker/ArrivalWM.h
 * @brief watermark generates at time t, and every thing arrives later then t will be discard, i.e., flush finial result at t
 * @todo multi window support is not done yet, left for future, but we do preserve the interfaces
 * @note This one assert completeness by arrival time. Require configurations:
 * - "errorBound" Double The assigned error bound
 * - "timeStep" U64 The simulation time step in us
 * - "watermarkTimeMs" U64 The time to generate a finial watermark in ms, indicating all data is ready
 */
class ArrivalWM : public AbstractWaterMarker {
 protected:
  /**
  * @brief The period to generate watermark, internal, in us
  */
  tsType watermarkTime = 0;
  /**
  * @brief The Delta value for next watermark
  */
  tsType nextWMDelta = 0;
  /**
   * @note currently only support single window
   */
  tsType windowLen = 0;
  tsType nextWMPoint = 0;

//  tsType maxEventTime=0;
  bool isReachWMPoint(TrackTuplePtr tp);

 public:
  ArrivalWM() = default;

  ~ArrivalWM() = default;

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
 * @cite ArrivalWMPtr
 * @brief The class to describe a shared pointer to @ref  ArrivalWM
 */
typedef std::shared_ptr<class ArrivalWM> ArrivalWMPtr;
/**
 * @cite newArrivalWM
 * @brief (Macro) To creat a new @ref ArrivalWM under shared pointer.
 */
#define newArrivalWM std::make_shared<OoOJoin::ArrivalWM>
}
#endif //INTELLISTREAM_INCLUDE_WATERMARKER_ArrivalWM_H_
