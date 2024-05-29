//
// Created by tony on 25/11/22.
//

#include <WaterMarker/ArrivalWM.h>

bool OoOJoin::ArrivalWM::setConfig(INTELLI::ConfigMapPtr cfg) {
  if (!AbstractWaterMarker::setConfig(cfg)) {
    return false;
  }
  watermarkTime = cfg->tryU64("watermarkTimeMs", 10, true) * 1000;
  return true;
}

size_t OoOJoin::ArrivalWM::creatWindow(OoOJoin::tsType tBegin, OoOJoin::tsType tEnd) {
  windowLen = tEnd - tBegin;
  if (watermarkTime == 0) {
    nextWMDelta = windowLen + 1;
  } else {
    nextWMDelta = watermarkTime;
  }
  nextWMPoint = nextWMDelta;
  return 1;
}

bool OoOJoin::ArrivalWM::isReachWMPoint(OoOJoin::TrackTuplePtr tp) {
  if (tp->arrivalTime >= nextWMPoint) {
    WM_INFO("Watermark reached at" + tp->toString());
    nextWMPoint += nextWMDelta;
    return true;
  }
  return false;
}

bool OoOJoin::ArrivalWM::reportTupleS(OoOJoin::TrackTuplePtr ts, size_t wid) {
  if (!wid) {
    return false;
  }
  return isReachWMPoint(ts);
}

bool OoOJoin::ArrivalWM::reportTupleR(OoOJoin::TrackTuplePtr tr, size_t wid) {
  if (!wid) {
    return false;
  }
  return isReachWMPoint(tr);
}