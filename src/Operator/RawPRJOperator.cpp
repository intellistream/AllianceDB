//
// Created by tony on 22/11/22.
//

#include <Operator/RawPRJOperator.h>
#include <JoinAlgos/JoinAlgoTable.h>
#include <chrono>
bool OoOJoin::RawPRJOperator::setConfig(INTELLI::ConfigMapPtr cfg) {
  if (!OoOJoin::AbstractOperator::setConfig(cfg)) {
    return false;
  }
  algoTag = "NPJSingle";
  // OP_INFO("selected join algorithm=" + algoTag);

  joinThreads = 1;

  joinSum = config->tryU64("joinSum", 0, true);
  // OP_INFO("selected join threads=" + to_string(joinThreads));
  return true;
}

bool OoOJoin::RawPRJOperator::start() {

  /**
   * @brief set window
   */
  myWindow.setRange(0, windowLen);
  myWindow.init(sLen, rLen, 1);
  intermediateResult = 0;
  lockedByWaterMark = false;
  return true;
}

void OoOJoin::RawPRJOperator::conductComputation() {
  JoinAlgoTable jt;
  AbstractJoinAlgoPtr algo = jt.findAlgo(algoTag);
  //NestedLoopJoin nj;
  algo->setConfig(config);
  algo->syncTimeStruct(timeBaseStruct);
  // OP_INFO("Invoke algorithm=" + algo->getAlgoName());
  auto lazyStart = std::chrono::high_resolution_clock::now();
  intermediateResult = algo->join(myWindow.windowS, myWindow.windowR, joinThreads);
  auto lazyEnd = std::chrono::high_resolution_clock::now();

  // Compute the duration in microseconds
  auto lazyDuration = std::chrono::duration_cast<std::chrono::microseconds>(lazyEnd - lazyStart);

  lazyRunningTime=lazyDuration.count();
  algo->labelProceesedTime(myWindow.windowR);
}

bool OoOJoin::RawPRJOperator::stop() {
  if (lockedByWaterMark) {
    WM_INFO("early terminate by watermark, already have results");
  }
  if (!lockedByWaterMark) {
    WM_INFO("No watermark encountered, compute now");
    //force to flush, if no watermark is given
    conductComputation();
  }
  return true;
}

bool OoOJoin::RawPRJOperator::feedTupleS(OoOJoin::TrackTuplePtr ts) {
  bool shouldGenWM;
  if (lockedByWaterMark) {
    return false;
  }
  myWindow.feedTupleS(ts);
  /**
   * @brief once see the event reaches window end, terminate everything
   */
  if (ts->arrivalTime > myWindow.getEnd()) {
    shouldGenWM = true;
  }
  if (shouldGenWM) {
    lockedByWaterMark = true;
    // run computation
    conductComputation();
  }
  return true;
}

bool OoOJoin::RawPRJOperator::feedTupleR(OoOJoin::TrackTuplePtr tr) {
  bool shouldGenWM;
  if (lockedByWaterMark) {
    return false;
  }
  myWindow.feedTupleR(tr);
  if (tr->arrivalTime > myWindow.getEnd()) {
    shouldGenWM = true;
  }
  if (shouldGenWM) {
    lockedByWaterMark = true;
    // run computation
    conductComputation();
  }
  return true;
}

size_t OoOJoin::RawPRJOperator::getResult() {
  return intermediateResult;
}

double OoOJoin::RawPRJOperator::getLazyRunningThroughput() {
  double cnt=myWindow.windowR.size()*1e6;
  /*NPJTuplePtr *tr = myWindow.windowR.data();
  size_t tRlen=myWindow.windowR.size();
  uint64_t minArrival=99999;
  uint64_t maxArrival=0;
  for (size_t i = 0; i < tRlen; i++) {
    if (tr[i]->processedTime>0) {
      if(tr[i]->arrivalTime>maxArrival)
      {
        maxArrival=tr[i]->arrivalTime;
      }
      if(tr[i]->arrivalTime<minArrival)
      {
        minArrival=tr[i]->arrivalTime;
      }
    }
  }
  INTELLI_INFO(to_string(myWindow.windowR.size())+"tuples");*/
  return cnt/(lazyRunningTime);
}