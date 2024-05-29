//
// Created by 86183 on 2023/1/8.
//



#include <future>
#include <utility>
#include <Operator/MSWJ/Operator/StreamOperator.h>
#include <JoinAlgos/AbstractJoinAlgo.h>
#include <WaterMarker/WMTable.h>

using namespace MSWJ;

StreamOperator::StreamOperator(TupleProductivityProfiler *profiler, INTELLI::ConfigMapPtr config) :
    config(std::move(config)), productivityProfiler(profiler) {
}

bool StreamOperator::start() {
  /**
  * @brief set watermark generator
  */
  //wmGen = newPeriodicalWM();
  wmGen->setConfig(config);
  mswjCompensation = config->tryU64("mswjCompensation", 0, true);
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

bool StreamOperator::stop() {
  /**
   */
  if (lockedByWaterMark) {
    WM_INFO("early terminate by watermark, already have results");
  }
  if (!lockedByWaterMark) {
    WM_INFO("No watermark encountered, compute now");
  }
  timeBreakDownAll = timeTrackingEnd(timeBreakDownAll);
  //lazyComputeOfAQP();
  size_t rLen = myWindow.windowR.size();
  tsType timeNow = lastTimeOfR;
  NPJTuplePtr *tr = myWindow.windowR.data();
  for (size_t i = 0; i < rLen; i++) {
    if (tr[i]->arrivalTime < timeNow) { tr[i]->processedTime = timeNow; }
  }
  return true;
}

//auto StreamOperator::mswjExecution(Tuple *join_tuple) -> void {
//    Tuple tuple = *join_tuple;
//    int stream_id = tuple.streamId;
//    //计算Di
//    int delay = tuple.delay;
//
//    //计算cross-join的结果大小
//    int cross_join = 1;
//
//    if (tuple.ts >= T_op_) {
//        T_op_ = tuple.ts;
//
//        for (int i = 0; i < window_map_.size(); i++) {
//            if (window_map_[i].empty()) {
//                continue;
//            }
//            //统计window内元组数量数据
//            productivityProfiler->addJoinRecord(stream_id, window_map_[i].size());
//
//            if (i == stream_id) {
//                continue;
//            }
//
//            for (auto iter = window_map_[i].begin(); iter != window_map_[i].end();) {
//                Tuple tuple_j = *iter;
//                cross_join++;
//                if (tuple_j.ts < tuple.ts - window_map_[i].size()) {
//                    window_map_[i].erase(iter++);
//                    cross_join--;
//                } else {
//                    iter++;
//                }
//            }
//
//        }
//
//        //更新cross_join_map
//
//        productivityProfiler->updateCrossJoin(getD(delay), cross_join);
//
//        //连接
//        std::unordered_map<int, std::vector<Tuple>> tempJoinMap;
//        int res = 1;
//        for (int i = 0; i < window_map_.size(); i++) {
//            if (window_map_[i].empty()) {
//                continue;
//            }
//
//            if (i == stream_id) {
//                continue;
//            }
//            while (!window_map_[i].empty()) {
//                Tuple tuple_j = window_map_[i].front();
//                window_map_[i].pop_front();
//                if (can_join_(tuple, tuple_j)) {
//                    //时间戳定义为ei.ts
//                    tuple_j.ts = tuple.ts;
//                    tempJoinMap[i].push_back(tuple_j);
//                }
//            }
//        }
//
//        int tempCount = 1;
//        //统计res,先统计二路join
//        for (const auto &it: tempJoinMap) {
//            tempCount *= it.second.size();
//            for (auto l: it.second) {
//                std::vector<Tuple> tempVec;
//                tempVec.push_back(tuple);
//                tempVec.push_back(l);
//                result_.push(tempVec);
//            }
//        }
//
//        res += tempCount;
//
//        joinResultCount_ += res;
//
//        //更新join result map
//        productivityProfiler->updateJoinRes(getD(delay), res);
//
//        window_map_[stream_id].push_back(tuple);
//    } else if (tuple.ts > T_op_ - window_map_[stream_id].size()) {
//        window_map_[stream_id].push_back(tuple);
//    }
//}


auto StreamOperator::mswjExecution(const TrackTuplePtr &trackTuple) -> bool {
  //indicate system is doing unit test
  if (wmGen == nullptr) {
    return false;
  }

  int stream_id = trackTuple->streamId;

  bool shouldGenWM;
  if (lockedByWaterMark) {
    return false;
  }

  if (stream_id == 1) {
    sIsInWindow = myWindow.feedTupleS(trackTuple);
    shouldGenWM = wmGen->reportTupleS(trackTuple, 1);
  } else {
    rIsInWindow = myWindow.feedTupleR(trackTuple);
    shouldGenWM = wmGen->reportTupleR(trackTuple, 1);
  }

  if (shouldGenWM) {
    lockedByWaterMark = true;
    WM_INFO("water mark in S");
    //  return false;
  }
  // bool shouldGenWM;
  if (stream_id == 1) {
    if (sIsInWindow) {
      productivityProfiler->updateCrossJoin(get_D(trackTuple->delay),
                                            myWindow.windowR.size() * myWindow.windowS.size());

      MSWJStateOfKeyPtr sk;
      /**
       * @brief First get the index of hash table
       */
      timeTrackingStart(tt_index);
      AbstractStateOfKeyPtr skrf = stateOfKeyTableS->getByKey(trackTuple->key);
      if (skrf == nullptr) // this key does'nt exist
      {
        sk = newMSWJStateOfKey();
        sk->key = trackTuple->key;
        sk->value = trackTuple->payload;
        stateOfKeyTableS->insert(sk);
      } else {
        sk = ImproveStateOfKeyTo(MSWJStateOfKey, skrf);
      }
      timeBreakDownIndex += timeTrackingEnd(tt_index);
      /**
       *
       */
      timeTrackingStart(tt_prediction);
      updateStateOfKey(sk, trackTuple);
      double futureTuplesS = MeanAQPIAWJOperator::predictUnarrivedTuples(sk);
      timeBreakDownPrediction += timeTrackingEnd(tt_prediction);
      //probe in R
      timeTrackingStart(tt_join);
      AbstractStateOfKeyPtr probrPtr = stateOfKeyTableR->getByKey(trackTuple->key);

      if (probrPtr != nullptr) {
        // std::stringif(joinSum) compensation = config->tryString("compensation", "false");

        MSWJStateOfKeyPtr py = ImproveStateOfKeyTo(MSWJStateOfKey, probrPtr);
        confirmedResult += py->arrivedTupleCnt;
        if (joinSum) {
          double tc = (int64_t) py->arrivedTupleCnt * py->joinedRValueAvg;
          confirmedResultJS += (uint64_t) tc;
        }
        if (!mswjCompensation) {
          intermediateResult += py->arrivedTupleCnt;

          if (joinConfirmCountMap.find(sk->key) == joinConfirmCountMap.end()) {
            joinConfirmCountMap[sk->key] = {{0, 0}, 0};
          }
          if (joinAQPCountMap.find(sk->key) == joinAQPCountMap.end()) {
            joinAQPCountMap[sk->key] = {{0, 0}, 0};
          }

          joinAQPCountMap[sk->key] = {{joinAQPCountMap[sk->key].first.first + py->arrivedTupleCnt,
                                       joinAQPCountMap[sk->key].first.second + sk->value},
                                      joinAQPCountMap[sk->key].second + 1};
          joinConfirmCountMap[sk->key] = {{joinConfirmCountMap[sk->key].first.first + py->arrivedTupleCnt,
                                           joinConfirmCountMap[sk->key].first.second + sk->value},
                                          joinConfirmCountMap[sk->key].second + 1};
          productivityProfiler->updateJoinRes(get_D(trackTuple->delay),
                                              py->arrivedTupleCnt);
        } else {
          size_t resultPlus = (futureTuplesS + sk->arrivedTupleCnt) *
              (py->lastUnarrivedTuples + py->arrivedTupleCnt) -
              (sk->arrivedTupleCnt + sk->lastUnarrivedTuples - 1) *
                  (py->lastUnarrivedTuples + py->arrivedTupleCnt);
          intermediateResult += resultPlus;

          if (joinConfirmCountMap.find(sk->key) == joinConfirmCountMap.end()) {
            joinConfirmCountMap[sk->key] = {{0, 0}, 0};
          }
          if (joinAQPCountMap.find(sk->key) == joinAQPCountMap.end()) {
            joinAQPCountMap[sk->key] = {{0, 0}, 0};
          }

          joinAQPCountMap[sk->key] = {{joinAQPCountMap[sk->key].first.first + resultPlus,
                                       joinAQPCountMap[sk->key].first.second + sk->value},
                                      joinAQPCountMap[sk->key].second + 1};
          joinConfirmCountMap[sk->key] = {{joinConfirmCountMap[sk->key].first.first + py->arrivedTupleCnt,
                                           joinConfirmCountMap[sk->key].first.second + sk->value},
                                          joinConfirmCountMap[sk->key].second + 1};
          productivityProfiler->updateJoinRes(get_D(trackTuple->delay),
                                              (futureTuplesS + sk->arrivedTupleCnt) *
                                                  (py->lastUnarrivedTuples + py->arrivedTupleCnt) -
                                                  (sk->arrivedTupleCnt + sk->lastUnarrivedTuples - 1) *
                                                      (py->lastUnarrivedTuples + py->arrivedTupleCnt));

        }
      }
      timeBreakDownJoin += timeTrackingEnd(tt_join);
      //sk->lastEstimateAllTuples=futureTuplesS+sk->arrivedTupleCnt;
      sk->lastUnarrivedTuples = futureTuplesS;
      lastTimeOfR = UtilityFunctions::timeLastUs(timeBaseStruct);
    }
  } else {
    if (rIsInWindow) {
      productivityProfiler->updateCrossJoin(get_D(trackTuple->delay),
                                            myWindow.windowR.size() * myWindow.windowS.size());

      MSWJStateOfKeyPtr sk;timeTrackingStart(tt_index);
      AbstractStateOfKeyPtr skrf = stateOfKeyTableR->getByKey(trackTuple->key);

      // lastTimeR=tr->arrivalTime;
      if (skrf == nullptr) // this key does'nt exist
      {
        sk = newMSWJStateOfKey();
        sk->key = trackTuple->key;
        sk->value = trackTuple->payload;
        stateOfKeyTableR->insert(sk);
      } else {
        sk = ImproveStateOfKeyTo(MSWJStateOfKey, skrf);
      }
      timeBreakDownIndex += timeTrackingEnd(tt_index);timeTrackingStart(tt_prediction);
      updateStateOfKey(sk, trackTuple);
      double futureTuplesR = MeanAQPIAWJOperator::predictUnarrivedTuples(sk);
      timeBreakDownPrediction += timeTrackingEnd(tt_prediction);
      //probe in S
      timeTrackingStart(tt_join);
      AbstractStateOfKeyPtr probrPtr = stateOfKeyTableS->getByKey(trackTuple->key);
      if (probrPtr != nullptr) {
        // std::string compensation = config->tryString("compensation", "false");
        /**
         * @brief update the avg value of R
         */
        sk->joinedRValueSum += (int64_t) trackTuple->payload;
        sk->joinedRValueCnt++;
        sk->joinedRValueAvg = sk->joinedRValueSum / sk->joinedRValueCnt;
        sk->rvAvgPrediction = sk->joinedRValuePredictor.update(sk->joinedRValueAvg);
        MSWJStateOfKeyPtr py = ImproveStateOfKeyTo(MSWJStateOfKey, probrPtr);
        confirmedResult += py->arrivedTupleCnt;
        if (joinSum) {
          double tc = (int64_t) py->arrivedTupleCnt * sk->joinedRValueAvg;
          confirmedResultJS += (uint64_t) tc;
        }

        if (!mswjCompensation) {
          intermediateResult += py->arrivedTupleCnt;

          if (joinConfirmCountMap.find(sk->key) == joinConfirmCountMap.end()) {
            joinConfirmCountMap[sk->key] = {{0, 0}, 0};
          }
          if (joinAQPCountMap.find(sk->key) == joinAQPCountMap.end()) {
            joinAQPCountMap[sk->key] = {{0, 0}, 0};
          }

          joinAQPCountMap[sk->key] = {{joinAQPCountMap[sk->key].first.first + py->arrivedTupleCnt,
                                       joinAQPCountMap[sk->key].first.second + sk->value},
                                      joinAQPCountMap[sk->key].second + 1};
          joinConfirmCountMap[sk->key] = {{joinConfirmCountMap[sk->key].first.first + py->arrivedTupleCnt,
                                           joinConfirmCountMap[sk->key].first.second + sk->value},
                                          joinConfirmCountMap[sk->key].second + 1};
          productivityProfiler->updateJoinRes(get_D(trackTuple->delay),
                                              py->arrivedTupleCnt);
        } else {
          size_t resultPlus = (futureTuplesR + sk->arrivedTupleCnt) *
              (py->lastUnarrivedTuples + py->arrivedTupleCnt) -
              (sk->arrivedTupleCnt + sk->lastUnarrivedTuples - 1) *
                  (py->lastUnarrivedTuples + py->arrivedTupleCnt);
          intermediateResult += resultPlus;

          if (joinConfirmCountMap.find(sk->key) == joinConfirmCountMap.end()) {
            joinConfirmCountMap[sk->key] = {{0, 0}, 0};
          }
          if (joinAQPCountMap.find(sk->key) == joinAQPCountMap.end()) {
            joinAQPCountMap[sk->key] = {{0, 0}, 0};
          }

          joinAQPCountMap[sk->key] = {{joinAQPCountMap[sk->key].first.first + resultPlus,
                                       joinAQPCountMap[sk->key].first.second + sk->value},
                                      joinAQPCountMap[sk->key].second + 1};
          joinConfirmCountMap[sk->key] = {{joinConfirmCountMap[sk->key].first.first + py->arrivedTupleCnt,
                                           joinConfirmCountMap[sk->key].first.second + sk->value},
                                          joinConfirmCountMap[sk->key].second + 1};
          productivityProfiler->updateJoinRes(get_D(trackTuple->delay),
                                              (futureTuplesR + sk->arrivedTupleCnt) *
                                                  (py->lastUnarrivedTuples + py->arrivedTupleCnt) -
                                                  (sk->arrivedTupleCnt + sk->lastUnarrivedTuples - 1) *
                                                      (py->lastUnarrivedTuples + py->arrivedTupleCnt));
        }
      }
      timeBreakDownJoin += timeTrackingEnd(tt_join);

      sk->lastUnarrivedTuples = futureTuplesR;
      lastTimeOfR = UtilityFunctions::timeLastUs(timeBaseStruct);
    }
  }

  return true;
}

auto StreamOperator::setConfig(INTELLI::ConfigMapPtr cfg) -> bool {
  if (!OoOJoin::MeanAQPIAWJOperator::setConfig(cfg)) {
    return false;
  }
  joinSum = cfg->tryU64("joinSum", 0, true);
  std::string wmTag = config->tryString("wmTag", "arrival", true);
  WMTablePtr wmTable = newWMTable();
  wmGen = wmTable->findWM(wmTag);
  if (wmGen == nullptr) {
    INTELLI_ERROR("NO such a watermarker named [" + wmTag + "]");
    return false;
  }
  INTELLI_INFO("Using the watermarker named [" + wmTag + "]");
  return true;
}

size_t StreamOperator::getResult() {
  if (joinSum) {
    /*uint64_t result = 0;
    for (auto it: joinConfirmCountMap) {
        result += getConfirmJoinSumResult(it.first);
    }
    return result;*/
    return confirmedResultJS;
  }
  return confirmedResult;
}

size_t StreamOperator::getAQPResult() {
  if (joinSum) {
    /*uint64_t result = 0;
    for (auto it: joinAQPCountMap) {
        result += getAQPJoinSumResult(it.first);
    }
    return result;*/
    return confirmedResultJS;
  }
  return intermediateResult;
}

size_t StreamOperator::getAQPJoinSumResult(keyType key) {
  //judge if sum of value is zero
  if (joinAQPCountMap[key].first.second == 0) {
    INTELLI_ERROR("The sum of value of key " + to_string(key) + " dataset should not be zero");
  }
  return joinAQPCountMap[key].first.first * joinAQPCountMap[key].first.second / joinAQPCountMap[key].second;
}

size_t StreamOperator::getConfirmJoinSumResult(keyType key) {
  if (joinConfirmCountMap[key].first.second == 0) {
    INTELLI_ERROR("The sum of value of key " + to_string(key) + " dataset should not be zero");
  }
  return joinConfirmCountMap[key].first.first * joinConfirmCountMap[key].first.second /
      joinConfirmCountMap[key].second;
}



