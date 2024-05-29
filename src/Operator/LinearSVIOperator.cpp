
#include <Operator/LinearSVIOperator.h>
#include <JoinAlgos/JoinAlgoTable.h>
#include <filesystem>

void OoOJoin::LinearSVIOperator::saveAllTensors() {
  if (aiModeEnum == 0) {
    /**
     * @brief 1. save the selectivity tensor
     */
    uint64_t xCols = streamStatisics.sviSelectivity.getXDimension();
    /**
     * @brief 2. set all final observations
     */
    streamStatisics.selObservations.setFinalObservation(streamStatisics.selectivity);
    streamStatisics.sSkewObservations.setFinalObservation(streamStatisics.sSkew);
    streamStatisics.sRateObservations.setFinalObservation(streamStatisics.sRate);
    streamStatisics.rSkewObservations.setFinalObservation(streamStatisics.rSkew);
    streamStatisics.rRateObservations.setFinalObservation(streamStatisics.rRate);
    /**
     * @brief 3. save all tensors
     */
    if (appendSel) {
      streamStatisics.selObservations.saveXYTensors2Files("torchscripts/" + ptPrefix + "/" + "tensor_selectivity",
                                                          xCols);
    }
    if (appendSkew) {
      streamStatisics.sSkewObservations.saveXYTensors2Files("torchscripts/" + ptPrefix + "/" + "tensor_sSkew", xCols);
      streamStatisics.rSkewObservations.saveXYTensors2Files("torchscripts/" + ptPrefix + "/" + "tensor_rSkew", xCols);
    }
    if (appendRate) {
      streamStatisics.sRateObservations.saveXYTensors2Files("torchscripts/" + ptPrefix + "/" + "tensor_sRate", xCols);
      streamStatisics.rRateObservations.saveXYTensors2Files("torchscripts/" + ptPrefix + "/" + "tensor_rRate", xCols);
    }
    if (exitAfterPretrain) {
      INTELLI_INFO("The tensors are saved, exit 0");
    }


    //exit(0);
    // std::cout<<streamStatisics.selectivityTensorY<<std::endl;
  }
}
void OoOJoin::LinearSVIOperator::preparePretrain() {
  /**
   * @brief 1. init all observation tensors
   */
  streamStatisics.selObservations.noNormalize = true;
  streamStatisics.sRateObservations.noNormalize = true;
  streamStatisics.rRateObservations.noNormalize = true;
  streamStatisics.selObservations.initObservationBuffer(selLen);
  streamStatisics.sSkewObservations.initObservationBuffer(selLen);
  streamStatisics.rSkewObservations.initObservationBuffer(selLen);
  streamStatisics.sRateObservations.initObservationBuffer(selLen);
  streamStatisics.rRateObservations.initObservationBuffer(selLen);
}
void OoOJoin::LinearSVIOperator::prepareInference() {
  torch::manual_seed(114514);
  /**
   * @brief 1. init all observation tensors
   */
  streamStatisics.selObservations.noNormalize = true;
  streamStatisics.sRateObservations.noNormalize = true;
  streamStatisics.rRateObservations.noNormalize = true;
  uint64_t xCols = streamStatisics.sviSelectivity.getXDimension();
  streamStatisics.selObservations.initObservationBuffer(xCols);
  streamStatisics.sSkewObservations.initObservationBuffer(xCols);
  streamStatisics.rSkewObservations.initObservationBuffer(xCols);
  streamStatisics.sRateObservations.initObservationBuffer(xCols);
  streamStatisics.rRateObservations.initObservationBuffer(xCols);
  /**
   * @brief 2. try to get the scaling factor
   */
  streamStatisics.selObservations.tryScalingFactor("torchscripts/" + ptPrefix + "/" + "tensor_selectivity");
  streamStatisics.sRateObservations.tryScalingFactor("torchscripts/" + ptPrefix + "/" + "tensor_sRate");
  streamStatisics.rRateObservations.tryScalingFactor("torchscripts/" + ptPrefix + "/" + "tensor_rRate");
  /**
   * @brief 3. set the small learning rate
   */
  streamStatisics.sviSelectivity.resetLearningRate(0.001);
  streamStatisics.sviSRate.resetLearningRate(0.01);
  streamStatisics.sviRRate.resetLearningRate(0.01);
}

bool OoOJoin::LinearSVIOperator::setConfig(INTELLI::ConfigMapPtr cfg) {
  if (!OoOJoin::MeanAQPIAWJOperator::setConfig(cfg)) {
    return false;
  }
  std::string wmTag = config->tryString("wmTag", "arrival", true);
  aiMode = config->tryString("aiMode", "pretrain", true);
  ptPrefix = config->tryString("ptPrefix", "linearSVI", true);
  ptPrefixSel = config->tryString("ptPrefixSel", ptPrefix, true);
  ptPrefixSRate = config->tryString("ptPrefixSRate", ptPrefix, true);
  ptPrefixRRate = config->tryString("ptPrefixRRate", ptPrefix, true);
  appendSel = config->tryU64("appendSel", 0, true);
  appendSkew = config->tryU64("appendSkew", 0, true);
  appendRate = config->tryU64("appendRate", 0, true);
  exitAfterPretrain = config->tryU64("exitAfterPretrain", 1, true);

  streamStatisics.sviSelectivity.loadModule("torchscripts/" + ptPrefixSel + "/" + ptPrefixSel + "_selectivity.pt");
  streamStatisics.sviSRate.loadModule("torchscripts/" + ptPrefixSRate + "/" + ptPrefixSRate + "_sRate.pt");
  streamStatisics.sviRRate.loadModule("torchscripts/" + ptPrefixRRate + "/" + ptPrefixRRate + "_rRate.pt");
  // INTELLI_WARNING("The dimension of DAN is "+to_string(streamStatisics.sviSelectivity.getXDimension()));
  if (aiMode == "pretrain") {
    aiModeEnum = 0;
    /**
     * @brief chack which tensors to be appended
     */
    if (appendSel || appendSkew || appendRate) {
      INTELLI_WARNING("The tensors in file system will be overwrite by me because you asked me to do so.");
      selLen = config->tryU64("selLen", 0, true);
      if (selLen == 0 && aiModeEnum == 0) {
        INTELLI_ERROR("Invalid pretrain settings, abort");
        exit(0);
      }
    }
    preparePretrain();
  } else if (aiMode == "continual_learning") {
    aiModeEnum = 1;
    prepareInference();
  } else if (aiMode == "inference") {
    aiModeEnum = 2;
    prepareInference();
  } else {
    aiModeEnum = 255;
    INTELLI_WARNING("This mode is N.A.");
    prepareInference();
  }

  WMTablePtr wmTable = newWMTable();
  wmGen = wmTable->findWM(wmTag);
  if (wmGen == nullptr) {
    INTELLI_ERROR("NO such a watermarker named [" + wmTag + "]");
    return false;
  }
  INTELLI_INFO("Using the watermarker named [" + wmTag + "]");
  wmGen->setConfig(config);
  return true;
}

bool OoOJoin::LinearSVIOperator::start() {
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
  streamStatisics.reset();

  return true;
}

void OoOJoin::LinearSVIOperator::conductComputation() {

}

bool OoOJoin::LinearSVIOperator::stop() {
  if (lockedByWaterMark) {
    WM_INFO("early terminate by watermark, already have results");
  }
  if (!lockedByWaterMark) {
    WM_INFO("No watermark encountered, compute now");
  }

  return true;
}

bool OoOJoin::LinearSVIOperator::feedTupleS(OoOJoin::TrackTuplePtr ts) {
  bool shouldGenWM, isInWindow;
  if (lockedByWaterMark) {
    return false;
  }
  isInWindow = myWindow.feedTupleS(ts);
  shouldGenWM = wmGen->reportTupleS(ts, 1);
  if (shouldGenWM) {
    lockedByWaterMark = true;
    WM_INFO("water mark in S");
    endOfWindow();
  }
  if (isInWindow) {
    LinearSVIStateOfKeyPtr stateOfKey;
    /**
     * @brief update the stream statistics
     *
     */
    streamStatisics.encounterSTuple(ts);
    if (aiModeEnum != 255) {
      streamStatisics.sSkewObservations.appendX(streamStatisics.sSkew);
      streamStatisics.sRateObservations.appendX(streamStatisics.sRate);

      updateEstimation(streamStatisics.sRateObservations, streamStatisics.sviSRate);

    }
    /**
     * @brief First get the index of hash table
     */
    timeTrackingStart(tt_index);
    AbstractStateOfKeyPtr stateOfSKey = stateOfKeyTableS->getByKey(ts->key);
    if (stateOfSKey == nullptr) // this key doesn't exist
    {
      stateOfKey = newLinearSVIStateOfKey();
      stateOfKey->key = ts->key;
      stateOfKeyTableS->insert(stateOfKey);
    } else {
      stateOfKey = ImproveStateOfKeyTo(LinearSVIStateOfKey, stateOfSKey);
    }
    timeBreakDownIndex += timeTrackingEnd(tt_index);
    /**
     *
     */
    timeTrackingStart(tt_prediction);
    updateStateOfKey(stateOfKey, ts);
    double futureTuplesS = MeanAQPIAWJOperator::predictUnarrivedTuples(stateOfKey);
    timeBreakDownPrediction += timeTrackingEnd(tt_prediction);
    //probe in R
    timeTrackingStart(tt_join);
    AbstractStateOfKeyPtr probrPtr = stateOfKeyTableR->getByKey(ts->key);
    if (probrPtr != nullptr) {
      LinearSVIStateOfKeyPtr py = ImproveStateOfKeyTo(LinearSVIStateOfKey, probrPtr);
      confirmedResult += py->arrivedTupleCnt;
      /**
       * @brief update selectivity here
       */
      streamStatisics.updateSelectivity(confirmedResult);
      //streamStatisics.selObservations.appendX(streamStatisics.selectivity);
      if (aiModeEnum != 255) {
        streamStatisics.selObservations.setFinalObservation(streamStatisics.selectivity);
        streamStatisics.selObservations.appendX(streamStatisics.selectivity);
        updateEstimation(streamStatisics.selObservations, streamStatisics.sviSelectivity);
      }

//            intermediateResult += py->arrivedTupleCnt;
      intermediateResult += (futureTuplesS + stateOfKey->arrivedTupleCnt) *
          (py->lastUnarrivedTuples + py->arrivedTupleCnt) -
          (stateOfKey->arrivedTupleCnt + stateOfKey->lastUnarrivedTuples - 1) *
              (py->lastUnarrivedTuples + py->arrivedTupleCnt);
    }
    timeBreakDownJoin += timeTrackingEnd(tt_join);
    stateOfKey->lastUnarrivedTuples = futureTuplesS;
    lastTimeOfR = UtilityFunctions::timeLastUs(timeBaseStruct);
  }
  return true;
}
void OoOJoin::LinearSVIOperator::updateEstimation(ObservationGroup &observation, TROCHPACK_SVI::LinearSVI &estimator) {

  if (aiModeEnum == 0) {
    return;
  }
  if (observation.buffFull) {
    observation.tempXTensor = observation.xTensor.clone();
    if (aiModeEnum == 2) {
      return;
    }

    if (observation.fullCnt) {
      auto tx = (observation.tempXTensor).clone();
      float oldMu = estimator.resultMu;
      estimator.runForward(tx);
      float newMu = estimator.resultMu;
      float loss = 1.0;
      uint16_t iterations = 0;
      if (std::abs(oldMu - newMu) > 0.1 * std::abs(oldMu)) {
        while (loss > -0.8 && iterations < 10) {
          estimator.learnStep(tx);
          loss = estimator.resultLoss;
          iterations++;
          INTELLI_INFO("Run continue learning, iteration=" + to_string(iterations) + "loss =" + to_string(loss));
        }

        estimator.runForward(tx);
      }
    }
  }
  //INTELLI_INFO("Run continue learning");
}
void OoOJoin::LinearSVIOperator::endOfWindow() {
  if (aiModeEnum == 0) {
    std::cout << streamStatisics.reportStr() << endl;
    std::cout << "joined " + to_string(confirmedResult) << endl;
    saveAllTensors();
    return;
  }
  if (aiModeEnum == 2 || aiModeEnum == 1) { /**
    * @brief estimate sel
    */
    tsType nnBegin = UtilityFunctions::timeLastUs(timeBaseStruct);
    streamStatisics.sviSelectivity.runForward(
        streamStatisics.selObservations.tempXTensor / (streamStatisics.selObservations.scalingFactor));
    float selMu = streamStatisics.sviSelectivity.resultMu;
    selMu = selMu * streamStatisics.selObservations.scalingFactor;
    INTELLI_INFO("The estimated selectivity is " + to_string(selMu));

    /**
    * @brief do some continual learning
    */
    /*if(aiModeEnum==1)
    {
      updateEstimation(streamStatisics.sRateObservations, streamStatisics.sviSRate);
      updateEstimation(streamStatisics.rRateObservations, streamStatisics.sviRRate);
    }*/
    /* */
    /**
     * @brief forward
     */
    streamStatisics.sviSRate.runForward(
        streamStatisics.sRateObservations.tempXTensor);
    float sRateMu = streamStatisics.sviSRate.resultMu;
    sRateMu = sRateMu;
    INTELLI_INFO("The estimated sRate is " + to_string(sRateMu));
    //streamStatisics.rRateObservations.scalingFactor=1.0;
    // streamStatisics.sviSRate.learnStep(streamStatisics.sRateObservations.xTensor);
    streamStatisics.sviRRate.runForward(
        streamStatisics.rRateObservations.tempXTensor);
    float rRateMu = streamStatisics.sviRRate.resultMu;
    rRateMu = rRateMu;
    INTELLI_INFO("The estimated rRate is " + to_string(rRateMu));
    float sCnt = windowLen * rRateMu / 1000.0;
    float rCnt = windowLen * sRateMu / 1000.0;
    intermediateResult = sCnt * rCnt * selMu;
    timeBreakDownAll = timeTrackingEnd(timeBreakDownAll);

    size_t rLen = myWindow.windowR.size();
    NPJTuplePtr *tr = myWindow.windowR.data();
    tsType timeNow = UtilityFunctions::timeLastUs(timeBaseStruct);
    for (size_t i = 0; i < rLen; i++) {
      if (tr[i]->arrivalTime < timeNow) {
        tr[i]->processedTime = timeNow;
      }
    }
    INTELLI_WARNING("SVI takes " + to_string((timeNow - nnBegin) / 1000));
    // exit(0);
  }

}
bool OoOJoin::LinearSVIOperator::feedTupleR(OoOJoin::TrackTuplePtr tr) {
  bool shouldGenWM, isInWindow;
  if (lockedByWaterMark) {
    return false;
  }
  isInWindow = myWindow.feedTupleR(tr);
  shouldGenWM = wmGen->reportTupleR(tr, 1);
  if (shouldGenWM) {
    lockedByWaterMark = true;
    WM_INFO("water mark in R");
    endOfWindow();
  }
  if (isInWindow) {
    /**
   * @brief update the stream statistics
   *
   */
    streamStatisics.encounterRTuple(tr);
    if (aiModeEnum != 255) {
      streamStatisics.rSkewObservations.appendX(streamStatisics.rSkew);
      streamStatisics.rRateObservations.appendX(streamStatisics.rRate);

      updateEstimation(streamStatisics.rRateObservations, streamStatisics.sviRRate);

    }
    LinearSVIStateOfKeyPtr stateOfKey;timeTrackingStart(tt_index);
    AbstractStateOfKeyPtr stateOfRKey = stateOfKeyTableR->getByKey(tr->key);

    // lastTimeR=tr->arrivalTime;
    if (stateOfRKey == nullptr) // this key does'nt exist
    {
      stateOfKey = newLinearSVIStateOfKey();
      stateOfKey->key = tr->key;
      stateOfKeyTableR->insert(stateOfKey);
    } else {
      stateOfKey = ImproveStateOfKeyTo(LinearSVIStateOfKey, stateOfRKey);
    }
    timeBreakDownIndex += timeTrackingEnd(tt_index);timeTrackingStart(tt_prediction);
    updateStateOfKey(stateOfKey, tr);
    double futureTuplesR = MeanAQPIAWJOperator::predictUnarrivedTuples(stateOfKey);
    timeBreakDownPrediction += timeTrackingEnd(tt_prediction);
    //probe in S
    timeTrackingStart(tt_join);
    AbstractStateOfKeyPtr probrPtr = stateOfKeyTableS->getByKey(tr->key);
    if (probrPtr != nullptr) {
      LinearSVIStateOfKeyPtr py = ImproveStateOfKeyTo(LinearSVIStateOfKey, probrPtr);
      confirmedResult += py->arrivedTupleCnt;
      /**
     * @brief update selectivity here
     */
      /**
        * @brief update selectivity here
        */
      streamStatisics.updateSelectivity(confirmedResult);
      //streamStatisics.selObservations.appendX(streamStatisics.selectivity);
      if (aiModeEnum != 255) {
        streamStatisics.selObservations.setFinalObservation(streamStatisics.selectivity);
        streamStatisics.selObservations.appendX(streamStatisics.selectivity);
        updateEstimation(streamStatisics.selObservations, streamStatisics.sviSelectivity);
      }
      //streamStatisics.updateSelectivity(confirmedResult);
      // appendSelectivityTensorX();
//            intermediateResult += py->arrivedTupleCnt;
      intermediateResult += (futureTuplesR + stateOfKey->arrivedTupleCnt) *
          (py->lastUnarrivedTuples + py->arrivedTupleCnt) -
          (stateOfKey->arrivedTupleCnt + stateOfKey->lastUnarrivedTuples - 1) *
              (py->lastUnarrivedTuples + py->arrivedTupleCnt);
    }
    timeBreakDownJoin += timeTrackingEnd(tt_join);
    stateOfKey->lastUnarrivedTuples = futureTuplesR;
    lastTimeOfR = UtilityFunctions::timeLastUs(timeBaseStruct);
  }
  return true;
}

size_t OoOJoin::LinearSVIOperator::getResult() {

  return confirmedResult;
}

size_t OoOJoin::LinearSVIOperator::getAQPResult() {
// return (intermediateResult+confirmedResult)/2;
/*if(aiModeEnum==1)
{
  return intermediateResult;
}*/
  return intermediateResult;
  //return (intermediateResult+confirmedResult)/2;
}