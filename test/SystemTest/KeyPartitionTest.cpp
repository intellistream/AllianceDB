//
// Created by tony on 27/06/23.
//
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
//#include <Common/LinearVAE.h>
#include <vector>
#include <OoOJoin.h>
#include <string>
//using namespace std;
using namespace OoOJoin;
#include "TestFunction.cpp"
TEST_CASE("Test SINGLE thread IMA running on random, watermarkTime = 7", "[short]")
{
  int a = 0;
  string configName, outPrefix = "";
  configName = "config_IMA.csv";
  ConfigMapPtr cfg = newConfigMap();
  cfg->fromFile(configName);
  cfg->edit("watermarkTimeMs", (uint64_t) 7);
  a = runTestBenchAdj(cfg, configName, outPrefix);
  REQUIRE(a == 1);
}
TEST_CASE("Test key partition running on random, watermarkTime = 10", "[short]")
{
  int a = 0;
  string configName, outPrefix = "";
  configName = "config_IMA.csv";
  /**
   * @brief pre process on the units
   */
  tsType windowLenMs, timeStepUs, watermarkTimeMs;
  ConfigMapPtr cfg = newConfigMap();
  cfg->fromFile(configName);
  cfg->edit("watermarkTimeMs", (uint64_t) 10);
  cfg->edit("operator", "IAWJSel");
  windowLenMs = cfg->tryU64("windowLenMs", 10, true);
  timeStepUs = cfg->tryU64("timeStepUs", 40, true);
  watermarkTimeMs = cfg->tryU64("watermarkTimeMs", 10, true);
  std::string tag = cfg->tryString("dataLoader", "random");
  DataLoaderTablePtr dt = newDataLoaderTable();

  AbstractDataLoaderPtr dl = dt->findDataLoader(tag);
  dl->setConfig(cfg);
  OoOJoin::KeyPartitionRunner kr;

  cfg->edit("threads", (uint64_t) 2);

  cfg->edit("rLen", (uint64_t) dl->getTupleVectorS().size());
  cfg->edit("sLen", (uint64_t) dl->getTupleVectorR().size());

  //Global configs
  cfg->edit("windowLen", (uint64_t) windowLenMs * 1000);
  cfg->edit("timeStep", (uint64_t) timeStepUs);
  cfg->edit("watermarkTime", (uint64_t) watermarkTimeMs * 1000);
  kr.setConfig(cfg);
  kr.setDataSet(dl->getTupleVectorS(), dl->getTupleVectorR());
  /*gettimeofday(&timeStart, nullptr);
  kr.syncTime(timeStart);*/
  kr.runStreaming();
  std::cout << "joined =" + to_string(kr.getResult()) << std::endl;
  std::cout << "aqp joined =" + to_string(kr.getAQPResult()) << std::endl;
  std::cout << "throughput= =" + to_string(kr.getThroughput()) << std::endl;
  a = 1;
  REQUIRE(a == 1);
}