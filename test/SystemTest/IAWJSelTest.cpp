#define CATCH_CONFIG_MAIN
//do not change these includes and name spaces
#include "catch.hpp"
//#include <Common/LinearVAE.h>
#include <vector>
#include <OoOJoin.h>
#include "TestFunction.cpp"
using namespace std;
using namespace OoOJoin;
//using namespace MSWJ;

TEST_CASE("Test IAWJSel running on random, watermarkTime = 7", "[short]")
{
  int a = 0;
  string configName, outPrefix = "";
  configName = "config_Normal.csv";
  ConfigMapPtr cfg = newConfigMap();
  cfg->fromFile(configName);
  cfg->edit("operator", "IAWJSel");
  cfg->edit("watermarkTimeMs", (uint64_t) 7);
  a = runTestBenchAdj(cfg, configName, outPrefix);
  REQUIRE(a == 1);
}

TEST_CASE("Test IAWJ running on random, watermarkTime = 10", "[short]")
{
  int a = 0;
  string configName, outPrefix = "";
  configName = "config_Normal.csv";
  ConfigMapPtr cfg = newConfigMap();
  cfg->fromFile(configName);
  cfg->edit("operator", "IAWJSel");
  cfg->edit("watermarkTimeMs", (uint64_t) 10);
  a = runTestBenchAdj(cfg, configName, outPrefix);
  REQUIRE(a == 1);
}

TEST_CASE("Test IAWJ running on random, watermarkTime = 12", "[short]")
{
  int a = 0;
  string configName, outPrefix = "";
  configName = "config_Normal.csv";
  ConfigMapPtr cfg = newConfigMap();
  cfg->fromFile(configName);
  cfg->edit("operator", "IAWJSel");
  cfg->edit("watermarkTimeMs", (uint64_t) 12);
  a = runTestBenchAdj(cfg, configName, outPrefix);
  REQUIRE(a == 1);
}

TEST_CASE("Test IAWJ running on random, watermarkTime = 14", "[short]")
{
  int a = 0;
  string configName, outPrefix = "";
  configName = "config_Normal.csv";
  ConfigMapPtr cfg = newConfigMap();
  cfg->fromFile(configName);
  cfg->edit("operator", "IAWJSel");
  cfg->edit("watermarkTimeMs", (uint64_t) 14);
  a = runTestBenchAdj(cfg, configName, outPrefix);
  REQUIRE(a == 1);
}

TEST_CASE("Test IAWJ running on random, watermarkTime = 16", "[short]")
{
  int a = 0;
  string configName, outPrefix = "";
  configName = "config_Normal.csv";
  ConfigMapPtr cfg = newConfigMap();
  cfg->fromFile(configName);
  cfg->edit("operator", "IAWJSel");
  cfg->edit("watermarkTimeMs", (uint64_t) 16);
  a = runTestBenchAdj(cfg, configName, outPrefix);
  REQUIRE(a == 1);
}
