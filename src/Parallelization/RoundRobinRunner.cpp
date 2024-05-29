//
// Created by tony on 27/06/23.
//

#include <Parallelization/RoundRobinRunner.h>
#include <string>

void OoOJoin::RoundRobinWorker::decentralizedMain() {

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
    auto end = std::chrono::high_resolution_clock::now();

    // Compute the duration in microseconds
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // Print the duration
    tNow = duration.count();

    while (tNow >= tNextS) {
      if (sPos <= testSize - 1) {
        auto newTs = sTuple[sPos];
        testOp->feedTupleS(newTs);
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
    while (tNow >= tNextR) {

      if (rPos <= testSize - 1) {
        auto newTr = rTuple[rPos];
        if (rPos % workers == myId) {
          testOp->feedTupleR(newTr);
          //INTELLI_INFO("TID " + to_string(myId) + "it's my turn");
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
    usleep(20);
  }
  testOp->stop();
  INTELLI_INFO("End tid" + to_string(myId) + " run");
}

void OoOJoin::RoundRobinWorker::inlineMain() {
  decentralizedMain();
}

void OoOJoin::RoundRobinRunner::setConfig(INTELLI::ConfigMapPtr _cfg) {
  cfg = _cfg;
  threads = cfg->tryU64("threads", 1, true);
  myWorker = std::vector<OoOJoin::KeyPartitionWorkerPtr>(threads);
  for (uint64_t i = 0; i < threads; i++) {
    myWorker[i] = newRoundRobinWorker();
    myWorker[i]->setConfig(cfg);
    myWorker[i]->setId(i, threads);
  }
}
void OoOJoin::RoundRobinRunner::runStreaming() {
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
size_t OoOJoin::RoundRobinRunner::getResult() {
  size_t ru = 0;
  for (uint64_t i = 0; i < threads; i++) {
    ru += myWorker[i]->getResult();
  }
  return ru;
}

void OoOJoin::RoundRobinRunner::setDataSet(std::vector<TrackTuplePtr> _r, std::vector<TrackTuplePtr> _s) {
  for (uint64_t i = 0; i < threads; i++) {
    myWorker[i]->setDataSet(_r, _s);
  }
}

size_t OoOJoin::RoundRobinRunner::getAQPResult() {
  size_t ru = 0;
  for (uint64_t i = 0; i < threads; i++) {
    ru += myWorker[i]->getAQPResult();
  }
  return ru;
}

double OoOJoin::RoundRobinRunner::getThroughput() {
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

double OoOJoin::RoundRobinRunner::getLatencyPercentage(double fraction) {
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