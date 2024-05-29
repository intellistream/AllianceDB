//
// Created by tony on 11/03/22.
//

#include <JoinAlgos/AbstractJoinAlgo.h>

using namespace OoOJoin;

size_t OoOJoin::AbstractJoinAlgo::join(C20Buffer<OoOJoin::TrackTuplePtr> windS,
                                       C20Buffer<OoOJoin::TrackTuplePtr> windR,
                                       int threads) {

  assert(windS.data());
  assert(windR.data());
  assert(threads);
  return 0;
}

bool OoOJoin::AbstractJoinAlgo::setConfig(INTELLI::ConfigMapPtr cfg) {
  config = cfg;
  if (config == nullptr) {
    return false;
  }
  /**
   * @brief parase the joinSum config here
   */
  joinSum = cfg->tryU64("joinSum", 0, true);
  return true;
}
void OoOJoin::AbstractJoinAlgo::labelProceesedTime(C20Buffer<OoOJoin::TrackTuplePtr> windR) {
  size_t trLen = windR.size();
  TrackTuplePtr *tr = windR.data();
  for (size_t i = 0; i < trLen; i++) {
    tr[i]->processedTime = processedTime;
  }
}