#define CATCH_CONFIG_MAIN

#include "catch.hpp"
//#include <Common/LinearVAE.h>
#include <vector>
#include <OoOJoin.h>
//using namespace std;
using namespace OoOJoin;

#include "catch.hpp"
#include <Operator/MSWJ/KSlack/KSlack.h>
#include "TestFunction.cpp"

using namespace std;
using namespace OoOJoin;
using namespace MSWJ;

/*TEST_CASE(
    "KSlack disorderHandling function static test,this test same as paper's Fig. 3. Example of using K-slack to handle intra-stream disorder",
    "[KSlack]") {
  // Create objects needed for KSlack instance
  ConfigMapPtr cfg = newConfigMap();
  cfg->edit("g", (uint64_t) 10 * MILLION_SECONDS);
  cfg->edit("L", (uint64_t) 50 * MILLION_SECONDS);
  cfg->edit("userRecall", 0.4);
  cfg->edit("b", (uint64_t) 10 * MILLION_SECONDS);
  cfg->edit("confidenceValue", 0.5);
  cfg->edit("P", (uint64_t) 10 * SECONDS);
  cfg->edit("maxDelay", (uint64_t) INT16_MAX);
  cfg->edit("StreamCount", (uint64_t) 2);
  cfg->edit("Stream_1", (uint64_t) 0);
  cfg->edit("Stream_2", (uint64_t) 0);

  auto tupleProductivityProfiler = std::make_shared<MSWJ::TupleProductivityProfiler>(cfg);
  auto statisticsManager = std::make_shared<MSWJ::StatisticsManager>(tupleProductivityProfiler.get(), cfg);
  auto bufferSizeManager = std::make_shared<MSWJ::BufferSizeManager>(statisticsManager.get(),
                                                                     tupleProductivityProfiler.get());
  auto streamOperator = std::make_shared<MSWJ::StreamOperator>(tupleProductivityProfiler.get(), cfg);
  auto synchronizer = std::make_shared<MSWJ::Synchronizer>(2, streamOperator.get(), cfg);

  bufferSizeManager->setConfig(cfg);

  // Create KSlack instance
  KSlack kSlack(1, bufferSizeManager.get(), statisticsManager.get(), synchronizer.get());
  kSlack.bufferSize = 1;

  // Create multiple TrackTuplePtr objects to simulate incoming tuples
  TrackTuplePtr tuple1 = std::make_shared<TrackTuple>(1, 1, 1);
  TrackTuplePtr tuple2 = std::make_shared<TrackTuple>(1, 2, 4);
  TrackTuplePtr tuple3 = std::make_shared<TrackTuple>(1, 3, 3);
  TrackTuplePtr tuple4 = std::make_shared<TrackTuple>(1, 4, 5);
  TrackTuplePtr tuple5 = std::make_shared<TrackTuple>(1, 5, 7);
  TrackTuplePtr tuple6 = std::make_shared<TrackTuple>(1, 6, 8);
  TrackTuplePtr tuple7 = std::make_shared<TrackTuple>(1, 7, 6);
  TrackTuplePtr tuple8 = std::make_shared<TrackTuple>(1, 8, 9);

  // Call disorderHandling function with tuples
  kSlack.disorderHandling(tuple1);
  REQUIRE(kSlack.buffer.top()->eventTime == 1);
  REQUIRE(kSlack.currentTime == 1);

  kSlack.disorderHandling(tuple2);
  REQUIRE(kSlack.buffer.size() == 1);
  REQUIRE(kSlack.currentTime == 4);
  REQUIRE(kSlack.buffer.top()->eventTime == 4);

  kSlack.disorderHandling(tuple3);
  REQUIRE(kSlack.buffer.size() == 2);
  REQUIRE(kSlack.currentTime == 4);
  REQUIRE(kSlack.buffer.top()->eventTime == 3);

  kSlack.disorderHandling(tuple4);
  REQUIRE(kSlack.buffer.size() == 1);
  REQUIRE(kSlack.currentTime == 5);
  REQUIRE(kSlack.buffer.top()->eventTime == 5);

  kSlack.disorderHandling(tuple5);
  REQUIRE(kSlack.buffer.size() == 1);
  REQUIRE(kSlack.currentTime == 7);
  REQUIRE(kSlack.buffer.top()->eventTime == 7);

  kSlack.disorderHandling(tuple6);
  REQUIRE(kSlack.buffer.size() == 1);
  REQUIRE(kSlack.currentTime == 8);
  REQUIRE(kSlack.buffer.top()->eventTime == 8);

  kSlack.disorderHandling(tuple7);
  REQUIRE(kSlack.buffer.size() == 2);
  REQUIRE(kSlack.currentTime == 8);
  REQUIRE(kSlack.buffer.top()->eventTime == 6);

  kSlack.disorderHandling(tuple8);
  REQUIRE(kSlack.buffer.size() == 1);
  REQUIRE(kSlack.currentTime == 9);
  REQUIRE(kSlack.buffer.top()->eventTime == 9);
}

TEST_CASE("Priority queue test with TrackTuple elements", "[PriorityQueue]") {

  // Create priority queue with TrackTuple elements
  std::priority_queue<TrackTuplePtr, std::deque<TrackTuplePtr>, TrackTuplePtrComparator> pq;

  // Create multiple TrackTuple objects
  TrackTuplePtr tuple1 = std::make_shared<TrackTuple>(1, 1, 4);
  TrackTuplePtr tuple2 = std::make_shared<TrackTuple>(1, 2, 1);
  TrackTuplePtr tuple3 = std::make_shared<TrackTuple>(1, 3, 3);
  TrackTuplePtr tuple4 = std::make_shared<TrackTuple>(1, 4, 5);
  TrackTuplePtr tuple5 = std::make_shared<TrackTuple>(1, 5, 7);


  // Insert TrackTuple objects into the queue
  pq.push(tuple1);
  pq.push(tuple2);
  pq.push(tuple3);
  pq.push(tuple4);
  pq.push(tuple5);

  // Check if queue is sorted correctly
  REQUIRE(pq.top()->eventTime == 1);
  pq.pop();
  REQUIRE(pq.top()->eventTime == 3);
  pq.pop();
  REQUIRE(pq.top()->eventTime == 4);
  pq.pop();
  REQUIRE(pq.top()->eventTime == 5);
  pq.pop();
  REQUIRE(pq.top()->eventTime == 7);
  pq.pop();
}

TEST_CASE("Synchronizer synchronizeStream function test with multiple streams", "[Synchronizer]") {
  // Create objects needed for KSlack instance
  ConfigMapPtr cfg = newConfigMap();
  cfg->edit("g", (uint64_t) 10 * MILLION_SECONDS);
  cfg->edit("L", (uint64_t) 50 * MILLION_SECONDS);
  cfg->edit("userRecall", 0.4);
  cfg->edit("b", (uint64_t) 10 * MILLION_SECONDS);
  cfg->edit("confidenceValue", 0.5);
  cfg->edit("P", (uint64_t) 10 * SECONDS);
  cfg->edit("maxDelay", (uint64_t) INT16_MAX);
  cfg->edit("StreamCount", (uint64_t) 2);
  cfg->edit("Stream_1", (uint64_t) 0);
  cfg->edit("Stream_2", (uint64_t) 0);

  auto tupleProductivityProfiler = std::make_shared<MSWJ::TupleProductivityProfiler>(cfg);
  auto statisticsManager = std::make_shared<MSWJ::StatisticsManager>(tupleProductivityProfiler.get(), cfg);
  auto bufferSizeManager = std::make_shared<MSWJ::BufferSizeManager>(statisticsManager.get(),
                                                                     tupleProductivityProfiler.get());
  auto streamOperator = std::make_shared<MSWJ::StreamOperator>(tupleProductivityProfiler.get(), cfg);
  auto synchronizer = std::make_shared<MSWJ::Synchronizer>(2, streamOperator.get(), cfg);

  bufferSizeManager->setConfig(cfg);

  // Create multiple TrackTuplePtr objects to simulate incoming tuples
  TrackTuplePtr rTuple1 = std::make_shared<TrackTuple>(1, 1, 1);
  TrackTuplePtr rTuple2 = std::make_shared<TrackTuple>(1, 2, 3);
  TrackTuplePtr rTuple3 = std::make_shared<TrackTuple>(1, 3, 4);
  TrackTuplePtr rTuple4 = std::make_shared<TrackTuple>(1, 4, 5);
  TrackTuplePtr rTuple5 = std::make_shared<TrackTuple>(1, 5, 7);
  TrackTuplePtr rTuple6 = std::make_shared<TrackTuple>(1, 6, 6);
  TrackTuplePtr rTuple7 = std::make_shared<TrackTuple>(1, 7, 8);
  TrackTuplePtr rTuple8 = std::make_shared<TrackTuple>(1, 8, 9);
  TrackTuplePtr sTuple1 = std::make_shared<TrackTuple>(1, 1, 1);
  TrackTuplePtr sTuple2 = std::make_shared<TrackTuple>(1, 2, 3);
  TrackTuplePtr sTuple3 = std::make_shared<TrackTuple>(1, 3, 4);
  TrackTuplePtr sTuple4 = std::make_shared<TrackTuple>(1, 4, 5);
  TrackTuplePtr sTuple5 = std::make_shared<TrackTuple>(1, 5, 7);
  TrackTuplePtr sTuple6 = std::make_shared<TrackTuple>(1, 6, 6);
  TrackTuplePtr sTuple7 = std::make_shared<TrackTuple>(1, 7, 8);
  TrackTuplePtr sTuple8 = std::make_shared<TrackTuple>(1, 8, 9);

  rTuple1->streamId = 1;
  rTuple2->streamId = 1;
  rTuple3->streamId = 1;
  rTuple4->streamId = 1;
  rTuple5->streamId = 1;
  rTuple6->streamId = 1;
  rTuple7->streamId = 1;
  rTuple8->streamId = 1;
  sTuple1->streamId = 2;
  sTuple2->streamId = 2;
  sTuple3->streamId = 2;
  sTuple4->streamId = 2;
  sTuple5->streamId = 2;
  sTuple6->streamId = 2;
  sTuple7->streamId = 2;
  sTuple8->streamId = 2;

  synchronizer->synchronizeStream(rTuple1);
  REQUIRE(synchronizer->synBufferMap[1].size() == 1);
  REQUIRE(synchronizer->synBufferMap[1].top()->eventTime == 1);

  synchronizer->synchronizeStream(rTuple2);
  REQUIRE(synchronizer->synBufferMap[1].size() == 2);
  REQUIRE(synchronizer->synBufferMap[1].top()->eventTime == 1);

  synchronizer->synchronizeStream(rTuple3);
  REQUIRE(synchronizer->synBufferMap[1].size() == 3);
  REQUIRE(synchronizer->synBufferMap[1].top()->eventTime == 1);

  synchronizer->synchronizeStream(sTuple1);
  REQUIRE(synchronizer->synBufferMap[1].size() == 2);
  REQUIRE(synchronizer->synBufferMap[1].top()->eventTime == 3);
  REQUIRE(synchronizer->synBufferMap[2].size() == 0);

  synchronizer->synchronizeStream(sTuple2);
  REQUIRE(synchronizer->synBufferMap[1].size() == 1);
  REQUIRE(synchronizer->synBufferMap[1].top()->eventTime == 4);
  REQUIRE(synchronizer->synBufferMap[2].size() == 0);

  synchronizer->synchronizeStream(rTuple4);
  REQUIRE(synchronizer->synBufferMap[1].size() == 2);
  REQUIRE(synchronizer->synBufferMap[2].size() == 0);

  synchronizer->synchronizeStream(rTuple5);
  REQUIRE(synchronizer->synBufferMap[2].size() == 0);
  REQUIRE(synchronizer->synBufferMap[2].top()->eventTime == 3);

  synchronizer->synchronizeStream(sTuple3);
  REQUIRE(synchronizer->synBufferMap[1].size() == 2);
  REQUIRE(synchronizer->synBufferMap[1].top()->eventTime == 5);

}*/

TEST_CASE("Test MSWJ running on random, watermarkTime = 10", "[short]")
{
  int a = 0;
  string configName, outPrefix = "";
  configName = "config_IMA.csv";
  ConfigMapPtr cfg = newConfigMap();
  cfg->fromFile(configName);
  cfg->edit("operator", "MSWJ");
  cfg->edit("watermarkTimeMs", (uint64_t) 7);
  a = runTestBenchAdj(cfg, configName, outPrefix);
  REQUIRE(a == 1);
}
/*
TEST_CASE("Test MSWJ running on random, watermarkTime = 10, with compensation", "[short]")
{
  int a = 0;
  string configName, outPrefix = "";
  configName = "config_IMA.csv";
  ConfigMapPtr cfg = newConfigMap();
  cfg->fromFile(configName);
  cfg->edit("operator", "MSWJ");
  cfg->edit("mswjCompensation", (uint64_t) 1);
  cfg->edit("watermarkTimeMs", (uint64_t) 10);
  a = runTestBenchAdj(cfg, configName, outPrefix);
  REQUIRE(a == 1);
}

TEST_CASE("Test MSWJ running on random, watermarkTime = 12", "[short]")
{
  int a = 0;
  string configName, outPrefix = "";
  configName = "config_IMA.csv";
  ConfigMapPtr cfg = newConfigMap();
  cfg->fromFile(configName);
  cfg->edit("operator", "MSWJ");
  cfg->edit("watermarkTimeMs", (uint64_t) 12);
  a = runTestBenchAdj(cfg, configName, outPrefix);
  REQUIRE(a == 1);
}

TEST_CASE("Test MSWJ running on random, watermarkTime = 14", "[short]")
{
  int a = 0;
  string configName, outPrefix = "";
  configName = "config_IMA.csv";
  ConfigMapPtr cfg = newConfigMap();
  cfg->fromFile(configName);
  cfg->edit("operator", "MSWJ");
  cfg->edit("watermarkTimeMs", (uint64_t) 14);
  a = runTestBenchAdj(cfg, configName, outPrefix);
  REQUIRE(a == 1);
}

TEST_CASE("Test MSWJ running on random with compensation, watermarkTime = 14", "[short]")
{
  int a = 0;
  string configName, outPrefix = "";
  configName = "config_IMA.csv";
  ConfigMapPtr cfg = newConfigMap();
  cfg->fromFile(configName);
  cfg->edit("operator", "MSWJ");
  cfg->edit("mswjCompensation", (uint64_t) 1);
  cfg->edit("watermarkTimeMs", (uint64_t) 14);
  a = runTestBenchAdj(cfg, configName, outPrefix);
  REQUIRE(a == 1);
}

TEST_CASE("Test MSWJ running on random, watermarkTime = 16", "[short]")
{
  int a = 0;
  string configName, outPrefix = "";
  configName = "config_IMA.csv";
  ConfigMapPtr cfg = newConfigMap();
  cfg->fromFile(configName);
  cfg->edit("operator", "MSWJ");
  cfg->edit("watermarkTimeMs", (uint64_t) 16);
  a = runTestBenchAdj(cfg, configName, outPrefix);
  REQUIRE(a == 1);
}

TEST_CASE("Test MSWJ joinSumResult running on random, watermarkTime = 16", "[short]")
{
  int a = 0;
  string configName, outPrefix = "";
  configName = "config_IMA.csv";
  ConfigMapPtr cfg = newConfigMap();
  cfg->fromFile(configName);
  cfg->edit("operator", "MSWJ");
  cfg->edit("joinSum", (uint64_t) 1);
  cfg->edit("watermarkTimeMs", (uint64_t) 16);
  a = runTestBenchAdj(cfg, configName, outPrefix);
  REQUIRE(a == 1);
}

TEST_CASE("Test MSWJ running on dataLoader, windowLenMs = 2000, watermarkTime = 200", "[short]")
{
  int a = 0;
  string configName, outPrefix = "";
  configName = "config_IMA.csv";
  ConfigMapPtr cfg = newConfigMap();
  cfg->fromFile(configName);
  cfg->edit("windowLenMs", (uint64_t) 2000);
  cfg->edit("operator", "MSWJ");
  cfg->edit("watermarkTimeMs", (uint64_t) 200);
  cfg->edit("dataLoader", "file");
  //Dataset files
  cfg->edit("fileDataLoader_rFile", "../datasets/sb_1000ms_1tMidDelayData.csv");
  cfg->edit("fileDataLoader_sFile", "../datasets/cj_1000ms_1tLowDelayData.csv");
  a = runTestBenchAdj(cfg, configName, outPrefix);
  REQUIRE(a == 1);
}

TEST_CASE("Test MSWJ running on dataLoader, windowLenMs = 2000, watermarkTime = 500", "[short]")
{
  int a = 0;
  string configName, outPrefix = "";
  configName = "config_IMA.csv";
  ConfigMapPtr cfg = newConfigMap();
  cfg->fromFile(configName);
  cfg->edit("windowLenMs", (uint64_t) 2000);
  cfg->edit("operator", "MSWJ");
  cfg->edit("watermarkTimeMs", (uint64_t) 500);
  cfg->edit("dataLoader", "file");
  //Dataset files
  cfg->edit("fileDataLoader_rFile", "../datasets/sb_1000ms_1tMidDelayData.csv");
  cfg->edit("fileDataLoader_sFile", "../datasets/cj_1000ms_1tLowDelayData.csv");
  a = runTestBenchAdj(cfg, configName, outPrefix);
  REQUIRE(a == 1);
}

TEST_CASE("Test MSWJ running on crazy random dataset, windowLenMs = 10, watermarkTime = 200", "[short]")
{
  int a = 0;
  string configName, outPrefix = "";
  configName = "config_IMA.csv";
  ConfigMapPtr cfg = newConfigMap();
  cfg->fromFile(configName);
  cfg->edit("windowLenMs", (uint64_t) 10);
  cfg->edit("operator", "MSWJ");
  cfg->edit("watermarkTimeMs", (uint64_t) 200);
  cfg->edit("maxArrivalSkewMs", (uint64_t) 1000);
  a = runTestBenchAdj(cfg, configName, outPrefix);
  REQUIRE(a == 1);
}

TEST_CASE("Test MSWJ running on dataLoader for joinSum result , windowLenMs = 2000, watermarkTime = 200", "[short]")
{
  int a = 0;
  string configName, outPrefix = "";
  configName = "config_IMA.csv";
  ConfigMapPtr cfg = newConfigMap();
  cfg->fromFile(configName);
  cfg->edit("windowLenMs", (uint64_t) 2000);
  cfg->edit("operator", "MSWJ");
  cfg->edit("watermarkTimeMs", (uint64_t) 200);
  cfg->edit("dataLoader", "file");
  cfg->edit("joinSum", (uint64_t) 1);
  //Dataset files
  cfg->edit("fileDataLoader_rFile", "../datasets/sb_1000ms_1tMidDelayData.csv");
  cfg->edit("fileDataLoader_sFile", "../datasets/cj_1000ms_1tLowDelayData.csv");
  a = runTestBenchAdj(cfg, configName, outPrefix);
  REQUIRE(a == 1);
}
*/