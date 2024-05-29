
#include <Operator/IAWJSelOperator.h>

bool OoOJoin::IAWJSelOperator::setConfig(INTELLI::ConfigMapPtr cfg) {
  if (!OoOJoin::MeanAQPIAWJOperator::setConfig(cfg)) {
    return false;
  }
  std::string wmTag = config->tryString("wmTag", "arrival", true);
  WMTablePtr wmTable = newWMTable();
  wmGen = wmTable->findWM(wmTag);
  if (wmGen == nullptr) {
    INTELLI_ERROR("NO such a watermarker named [" + wmTag + "]");
    return false;
  }
  INTELLI_INFO("Using the watermarker named [" + wmTag + "]");
  noRTrace = newIMAStateOfKey();
  noSTrace = newIMAStateOfKey();
  joinSum = cfg->tryU64("joinSum", 0, true);
  wmGen->setConfig(config);
  return true;
}

bool OoOJoin::IAWJSelOperator::start() {
  /**
  * @brief set watermark generator
  */
  //wmGen = newPeriodicalWM();

  wmGen->syncTimeStruct(timeBaseStruct);
  /**
   * @note:
  */
  wmGen->creatWindow(0, windowLen);
  // wmGen->creatWindow(0, windowLen);
  /**
   * @brief set window
   */
  stateOfKeyTableR = newStateOfKeyHashTable(4096, 4);
  stateOfKeyTableS = newStateOfKeyHashTable(4096, 4);
  myWindow.setRange(0, windowLen);
  windowBound = windowLen;
  myWindow.init(sLen, rLen, 1);

  intermediateResult = 0;
  confirmedResult = 0;
  lockedByWaterMark = false;
  timeBreakDownPrediction = 0;
  timeBreakDownIndex = 0;
  timeBreakDownJoin = 0;
  timeBreakDownAll = 0;timeTrackingStartNoClaim(timeBreakDownAll);
  /**
   * @brief additional set-ups of prediction states
   */
  selPredictor.reset();
  return true;
}

void OoOJoin::IAWJSelOperator::conductComputation() {

}

bool OoOJoin::IAWJSelOperator::stop() {
  if (lockedByWaterMark) {
    WM_INFO("early terminate by watermark, already have results");
  }
  if (!lockedByWaterMark) {
    WM_INFO("No watermark encountered, compute now");
  }
  timeBreakDownAll = timeTrackingEnd(timeBreakDownAll);

  size_t rLen = myWindow.windowR.size();
  OoOJoin::TrackTuplePtr *tr = myWindow.windowR.data();
  tsType timeNow = lastTimeOfR;
  for (size_t i = 0; i < rLen; i++) {
    if (tr[i]->arrivalTime < timeNow) {
      tr[i]->processedTime = timeNow;
    }
  }
  return true;
}
bool OoOJoin::IAWJSelOperator::feedTupleS(OoOJoin::TrackTuplePtr ts) {
  bool shouldGenWM, isInWindow;
  if (lockedByWaterMark) {
    return false;
  }
  isInWindow = myWindow.feedTupleS(ts);
  shouldGenWM = wmGen->reportTupleS(ts, 1);
  if (shouldGenWM) {
    lockedByWaterMark = true;
    WM_INFO("water mark in S");
    lastTimeOfR = UtilityFunctions::timeLastUs(timeBaseStruct);
  }
  if (isInWindow) {
    IMAStateOfKeyPtr stateOfKey;
    /**
     * @brief First get the index of hash table
     */
    timeTrackingStart(tt_index);
    AbstractStateOfKeyPtr stateOfSKey = stateOfKeyTableS->getByKey(ts->key);
    if (stateOfSKey == nullptr) // this key doesn't exist
    {
      stateOfKey = newIMAStateOfKey();
      stateOfKey->key = ts->key;
      stateOfKeyTableS->insert(stateOfKey);
    } else {
      stateOfKey = ImproveStateOfKeyTo(IMAStateOfKey, stateOfSKey);
    }
    timeBreakDownIndex += timeTrackingEnd(tt_index);
    /**
     *
     */
    timeTrackingStart(tt_prediction);
    updateStateOfKey(stateOfKey, ts);
    double futureTuplesS = MeanAQPIAWJOperator::predictUnarrivedTuples(stateOfKey);
    /**
     * @brief make nos also per stream
     */
    updateStateOfKey(noSTrace, ts);
    futureTuplesS = MeanAQPIAWJOperator::predictUnarrivedTuples(noSTrace);
    timeBreakDownPrediction += timeTrackingEnd(tt_prediction);
    //probe in R
    timeTrackingStart(tt_join);
    AbstractStateOfKeyPtr probrPtr = stateOfKeyTableR->getByKey(ts->key);
    if (probrPtr != nullptr) {
      IMAStateOfKeyPtr py = ImproveStateOfKeyTo(IMAStateOfKey, probrPtr);
      confirmedResult += py->arrivedTupleCnt;
      noS = myWindow.windowS.size() + futureTuplesS;
      noR = noR == 0 ? myWindow.windowR.size() : noR;
      selectivityS = confirmedResult * 1.0 / (noR * noS);
      noSObservation = myWindow.windowS.size();
      noRObservation = myWindow.windowR.size();
      selObservation = confirmedResult * 1.0 / (noSObservation * noRObservation);
      selPrediction = selPredictor.update(selObservation);

    }
    timeBreakDownJoin += timeTrackingEnd(tt_join);
    stateOfKey->lastUnarrivedTuples = futureTuplesS;
  }
  return true;
}

bool OoOJoin::IAWJSelOperator::feedTupleR(OoOJoin::TrackTuplePtr tr) {
  bool shouldGenWM, isInWindow;
  if (lockedByWaterMark) {
    return false;
  }
  isInWindow = myWindow.feedTupleR(tr);
  shouldGenWM = wmGen->reportTupleR(tr, 1);
  if (shouldGenWM) {
    lockedByWaterMark = true;
    WM_INFO("water mark in R");
    lastTimeOfR = UtilityFunctions::timeLastUs(timeBaseStruct);
  }
  if (isInWindow) {

    IMAStateOfKeyPtr stateOfKey;timeTrackingStart(tt_index);
    AbstractStateOfKeyPtr stateOfRKey = stateOfKeyTableR->getByKey(tr->key);

    // lastTimeR=tr->arrivalTime;
    if (stateOfRKey == nullptr) // this key does'nt exist
    {
      stateOfKey = newIMAStateOfKey();
      stateOfKey->key = tr->key;
      stateOfKeyTableR->insert(stateOfKey);
    } else {
      stateOfKey = ImproveStateOfKeyTo(IMAStateOfKey, stateOfRKey);
    }
    timeBreakDownIndex += timeTrackingEnd(tt_index);timeTrackingStart(tt_prediction);
    updateStateOfKey(stateOfKey, tr);
    double futureTuplesR = MeanAQPIAWJOperator::predictUnarrivedTuples(stateOfKey);
    /**
   * @brief make nor also per stream
   */
    updateStateOfKey(noRTrace, tr);
    futureTuplesR = MeanAQPIAWJOperator::predictUnarrivedTuples(noRTrace);
    timeBreakDownPrediction += timeTrackingEnd(tt_prediction);
    timeBreakDownPrediction += timeTrackingEnd(tt_prediction);
    //probe in S
    timeTrackingStart(tt_join);
    AbstractStateOfKeyPtr probrPtr = stateOfKeyTableS->getByKey(tr->key);
    if (probrPtr != nullptr) {
      IMAStateOfKeyPtr py = ImproveStateOfKeyTo(IMAStateOfKey, probrPtr);
      confirmedResult += py->arrivedTupleCnt;
      noS = noS == 0 ? myWindow.windowS.size() : noS;
      noR = myWindow.windowR.size() + futureTuplesR;
      selectivityR = confirmedResult * 1.0 / (noR * noS);
      noSObservation = myWindow.windowS.size();
      noRObservation = myWindow.windowR.size();
      selObservation = confirmedResult * 1.0 / (noSObservation * noRObservation);
      selPrediction = selPredictor.update(selObservation);
      /**
       * @brief rvalue observations here
       */
      noRTrace->joinedRValueSum += (int64_t) tr->payload;
      noRTrace->joinedRValueCnt++;
      noRTrace->joinedRValueAvg = noRTrace->joinedRValueSum / noRTrace->joinedRValueCnt;
      noRTrace->rvAvgPrediction = noRTrace->joinedRValuePredictor.update(noRTrace->joinedRValueAvg);
    }
    timeBreakDownJoin += timeTrackingEnd(tt_join);
    stateOfKey->lastUnarrivedTuples = futureTuplesR;
  }
  return true;
}

size_t OoOJoin::IAWJSelOperator::getResult() {
  if (joinSum) {
    return confirmedResult * noRTrace->joinedRValueAvg;
  }

  return confirmedResult;
}

size_t OoOJoin::IAWJSelOperator::getAQPResult() {
  // size_t intermediateResult = (selectivityR + selectivityS) * (noR * noS);
  //selPrediction=selObservation;
  if (noSTrace->lastArrivalTuple == nullptr) {
    return getResult();
  }
  if (noRTrace->lastArrivalTuple == nullptr) {
    return getResult();
  }

  uint64_t lastSArrival = noSTrace->lastArrivalTuple->arrivalTime + noSTrace->lastEventTuple->eventTime;
  uint64_t lastRArrival = noRTrace->lastArrivalTuple->arrivalTime + noRTrace->lastEventTuple->eventTime;
  double lastWindowArrival = (lastSArrival + lastRArrival) / 2;
  double expectedLastArrival =
      2 * myWindow.getEnd() - myWindow.getStart() + (noSTrace->arrivalSkew + noRTrace->arrivalSkew) / 2;
  size_t ru;
  double compensationWeight;
  if (lastWindowArrival > expectedLastArrival) {
    compensationWeight = 0.0;
  } else {
    if ((lastSArrival + lastRArrival) / 2 >= 2 * myWindow.getEnd() - myWindow.getStart()) {
      compensationWeight = 0.2;
    } else {
      compensationWeight = 0.5;
    }
  }
  if (joinSum) {
    auto compensated = selPrediction * noS * noR * noRTrace->rvAvgPrediction;
    double raw = getResult();
    auto temp = compensated;
    ru = (size_t) (temp * compensationWeight + raw * (1 - compensationWeight));
  } else {
    ru = selPrediction * noS * noR;
  }

  INTELLI_INFO("last arrival: " + to_string(lastWindowArrival) + ", window end"
                   + to_string(myWindow.getEnd() + noSTrace->arrivalSkew));
  return ru;
  //return ru;
}