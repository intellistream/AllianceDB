//
// Created by tony on 23/11/22.
//

#include <TestBench/TestBench.h>
#include <Utils/UtilityFunctions.hpp>
#include <algorithm>
#include <utility>
#include <Utils/IntelliLog.h>
#include <TestBench/RandomDataLoader.h>
#include <Operator/MSWJOperator.h>

using namespace INTELLI;
using namespace OoOJoin;
using namespace std;

void OoOJoin::TestBench::OoOSort(std::vector<TrackTuplePtr> &arr) {
  std::sort(arr.begin(), arr.end(), [](const TrackTuplePtr &t1, const TrackTuplePtr &t2) {
    return t1->arrivalTime < t2->arrivalTime;
  });

  ofstream outfile("data.txt");
  if (outfile.is_open()) { // 如果文件成功打开
    for (auto it : arr) {
      outfile << it->key << "," << it->eventTime << "," << it->arrivalTime <<
              endl; // 将数组元素写入文件中
    }
    outfile.close(); // 关闭文件
  } else {
    cout << "Fail to create data.txt" << endl;
  }

}

void OoOJoin::TestBench::forceInOrder(std::vector<TrackTuplePtr> &arr) {
  size_t len = arr.size();
  size_t i;
  for (i = 0; i < len; i++) {
    arr[i]->arrivalTime = arr[i]->eventTime;
  }
}

void OoOJoin::TestBench::inOrderSort(std::vector<TrackTuplePtr> &arr) {
  size_t len = arr.size();
  std::sort(arr.begin(), arr.end(), [](const TrackTuplePtr &t1, const TrackTuplePtr &t2) {
    return t1->eventTime < t2->eventTime;
  });
  for (size_t i = 0; i < len; i++) {
    arr[i]->arrivalTime = arr[i]->eventTime;
  }
}

void OoOJoin::TestBench::setDataSet(std::vector<TrackTuplePtr> _r, std::vector<TrackTuplePtr> _s) {
  rTuple = std::move(_r);
  sTuple = std::move(_s);
//    std::vector<TrackTuplePtr> r_;
//    std::vector<TrackTuplePtr> s_;
//    for (int i = 0; i < 5000; i++) {
//        r_.push_back(rTuple[i]);
//        s_.push_back(sTuple[i]);
//    }
//    rTuple = std::move(r_);
//    sTuple = std::move(s_);
}

bool OoOJoin::TestBench::setOperator(OoOJoin::AbstractOperatorPtr op, ConfigMapPtr cfg) {
  testOp = std::move(op);
  opConfig = std::move(cfg);
  if (opConfig == nullptr) {
    return false;
  }
  if (opConfig->existU64("timeStep")) {
    timeStep = opConfig->getU64("timeStep");
    // TB_INFO("Feeding time step=" + to_string(timeStep) + "us");
  } else {
    //  TB_WARNNING("No setting of timeStep, use 1\n");
    timeStep = 1;
  }
  return true;
}
void OoOJoin::TestBench::aiPretrain() {
  /**
   * @brief 1. get the max time stamp of s and r
   */
  uint64_t sMax = sTuple[sTuple.size() - 1]->arrivalTime;
  uint64_t rMax = sTuple[sTuple.size() - 1]->arrivalTime;
  uint64_t wmTime = sMax;
  if (rMax > sMax) {
    wmTime = rMax;
  }
  wmTime++;
  opConfig->edit("watermarkTimeMs", (uint64_t) wmTime / 1000);
  /***
   * @brief only valid for pre-train!!!
   */
  opConfig->edit("selLen", (uint64_t) (sTuple.size() + rTuple.size()));
  INTELLI_WARNING("In pre-training, force watermarkTime to be " + to_string(wmTime));
  if (opConfig->existString("operator") && opConfig->getString("operator") == "MSWJ") {
    inlineTestOfMSWJ();
  } else {
    inlineTestOfCommon();
  }
  //exit(0);
}
void OoOJoin::TestBench::inlineTest() {
  if (opConfig->existString("aiMode") && opConfig->getString("aiMode") == "pretrain") {
    INTELLI_WARNING("Note, this is training, will terminate after done");
    aiPretrain();
    //exit(0);
    return;
  }

  if (opConfig->existString("operator") && opConfig->getString("operator") == "MSWJ") {
    inlineTestOfMSWJ();
  } else {
    inlineTestOfCommon();
  }
}

size_t OoOJoin::TestBench::OoOTest(bool additionalSort) {
  if (additionalSort) {
    OoOSort(rTuple);
    OoOSort(sTuple);
  }
  inlineTest();
  AQPResult = testOp->getAQPResult();
  return testOp->getResult();
}

size_t OoOJoin::TestBench::inOrderTest(bool additionalSort) {

  /**
   * @brief prevent ai operator from appending tensors
   */
  opConfig->edit("appendTensor", (uint64_t) 0);
  opConfig->edit("operator", "IAWJ");
  /*forceInOrder(rTuple);
  forceInOrder(sTuple);
  if (additionalSort) {
    inOrderSort(rTuple);
    inOrderSort(sTuple);
  }*/
  std::cout << additionalSort;
  uint64_t sMax = sTuple[sTuple.size() - 1]->arrivalTime;
  uint64_t rMax = sTuple[sTuple.size() - 1]->arrivalTime;
  uint64_t wmTime = sMax;
  if (rMax > sMax) {
    wmTime = rMax;
  }
  wmTime++;
  opConfig->edit("watermarkTimeMs", (uint64_t) wmTime / 1000);
  inlineTest();
  AQPResult = testOp->getAQPResult();
  return testOp->getResult();
}

void OoOJoin::TestBench::logRTuples(bool skipZero) {
  //   TB_INFO("/***Printing the rTuples in the following***/");
  size_t rLen = rTuple.size();
  for (size_t i = 0; i < rLen; i++) {
    if (skipZero && rTuple[i]->processedTime == 0) {

    } else {
      //TB_INFO(rTuple[i]->toString());
    }

  }
  // TB_INFO("/***Done***/");
}

bool OoOJoin::TestBench::saveRTuplesToFile(std::string fname, bool skipZero) {
  ofstream of;
  of.open(fname);
  if (of.fail()) {
    return false;
  }
  of << "key,value,eventTime,arrivalTime,processedTime\n";
  size_t rLen = rTuple.size();
  for (size_t i = 0; i < rLen; i++) {
    if (skipZero && rTuple[i]->processedTime == 0) {

    } else {
      TrackTuplePtr tp = rTuple[i];
      string line = to_string(tp->key) + "," + to_string(tp->payload) + "," + to_string(tp->eventTime) + ","
          + to_string(tp->arrivalTime) + "," + to_string(tp->processedTime) + "\n";
      of << line;
    }

  }
  of.close();
  return true;
}

bool OoOJoin::TestBench::saveSTuplesToFile(std::string fname, bool skipZero) {
  ofstream of;
  of.open(fname);
  if (of.fail()) {
    return false;
  }
  of << "key,value,eventTime,arrivalTime,processedTime\n";
  size_t rLen = rTuple.size();
  for (size_t i = 0; i < rLen; i++) {
    if (skipZero && rTuple[i]->processedTime == 0) {

    } else {
      TrackTuplePtr tp = rTuple[i];
      string line = to_string(tp->key) + "," + to_string(tp->payload) + "," + to_string(tp->eventTime) + ","
          + to_string(tp->arrivalTime) + "," + to_string(tp->processedTime) + "\n";
      of << line;
    }

  }
  of.close();
  return true;
}

double OoOJoin::TestBench::getAvgLatency() {
  size_t rLen = rTuple.size();
  size_t nonZeroCnt = 0;
  double sum = 0;
  for (size_t i = 0; i < rLen; i++) {
    if (rTuple[i]->processedTime >= rTuple[i]->arrivalTime && rTuple[i]->processedTime != 0) {
      double temp = rTuple[i]->processedTime - rTuple[i]->arrivalTime;
      sum += temp;
      nonZeroCnt++;
    }
  }
  return sum / nonZeroCnt;
}

double OoOJoin::TestBench::getThroughput() {
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
        TB_WARNNING("No valid elapsed time, maybe there is no joined result?");
    return 0;
  }
  double thr = rLen;
  thr = thr * 1e6 / elapsedTime;
  return thr;
}

double OoOJoin::TestBench::getLatencyPercentage(double fraction) {
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
        TB_WARNNING("No valid latency, maybe there is no joined result?");
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

ConfigMapPtr OoOJoin::TestBench::getTimeBreakDown() {
  if (testOp != nullptr) {
    return testOp->getTimeBreakDown();
  }
  return nullptr;
}

void OoOJoin::TestBench::setDataLoader(const std::string &tag, ConfigMapPtr globalCfg) {
  DataLoaderTablePtr dt = newDataLoaderTable();
  AbstractDataLoaderPtr dl = dt->findDataLoader(tag);
  if (dl == nullptr) {
        TB_WARNNING("Invalid DataLoader [" + tag + "], use random instead");
    dl = newRandomDataLoader();
  }
  dl->setConfig(globalCfg);
  setDataSet(dl->getTupleVectorS(), dl->getTupleVectorR());
  TB_INFO("Using DataLoader [" + tag + "]");
}

void TestBench::inlineTestOfCommon() {
  struct timeval timeStart{};
  size_t testSize = (rTuple.size() > sTuple.size()) ? sTuple.size() : rTuple.size();
  size_t rPos = 0, sPos = 0;
  size_t tNow = 0;
  size_t tMaxS = sTuple[testSize - 1]->arrivalTime;
  size_t tMaxR = rTuple[testSize - 1]->arrivalTime;
  size_t tMax = (tMaxS > tMaxR) ? tMaxS : tMaxR;
  size_t tNextS = 0, tNextR = 0;
  /*for(size_t i=0;i<testSize;i++)
  {
   TB_INFO(sTuple[i]->toString());
  }*/
  testOp->setConfig(opConfig);
  gettimeofday(&timeStart, nullptr);
  testOp->syncTimeStruct(timeStart);
  testOp->start();
  while (tNow < tMax) {
    tNow = UtilityFunctions::timeLastUs(timeStart);
    //INTELLI_INFO("T=" << tNow);
    while (tNow >= tNextS) {
      //tNow = UtilityFunctions::timeLastUs(timeStart);
      if (sPos <= testSize - 1) {
        testOp->feedTupleS(sTuple[sPos]);
        sPos++;
        if (sPos <= testSize - 1) {
          tNextS = sTuple[sPos]->arrivalTime;
        } else {
          tNextS = -1;
          //  INTELLI_INFO("NO MORE S");
          break;
        }

      }

    }
    tNow = UtilityFunctions::timeLastUs(timeStart);
    //INTELLI_INFO("detect R");
    while (tNow >= tNextR) {
      // tNow = UtilityFunctions::timeLastUs(timeStart);
      if (rPos <= testSize - 1) {
        testOp->feedTupleR(rTuple[rPos]);
        //INTELLI_INFO("feed"+rTuple[rPos]->toString()+"at "+ to_string(tNow));
        rPos++;
        if (rPos <= testSize - 1) {
          tNextR = rTuple[rPos]->arrivalTime;

        } else {
          tNextR = -1;
          //  INTELLI_INFO("NO MORE R");
          break;
        }
      }
    }
    //usleep(20);
  }
  testOp->stop();
}

void TestBench::inlineTestOfMSWJ() {
  struct timeval timeStart{};
  size_t testSize = (rTuple.size() > sTuple.size()) ? sTuple.size() : rTuple.size();
  size_t rPos = 0, sPos = 0;
  size_t tNow = 0;
  size_t tMaxS = sTuple[testSize - 1]->arrivalTime;
  size_t tMaxR = rTuple[testSize - 1]->arrivalTime;
  size_t tMax = (tMaxS > tMaxR) ? tMaxS : tMaxR;
  size_t tNextS = 0, tNextR = 0;
  /*for(size_t i=0;i<testSize;i++)
  {
   TB_INFO(sTuple[i]->toString());
  }*/
  testOp->setConfig(opConfig);
  gettimeofday(&timeStart, nullptr);
  testOp->syncTimeStruct(timeStart);
  testOp->start();
  while (tNow < tMax) {
    tNow = UtilityFunctions::timeLastUs(timeStart);
    //INTELLI_INFO("T=" << tNow);
    while (tNow >= tNextS) {
      if (sPos <= testSize - 1) {
        testOp->feedTupleS(sTuple[sPos]);
        sPos++;
        if (sPos <= testSize - 1) {
          tNextS = sTuple[sPos]->arrivalTime;
        } else {
          tNextS = -1;
          //pass a end flag to operator
          TrackTuplePtr endFlagTuple = newTrackTuple(1);
          endFlagTuple->isEnd = true;
          testOp->feedTupleS(endFlagTuple);
          //  INTELLI_INFO("NO MORE S");
          break;
        }

      }

    }
    tNow = UtilityFunctions::timeLastUs(timeStart);
    //INTELLI_INFO("detect R");
    while (tNow >= tNextR) {
      if (rPos <= testSize - 1) {
        testOp->feedTupleR(rTuple[rPos]);
        //INTELLI_INFO("feed"+rTuple[rPos]->toString()+"at "+ to_string(tNow));
        rPos++;
        if (rPos <= testSize - 1) {
          tNextR = rTuple[rPos]->arrivalTime;

        } else {
          tNextR = -1;
          TrackTuplePtr endFlagTuple = newTrackTuple(1);
          endFlagTuple->isEnd = true;
          testOp->feedTupleR(endFlagTuple);
          //  INTELLI_INFO("NO MORE R");
          break;
        }
      }
    }
    //usleep(20);
  }
  testOp->stop();
}
