#define CATCH_CONFIG_MAIN
#include "catch.hpp"
//#include <Common/LinearVAE.h>
#include <vector>
#include <OoOJoin.h>
//using namespace std;
using namespace OoOJoin;
//using namespace MSWJ;
#include "TestFunction.cpp"
TEST_CASE("Test IMA running on random, watermarkTime = 7", "[short]")
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

TEST_CASE("Test IMA running on random, watermarkTime = 10", "[short]")
{
  int a = 0;
  string configName, outPrefix = "";
  configName = "config_IMA.csv";
  ConfigMapPtr cfg = newConfigMap();
  cfg->fromFile(configName);
  cfg->edit("watermarkTimeMs", (uint64_t) 10);
  a = runTestBenchAdj(cfg, configName, outPrefix);
  REQUIRE(a == 1);
}

TEST_CASE("Test IMA running on random, watermarkTime = 12", "[short]")
{
  int a = 0;
  string configName, outPrefix = "";
  configName = "config_IMA.csv";
  ConfigMapPtr cfg = newConfigMap();
  cfg->fromFile(configName);
  cfg->edit("watermarkTimeMs", (uint64_t) 12);
  a = runTestBenchAdj(cfg, configName, outPrefix);
  REQUIRE(a == 1);
}

TEST_CASE("Test IMA running on random, watermarkTime = 14", "[short]")
{
  int a = 0;
  string configName, outPrefix = "";
  configName = "config_IMA.csv";
  ConfigMapPtr cfg = newConfigMap();
  cfg->fromFile(configName);
  cfg->edit("watermarkTimeMs", (uint64_t) 14);
  a = runTestBenchAdj(cfg, configName, outPrefix);
  REQUIRE(a == 1);
}

TEST_CASE("Test IMA running on random, watermarkTime = 16", "[short]")
{
  int a = 0;
  string configName, outPrefix = "";
  configName = "config_IMA.csv";
  ConfigMapPtr cfg = newConfigMap();
  cfg->fromFile(configName);
  cfg->edit("watermarkTimeMs", (uint64_t) 16);
  a = runTestBenchAdj(cfg, configName, outPrefix);
  REQUIRE(a == 1);
}
