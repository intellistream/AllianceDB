//
// Created by tony on 22/11/22.
//

#include <Operator/LazyIAWJSelOperator.h>
#include <JoinAlgos/JoinAlgoTable.h>
#include <cmath>
#include <chrono>
bool OoOJoin::LazyIAWJSelOperator::setConfig(INTELLI::ConfigMapPtr cfg) {
  if (!OoOJoin::AbstractOperator::setConfig(cfg)) {
    return false;
  }

  joinThreads = 1;
  algoTag = "NPJSingle";
  // OP_INFO("selected join algorithm=" + algoTag);
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

bool OoOJoin::LazyIAWJSelOperator::start() {
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
void OoOJoin::LazyIAWJSelOperator::LazyStatistics() {
  auto processedSLen=myWindow.windowS.size();
  auto processedRLen=myWindow.windowR.size();
  int64_t  validS=0,validR=0;
  avgSkewR=0;
  avgSkewS=0;
  /**
   * @brief statistic on R
   */
  for(size_t i=0;i<processedSLen;i++)
  {
    auto tr=*myWindow.windowR.data(i);
    if(tr!=nullptr)
    {
      avgSkewR+=tr->arrivalTime-tr->eventTime;
      validR++;
    }
  }
  avgSkewR=avgSkewR/validR;

  /**
   * @brief statistic on S
   */
  for(size_t i=0;i<processedRLen;i++)
  {
    auto ts=*myWindow.windowS.data(i);
    if(ts!=nullptr)
    {
      avgSkewS+=ts->arrivalTime-ts->eventTime;
      validS++;
    }
  }
  avgSkewS=avgSkewS/validS;

}
double OoOJoin::LazyIAWJSelOperator::LazyPredictFutureTuples(double avgSkew,uint64_t arrivedCnt) {
  tsType lastTime =lastArriveTime;
  double goThroughTime = lastTime - avgSkew - myWindow.getStart();
  double futureTime = myWindow.getEnd() + avgSkew - lastTime;
  double max_ratio = 100.0;
  //double alpha = 0.5;
  double ratio = futureTime / (goThroughTime > 0 ? goThroughTime : 1); // prevent division by zero
  ratio = std::min(ratio, max_ratio); // Limit the ratio to the maximum allowed value
  // Apply adaptive EWMA
  double futureTuple = ratio * arrivedCnt;
  return futureTuple;
}
void OoOJoin::LazyIAWJSelOperator::conductComputation() {
  JoinAlgoTable jt;
  AbstractJoinAlgoPtr algo = jt.findAlgo(algoTag);
  //NestedLoopJoin nj;
  algo->setConfig(config);
  algo->syncTimeStruct(timeBaseStruct);
  auto lazyStart = std::chrono::high_resolution_clock::now();
  // OP_INFO("Invoke algorithm=" + algo->getAlgoName());
  LazyStatistics();
  double unarrivedS=LazyPredictFutureTuples(avgSkewS,myWindow.windowS.size());
  double unarrivedR=LazyPredictFutureTuples(avgSkewR,myWindow.windowR.size());
  intermediateResult = algo->join(myWindow.windowS, myWindow.windowR, joinThreads);
  auto lazyEnd = std::chrono::high_resolution_clock::now();

  // Compute the duration in microseconds
  auto lazyDuration = std::chrono::duration_cast<std::chrono::microseconds>(lazyEnd - lazyStart);

  lazyRunningTime=lazyDuration.count();
  double sel=intermediateResult;
  sel=sel/myWindow.windowR.size()/myWindow.windowS.size();
  double aqpRu=sel*(myWindow.windowS.size()+unarrivedS)*(myWindow.windowR.size()+unarrivedR);
  compensatedRu=aqpRu;
  algo->labelProceesedTime(myWindow.windowR);
}

bool OoOJoin::LazyIAWJSelOperator::stop() {
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

bool OoOJoin::LazyIAWJSelOperator::feedTupleS(OoOJoin::TrackTuplePtr ts) {
  bool shouldGenWM;
  if (lockedByWaterMark) {

    return false;
  }
  myWindow.feedTupleS(ts);
  shouldGenWM = wmGen->reportTupleS(ts, 1);
  if (shouldGenWM) {
    lockedByWaterMark = true;
    lastArriveTime=ts->arrivalTime;
    // run computation
    conductComputation();
  }
  return true;
}

bool OoOJoin::LazyIAWJSelOperator::feedTupleR(OoOJoin::TrackTuplePtr tr) {
  bool shouldGenWM;
  if (lockedByWaterMark) {
    return false;
  }
  myWindow.feedTupleR(tr);
  shouldGenWM = wmGen->reportTupleR(tr, 1);
  if (shouldGenWM) {
    lockedByWaterMark = true;
    lastArriveTime=tr->arrivalTime;
    // run computation
    conductComputation();
  }
  return true;
}

size_t OoOJoin::LazyIAWJSelOperator::getResult() {
  return intermediateResult;
}

size_t OoOJoin::LazyIAWJSelOperator::getAQPResult()  {
  return compensatedRu;

}

double OoOJoin::LazyIAWJSelOperator::getLazyRunningThroughput() {
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
