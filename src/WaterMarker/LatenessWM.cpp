//
// Created by tony on 25/11/22.
//

#include <WaterMarker/LatenessWM.h>

bool OoOJoin::LatenessWM::setConfig(INTELLI::ConfigMapPtr cfg) {
  if (!AbstractWaterMarker::setConfig(cfg)) {
    return false;
  }
  tsType latenessMs = lateness = cfg->tryU64("latenessMs", 0, true);
  lateness = latenessMs * 1000;
  earlierEmit = cfg->tryU64("earlierEmitMs", 0, true) * 1000;
  return true;
}

size_t OoOJoin::LatenessWM::creatWindow(OoOJoin::tsType tBegin, OoOJoin::tsType tEnd) {
  //windowLen = tEnd - tBegin;
  if (tBegin > tEnd) {
    return 0;
  }
  windowUpperBound = tEnd;
  return 1;
}

bool OoOJoin::LatenessWM::isReachWMPoint(OoOJoin::TrackTuplePtr tp) {
  if (tp->eventTime > maxEventTime) {
    maxEventTime = tp->eventTime;
  }
  /**
   * When assign this final water mark, it asserts that everything earlier than window bound is arrived, so we can
   * provide the final result
   */
  if (windowUpperBound - earlierEmit + lateness < maxEventTime) {
    INTELLI_INFO("Max event time =" + to_string(maxEventTime) + ", windowBound=" + to_string(windowUpperBound));
    INTELLI_INFO("When seeing tuple " + tp->toString());
    return true;
  }

  return false;
}

bool OoOJoin::LatenessWM::reportTupleS(OoOJoin::TrackTuplePtr ts, size_t wid) {
  if (!wid) {
    return false;
  }

  return isReachWMPoint(ts);
}

bool OoOJoin::LatenessWM::reportTupleR(OoOJoin::TrackTuplePtr tr, size_t wid) {
  if (!wid) {
    return false;
  }
  return isReachWMPoint(tr);
}