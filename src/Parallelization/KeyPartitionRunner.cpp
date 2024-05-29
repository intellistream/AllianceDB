//
// Created by tony on 27/06/23.
//

#include <Parallelization/KeyPartitionRunner.h>
#include <string>
#include <chrono>
#include <Utils/UtilityFunctions.hpp>
static inline uint64_t elfHash(uint64_t x, uint32_t bits) { return (x * 2654435761U) >> (32 - bits); }
void OoOJoin::KeyPartitionWorker::setDataSet(std::vector<TrackTuplePtr> _r, std::vector<TrackTuplePtr> _s) {
  size_t sLen = _s.size();
  size_t rLen = _r.size();
  rTuple = std::vector<TrackTuplePtr>(rLen);
  sTuple = std::vector<TrackTuplePtr>(sLen);
  for (size_t i = 0; i < rLen; i++) {
    rTuple[i] = newTrackTuple(*_r[i]);
  }
  for (size_t i = 0; i < sLen; i++) {
    sTuple[i] = newTrackTuple(*_s[i]);
  }

}
void OoOJoin::KeyPartitionWorker::setConfig(INTELLI::ConfigMapPtr _cfg) {
  OoOJoin::OperatorTable opt;
  cfg = _cfg;
  std::string opTag = cfg->tryString("operator", "IMA");
  testOp = opt.findOperator(opTag);
  if (testOp == nullptr) {
    INTELLI_ERROR("can not find the operator");
    exit(-1);
  }
  testOp->setConfig(_cfg);
  if(opTag=="PRJ"||opTag=="LazyIAWJSel")
  {
    isLazy= true;
  }
 windowLen = _cfg->getU64("windowLen");
}
void OoOJoin::KeyPartitionWorker::prepareRunning() {

  // testOp->start();
}
bool OoOJoin::KeyPartitionWorker::isMySTuple(OoOJoin::TrackTuplePtr ts) {
  if (elfHash(ts->key,10) % workers == myId) {
    return true;
  }
  return false;
}
bool OoOJoin::KeyPartitionWorker::isMyRTuple(OoOJoin::TrackTuplePtr tr) {
  if (elfHash(tr->key,10)% workers == myId) {
    return true;
  }
  return false;
}
void OoOJoin::KeyPartitionWorker::decentralizedMain() {

  INTELLI_INFO("Start tid" + to_string(myId) + " run");
  INTELLI::UtilityFunctions::bind2Core(myId);
  size_t testSize = (rTuple.size() > sTuple.size()) ? sTuple.size() : rTuple.size();
  size_t rPos = 0, sPos = 0;
  size_t tNow = 0;
  size_t tMaxS = sTuple[testSize - 1]->arrivalTime;
  size_t tMaxR = rTuple[testSize - 1]->arrivalTime;
  size_t tMax = (tMaxS > tMaxR) ? tMaxS : tMaxR;
  size_t tNextS = 0, tNextR = 0;
  gettimeofday(&tSystem, nullptr);
  testOp->syncTimeStruct(tSystem);
  testOp->start();
  auto start = std::chrono::high_resolution_clock::now();

  //testOp->start();
  while (tNow < tMax) {
    // tNow = UtilityFunctions::timeLastUs(tSystem);
    auto end = std::chrono::high_resolution_clock::now();

    // Compute the duration in microseconds
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // Print the duration
    tNow = duration.count();
    //printf("%ld\r\n",tNow);

    if (tNow >= tNextS) {
      if (sPos <= testSize - 1) {
        auto newTs = sTuple[sPos];
        if (isMySTuple(newTs)) {
          testOp->feedTupleS(newTs);

        }
        sPos++;
        if (sPos <= testSize - 1) {
          tNextS = sTuple[sPos]->arrivalTime;
        } else {
          tNextS = -1;
          break;
        }

      }

    }
    end = std::chrono::high_resolution_clock::now();

    // Compute the duration in microseconds
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    // Print the duration
    tNow = duration.count();
    if (tNow >= tNextR) {

      if (rPos <= testSize - 1) {
        auto newTr = rTuple[rPos];
        if (isMyRTuple(newTr))
          //if(rPos%workers==myId)
        {
          testOp->feedTupleR(newTr);
        }
        rPos++;
        if (rPos <= testSize - 1) {
          tNextR = rTuple[rPos]->arrivalTime;
        } else {
          tNextR = -1;
          break;
        }
      }
    }
    //usleep(20);
  }
  testOp->stop();
  INTELLI_INFO("End tid" + to_string(myId) + " run");
}

double OoOJoin::KeyPartitionWorker::getThroughput() {
  if(isLazy)
  {
   // INTELLI_INFO("this is lazy worker");
   return testOp->getLazyRunningThroughput()*workers;
  }
  size_t rLen = rTuple.size();
  tsType minArrival = rTuple[0]->arrivalTime;
  tsType maxProcessed = 0;
  for (size_t i = 0; i < rLen; i++) {
    if (rTuple[i]->processedTime >= maxProcessed) {
      maxProcessed = rTuple[i]->processedTime;
    }
    if (rTuple[i]->arrivalTime <= minArrival) {
      minArrival = rTuple[i]->arrivalTime;
    }
  }
  double elapsedTime = (maxProcessed - minArrival);
  if (elapsedTime <= 0) {
    INTELLI_WARNING("No valid elapsed time, maybe there is no joined result?");
    return 0;
  }
  double thr = rLen;
  thr = thr * 1e6 / elapsedTime;
  return thr;
}

double OoOJoin::KeyPartitionWorker::getLatencyPercentage(double fraction) {
  size_t rLen = rTuple.size();
  size_t nonZeroCnt = 0;
  vector<tsType> validLatency;
  for (size_t i = 0; i < rLen; i++) {
    if (rTuple[i]->processedTime >= rTuple[i]->arrivalTime && rTuple[i]->processedTime != 0) {
      validLatency.push_back(rTuple[i]->processedTime - rTuple[i]->arrivalTime);
      nonZeroCnt++;
    }
  }
  if (nonZeroCnt == 0) {
    INTELLI_WARNING("No valid latency, maybe there is no joined result?");
    return 0;
  }
  std::sort(validLatency.begin(), validLatency.end());
  double t = nonZeroCnt;
  t = t * fraction;
  size_t idx = (size_t) t + 1;
  if (idx >= validLatency.size()) {
    idx = validLatency.size() - 1;
  }
  return validLatency[idx];
}
size_t OoOJoin::KeyPartitionWorker::getResult() {
  if (testOp == nullptr) {
    return 0;
  }
  return testOp->getResult();
}
size_t OoOJoin::KeyPartitionWorker::getAQPResult() {
  if (testOp == nullptr) {
    return 0;
  }
  return testOp->getAQPResult();
}
void OoOJoin::KeyPartitionWorker::inlineMain() {
  decentralizedMain();
}

void OoOJoin::KeyPartitionRunner::setConfig(INTELLI::ConfigMapPtr _cfg) {
  cfg = _cfg;
  threads = cfg->tryU64("threads", 1, true);
  myWorker = std::vector<OoOJoin::KeyPartitionWorkerPtr>(threads);
  for (uint64_t i = 0; i < threads; i++) {
    myWorker[i] = newKeyPartitionWorker();
    myWorker[i]->setConfig(cfg);
    myWorker[i]->setId(i, threads);
  }
}
void OoOJoin::KeyPartitionRunner::setDataSet(std::vector<TrackTuplePtr> _r, std::vector<TrackTuplePtr> _s) {
  for (uint64_t i = 0; i < threads; i++) {
    myWorker[i]->setDataSet(_r, _s);
  }
}
void OoOJoin::KeyPartitionRunner::runStreaming() {
  for (uint64_t i = 0; i < threads; i++) {
    myWorker[i]->prepareRunning();
  }
  for (uint64_t i = 0; i < threads; i++) {
    myWorker[i]->startThread();
  }
  INTELLI_INFO("Start " + to_string(threads) + " threads");
  for (uint64_t i = 0; i < threads; i++) {
    myWorker[i]->joinThread();
  }
  INTELLI_INFO("Finish " + to_string(threads) + " threads");
}
size_t OoOJoin::KeyPartitionRunner::getResult() {
  size_t ru = 0;
  for (uint64_t i = 0; i < threads; i++) {
    ru += myWorker[i]->getResult();
  }
  return ru;
}

size_t OoOJoin::KeyPartitionRunner::getAQPResult() {
  size_t ru = 0;
  for (uint64_t i = 0; i < threads; i++) {
    ru += myWorker[i]->getAQPResult();
  }
  return ru;
}

double OoOJoin::KeyPartitionRunner::getThroughput() {
  double ru = 0;
  uint64_t nz = 0;
  for (uint64_t i = 0; i < threads; i++) {
    ru += myWorker[i]->getThroughput();
    if (myWorker[i]->getThroughput() != 0) {
      nz++;
    }
  }
  return ru / nz;
}
double OoOJoin::KeyPartitionRunner::getLatencyPercentage(double fraction) {
  double ru = 0;
  uint64_t nz = 0;
  for (uint64_t i = 0; i < threads; i++) {
    ru += myWorker[i]->getLatencyPercentage(fraction);
    if (myWorker[i]->getLatencyPercentage(fraction) != 0) {
      nz++;
    }
  }
  return ru / nz;
}