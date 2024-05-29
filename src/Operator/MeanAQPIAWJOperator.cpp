
#include <Operator/MeanAQPIAWJOperator.h>
#include <JoinAlgos/JoinAlgoTable.h>
#include <complex>

bool OoOJoin::MeanAQPIAWJOperator::setConfig(INTELLI::ConfigMapPtr cfg) {
  if (!OoOJoin::AbstractOperator::setConfig(cfg)) {
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
  joinSum = cfg->tryU64("joinSum", 0, true);
  wmGen->setConfig(cfg);
  return true;
}

bool OoOJoin::MeanAQPIAWJOperator::start() {
  /**
  * @brief set watermark generator
  */
  // wmGen = newPeriodicalWM();

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
  return true;
}

void OoOJoin::MeanAQPIAWJOperator::conductComputation() {

}

void OoOJoin::MeanAQPIAWJOperator::updateStateOfKey(MeanStateOfKeyPtr sk, TrackTuplePtr tp) {

  double skewTp = tp->arrivalTime - tp->eventTime;
  double rateTp = tp->arrivalTime;
  rateTp = rateTp / (sk->arrivedTupleCnt + 1);
  //size_t prevUnarrived=0,currentUnarrived=0;
  sk->arrivalSkew = (1 - alphaArrivalSkew) * sk->arrivalSkew + alphaArrivalSkew * skewTp;
  sk->sigmaArrivalSkew =
      (1 - betaArrivalSkew) * sk->sigmaArrivalSkew + betaArrivalSkew * abs(sk->arrivalSkew - skewTp);
  sk->arrivedTupleCnt++;
  if (sk->lastArrivalTuple == nullptr) {
    sk->lastArrivalTuple = tp;
  } else {
    if (sk->lastArrivalTuple->arrivalTime < tp->arrivalTime) {
      sk->lastArrivalTuple = tp;
    }
  }
  if (sk->lastEventTuple == nullptr) {
    sk->lastEventTuple = tp;
  } else {
    if (sk->lastEventTuple->eventTime < tp->eventTime) {
      sk->lastEventTuple = tp;
    }
  }

}

bool OoOJoin::MeanAQPIAWJOperator::stop() {
  /**
   */
  if (lockedByWaterMark) {
    WM_INFO("early terminate by watermark, already have results");
  }
  if (!lockedByWaterMark) {
    WM_INFO("No watermark encountered, compute now");
  }
  lazyComputeOfAQP();
  timeBreakDownAll = timeTrackingEnd(timeBreakDownAll);
  size_t rLen = myWindow.windowR.size();
  NPJTuplePtr *tr = myWindow.windowR.data();
  tsType timeNow = lastTimeOfR;
  for (size_t i = 0; i < rLen; i++) {
    if (tr[i]->arrivalTime < timeNow) { tr[i]->processedTime = timeNow; }
  }
  return true;
}

bool OoOJoin::MeanAQPIAWJOperator::feedTupleS(OoOJoin::TrackTuplePtr ts) {
  bool shouldGenWM, isInWindow;
  if (lockedByWaterMark) {
    return false;
  }
  isInWindow = myWindow.feedTupleS(ts);
  shouldGenWM = wmGen->reportTupleS(ts, 1);
  if (shouldGenWM) {
    lockedByWaterMark = true;
  }
  // bool shouldGenWM;
  if (isInWindow) {
    MeanStateOfKeyPtr sk;timeTrackingStart(tt_index);
    AbstractStateOfKeyPtr skrf = stateOfKeyTableS->getByKey(ts->key);
    //lastTimeS=ts->arrivalTime;
    if (skrf == nullptr) // this key does'nt exist
    {
      sk = newMeanStateOfKey();
      sk->key = ts->key;
      stateOfKeyTableS->insert(sk);
    } else {
      sk = ImproveStateOfKeyTo(MeanStateOfKey, skrf);
    }
    timeBreakDownIndex += timeTrackingEnd(tt_index);timeTrackingStart(tt_prediction);
    updateStateOfKey(sk, ts);
    timeBreakDownPrediction += timeTrackingEnd(tt_prediction);
    // lazyComputeOfAQP();
  }
  return true;
}

bool OoOJoin::MeanAQPIAWJOperator::feedTupleR(OoOJoin::TrackTuplePtr tr) {
  bool shouldGenWM, isInWindow;
  if (lockedByWaterMark) {
    return false;
  }
  isInWindow = myWindow.feedTupleR(tr);
  shouldGenWM = wmGen->reportTupleR(tr, 1);
  if (shouldGenWM) {
    lockedByWaterMark = true;
    //return false;

  }
  // bool shouldGenWM;
  if (isInWindow) {
    MeanStateOfKeyPtr sk;timeTrackingStart(tt_index);
    AbstractStateOfKeyPtr skrf = stateOfKeyTableR->getByKey(tr->key);

    if (skrf == nullptr) // this key does'nt exist
    {
      sk = newMeanStateOfKey();
      sk->key = tr->key;
      stateOfKeyTableR->insert(sk);

    } else {
      sk = ImproveStateOfKeyTo(MeanStateOfKey, skrf);
    }
    sk->joinedRValueSum += (int64_t) tr->payload;
    sk->joinedRValueCnt++;
    sk->joinedRValueAvg = sk->joinedRValueSum / sk->joinedRValueCnt;
    sk->rvAvgPrediction = sk->joinedRValuePredictor.update(sk->joinedRValueAvg);

    timeBreakDownIndex += timeTrackingEnd(tt_index);timeTrackingStart(tt_prediction);
    updateStateOfKey(sk, tr);
    timeBreakDownPrediction += timeTrackingEnd(tt_prediction);
    //lazyComputeOfAQP();
  }
  return true;
}

size_t OoOJoin::MeanAQPIAWJOperator::getResult() {
  return confirmedResult;
  // return confirmedResult;
}

double OoOJoin::MeanAQPIAWJOperator::predictUnarrivedTuples(MeanStateOfKeyPtr px) {
  tsType lastTime = px->lastArrivalTuple->arrivalTime;
  double avgSkew = px->arrivalSkew;
  double goThroughTime = lastTime - avgSkew - myWindow.getStart();
  double futureTime = myWindow.getEnd() + avgSkew - lastTime;
  double ratio = futureTime / (goThroughTime > 0 ? goThroughTime : 1); // prevent division by zero
  ratio = std::min(ratio, max_ratio); // Limit the ratio to the maximum allowed value

  // Apply adaptive EWMA
  double futureTuple = ratio * px->arrivedTupleCnt;
  if (futureTuple < 0) {
    futureTuple = 0;
  }

  // Update filter parameters
  if (goThroughTime == 0)return futureTuple;
  double arrivalRate = px->arrivedTupleCnt / goThroughTime;
  if (arrivalRate > 0) {
    double predictedArrivalRate = futureTuple / futureTime;

    double ratio = predictedArrivalRate / arrivalRate;
    if (ratio > 1.2) {
      alpha = std::min(alpha + 0.5, MAX_ALPHA);
    } else if (ratio < 0.3) {
      alpha = std::max(alpha - 0.5, MIN_ALPHA);
    }

    max_ratio = std::sqrt(predictedArrivalRate / arrivalRate);
    max_ratio = std::min(std::max(max_ratio, MIN_MAX_RATIO), MAX_MAX_RATIO);

  }

  return futureTuple;
}

void OoOJoin::MeanAQPIAWJOperator::lazyComputeOfAQP() {
  AbstractStateOfKeyPtr probrPtr = nullptr;
  intermediateResult = 0;timeTrackingStart(tt_join);
  for (size_t i = 0; i < stateOfKeyTableR->buckets.size(); i++) {
    for (auto iter : stateOfKeyTableR->buckets[i]) {
      if (iter != nullptr) {
        MeanStateOfKeyPtr px = ImproveStateOfKeyTo(MeanStateOfKey, iter);
        probrPtr = stateOfKeyTableS->getByKey(px->key);
        if (probrPtr != nullptr) {

          double unarrivedS = predictUnarrivedTuples(px);
          MeanStateOfKeyPtr py = ImproveStateOfKeyTo(MeanStateOfKey, probrPtr);
          double unarrivedR = predictUnarrivedTuples(py);
          if (joinSum) {
            uint64_t cIR = (px->arrivedTupleCnt) * (py->arrivedTupleCnt) * px->joinedRValueAvg;
            uint64_t
                pIR = (px->arrivedTupleCnt + unarrivedS) * (py->arrivedTupleCnt + unarrivedR) * px->rvAvgPrediction;
            intermediateResult += (cIR + pIR) / 2;
            confirmedResult += cIR;
          } else {
            intermediateResult += (px->arrivedTupleCnt + unarrivedS) * (py->arrivedTupleCnt + unarrivedR);
            confirmedResult += (px->arrivedTupleCnt) * (py->arrivedTupleCnt);
          }

        }
      }
    }
  }
  timeBreakDownJoin += timeTrackingEnd(tt_join);
  lastTimeOfR = UtilityFunctions::timeLastUs(timeBaseStruct);
}

size_t OoOJoin::MeanAQPIAWJOperator::getAQPResult() {
  return intermediateResult;
}

ConfigMapPtr OoOJoin::MeanAQPIAWJOperator::getTimeBreakDown() {
  ConfigMapPtr ru = newConfigMap();
  ru->edit("index", (uint64_t) timeBreakDownIndex);
  ru->edit("prediction", (uint64_t) timeBreakDownPrediction);
  ru->edit("join", (uint64_t) timeBreakDownJoin);
  return ru;
}
