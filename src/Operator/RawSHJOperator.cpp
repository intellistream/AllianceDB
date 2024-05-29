
#include <Operator/RawSHJOperator.h>
#include <JoinAlgos/JoinAlgoTable.h>

bool OoOJoin::RawSHJOperator::setConfig(INTELLI::ConfigMapPtr cfg) {
  if (!OoOJoin::MeanAQPIAWJOperator::setConfig(cfg)) {
    return false;
  }
  config = cfg;

  joinSum = cfg->tryU64("joinSum", 0, true);

  return true;
}

bool OoOJoin::RawSHJOperator::start() {
  /**
  * @brief set watermark generator
  */
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
  return true;
}

void OoOJoin::RawSHJOperator::conductComputation() {

}

bool OoOJoin::RawSHJOperator::stop() {
  if (lockedByWaterMark) {
    WM_INFO("early terminate by watermark, already have results");
  }
  if (!lockedByWaterMark) {
    WM_INFO("No watermark encountered, compute now");
  }
  timeBreakDownAll = timeTrackingEnd(timeBreakDownAll);

  size_t rLen = myWindow.windowR.size();
  NPJTuplePtr *tr = myWindow.windowR.data();
  tsType timeNow = lastTimeOfR;
  for (size_t i = 0; i < rLen; i++) {
    if (tr[i]->arrivalTime < timeNow) {
      tr[i]->processedTime = timeNow;
    }
  }
  return true;
}

bool OoOJoin::RawSHJOperator::feedTupleS(OoOJoin::TrackTuplePtr ts) {
  bool shouldGenWM, isInWindow;
  if (lockedByWaterMark) {
    return false;
  }
  isInWindow = myWindow.feedTupleS(ts);
  if (ts->arrivalTime > myWindow.getEnd()) {
    shouldGenWM = true;
    lastTimeOfR = UtilityFunctions::timeLastUs(timeBaseStruct);
  }
  if (shouldGenWM) {
    lockedByWaterMark = true;
    WM_INFO("water mark in S");
  }
  if (isInWindow) {
    MeanStateOfKeyPtr stateOfKey;
    /**
     * @brief First get the index of hash table
     */
    timeTrackingStart(tt_index);
    AbstractStateOfKeyPtr stateOfSKey = stateOfKeyTableS->getByKey(ts->key);
    if (stateOfSKey == nullptr) // this key doesn't exist
    {
      stateOfKey = newMeanStateOfKey();
      stateOfKey->key = ts->key;
      stateOfKeyTableS->insert(stateOfKey);
    } else {
      stateOfKey = ImproveStateOfKeyTo(MeanStateOfKey, stateOfSKey);
    }
    timeBreakDownIndex += timeTrackingEnd(tt_index);

    /**
     *
     */
    timeTrackingStart(tt_prediction);
    updateStateOfKey(stateOfKey, ts);
    timeBreakDownPrediction += timeTrackingEnd(tt_prediction);
    //probe in R
    timeTrackingStart(tt_join);
    AbstractStateOfKeyPtr probrPtr = stateOfKeyTableR->getByKey(ts->key);
    if (probrPtr != nullptr) {
      MeanStateOfKeyPtr py = ImproveStateOfKeyTo(MeanStateOfKey, probrPtr);
      if (joinSum) { /**
         * @brief we are dealing with r.value, so here is py->xxx
         */
        double tc = (int64_t) py->arrivedTupleCnt * py->joinedRValueAvg;
        confirmedResult += (uint64_t) tc;

      } else {
        confirmedResult += py->arrivedTupleCnt;
      }

    }
    timeBreakDownJoin += timeTrackingEnd(tt_join);

  }
  return true;
}
bool OoOJoin::RawSHJOperator::feedTupleR(OoOJoin::TrackTuplePtr tr) {
  bool shouldGenWM, isInWindow;
  if (lockedByWaterMark) {
    return false;
  }
  isInWindow = myWindow.feedTupleR(tr);
  if (tr->arrivalTime > myWindow.getEnd()) {
    shouldGenWM = true;
    lastTimeOfR = UtilityFunctions::timeLastUs(timeBaseStruct);
  }
  if (shouldGenWM) {
    lockedByWaterMark = true;
    WM_INFO("water mark in R");

  }
  if (isInWindow) {

    MeanStateOfKeyPtr stateOfKey;timeTrackingStart(tt_index);
    AbstractStateOfKeyPtr stateOfRKey = stateOfKeyTableR->getByKey(tr->key);


    // lastTimeR=tr->arrivalTime;
    if (stateOfRKey == nullptr) // this key does'nt exist
    {
      stateOfKey = newMeanStateOfKey();
      stateOfKey->key = tr->key;
      stateOfKeyTableR->insert(stateOfKey);
    } else {
      stateOfKey = ImproveStateOfKeyTo(MeanStateOfKey, stateOfRKey);
    }

    timeBreakDownIndex += timeTrackingEnd(tt_index);timeTrackingStart(tt_prediction);
    updateStateOfKey(stateOfKey, tr);
    timeBreakDownPrediction += timeTrackingEnd(tt_prediction);
    //probe in S
    timeTrackingStart(tt_join);
    AbstractStateOfKeyPtr probrPtr = stateOfKeyTableS->getByKey(tr->key);
    if (probrPtr != nullptr) {
      /**
            * @brief rvalue estimation
        */
      stateOfKey->joinedRValueSum += (int64_t) tr->payload;
      stateOfKey->joinedRValueCnt++;
      stateOfKey->joinedRValueAvg = stateOfKey->joinedRValueSum / stateOfKey->joinedRValueCnt;
      stateOfKey->rvAvgPrediction = stateOfKey->joinedRValuePredictor.update(stateOfKey->joinedRValueAvg);
      MeanStateOfKeyPtr py = ImproveStateOfKeyTo(MeanStateOfKey, probrPtr);
      if (joinSum) { /**
         * @brief we are dealing with r.value, so here is stateOfKey->xxx
         */

        double tc = (int64_t) py->arrivedTupleCnt * stateOfKey->joinedRValueAvg;
        confirmedResult += (uint64_t) tc;

      } else {
        confirmedResult += py->arrivedTupleCnt;
      }

    }
    timeBreakDownJoin += timeTrackingEnd(tt_join);

  }
  return true;
}

size_t OoOJoin::RawSHJOperator::getResult() {

  return confirmedResult;
}

size_t OoOJoin::RawSHJOperator::getAQPResult() {
  return getResult();
}