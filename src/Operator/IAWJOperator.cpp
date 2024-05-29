//
// Created by tony on 22/11/22.
//

#include <Operator/IAWJOperator.h>
#include <JoinAlgos/JoinAlgoTable.h>

bool OoOJoin::IAWJOperator::setConfig(INTELLI::ConfigMapPtr cfg) {
  if (!OoOJoin::AbstractOperator::setConfig(cfg)) {
    return false;
  }
  algoTag = config->tryString("algo", "NPJSingle", true);
  // OP_INFO("selected join algorithm=" + algoTag);
  if (config->existU64("threads")) {
    joinThreads = config->getU64("threads");
  }
  std::string wmTag = config->tryString("wmTag", "arrival", true);
  WMTablePtr wmTable = newWMTable();
  wmGen = wmTable->findWM(wmTag);
  if (wmGen == nullptr) {
    INTELLI_ERROR("NO such a watermarker named [" + wmTag + "]");
    return false;
  }
  joinSum = config->tryU64("joinSum", 0, true);
  INTELLI_INFO("Using the watermarker named [" + wmTag + "]");
  // OP_INFO("selected join threads=" + to_string(joinThreads));
  return true;
}

bool OoOJoin::IAWJOperator::start() {
  /**
   * @brief set watermark generator
   */
  //wmGen = newPeriodicalWM();
  wmGen->setConfig(config);
  wmGen->syncTimeStruct(timeBaseStruct);
  /**
   * @note:
  */
  wmGen->creatWindow(0, windowLen);
  /**
   * @brief set window
   */
  myWindow.setRange(0, windowLen);
  myWindow.init(sLen, rLen, 1);
  intermediateResult = 0;
  lockedByWaterMark = false;
  return true;
}

void OoOJoin::IAWJOperator::conductComputation() {
  JoinAlgoTable jt;
  AbstractJoinAlgoPtr algo = jt.findAlgo(algoTag);
  //NestedLoopJoin nj;
  algo->setConfig(config);
  algo->syncTimeStruct(timeBaseStruct);
  // OP_INFO("Invoke algorithm=" + algo->getAlgoName());
  intermediateResult = algo->join(myWindow.windowS, myWindow.windowR, joinThreads);
  algo->labelProceesedTime(myWindow.windowR);
}

bool OoOJoin::IAWJOperator::stop() {
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

bool OoOJoin::IAWJOperator::feedTupleS(OoOJoin::TrackTuplePtr ts) {
  bool shouldGenWM;
  if (lockedByWaterMark) {
    return false;
  }
  myWindow.feedTupleS(ts);
  shouldGenWM = wmGen->reportTupleS(ts, 1);
  if (shouldGenWM) {
    lockedByWaterMark = true;
    // run computation
    conductComputation();
  }
  return true;
}

bool OoOJoin::IAWJOperator::feedTupleR(OoOJoin::TrackTuplePtr tr) {
  bool shouldGenWM;
  if (lockedByWaterMark) {
    return false;
  }
  myWindow.feedTupleR(tr);
  shouldGenWM = wmGen->reportTupleR(tr, 1);
  if (shouldGenWM) {
    lockedByWaterMark = true;
    // run computation
    conductComputation();
  }
  return true;
}

size_t OoOJoin::IAWJOperator::getResult() {
  return intermediateResult;
}
