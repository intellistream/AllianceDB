//
// Created by tony on 29/12/22.
//

#include <TestBench/RandomDataLoader.h>

using namespace INTELLI;
using namespace OoOJoin;

bool RandomDataLoader::setConfig(ConfigMapPtr cfg) {
  cfgGlobal = cfg;
  windowLenMs = cfg->tryU64("windowLenMs", 10);
  timeStepUs = cfg->tryU64("timeStepUs", 40);
  watermarkTimeMs = cfg->tryU64("watermarkTimeMs", 10);
  maxArrivalSkewMs = cfg->tryU64("maxArrivalSkewMs", 10 / 2);
  eventRateKTps = cfg->tryU64("eventRateKTps", 10);
  uint64_t keyRange = cfg->tryU64("keyRange", 10);
  size_t testSize = (windowLenMs + maxArrivalSkewMs) * eventRateKTps;
  sTuple = genTuplesSmooth(testSize, keyRange, eventRateKTps, timeStepUs, maxArrivalSkewMs * 1000, 7758258);
  rTuple = genTuplesSmooth(testSize, keyRange, eventRateKTps, timeStepUs, maxArrivalSkewMs * 1000, 114514);
  return true;
}

vector<TrackTuplePtr> RandomDataLoader::getTupleVectorS() {
  return sTuple;
}

vector<TrackTuplePtr> RandomDataLoader::getTupleVectorR() {
  return rTuple;
}