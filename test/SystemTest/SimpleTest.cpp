#include <vector>

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include <OoOJoin.h>
#include "TestFunction.cpp"

using namespace std;
using namespace OoOJoin;

TEST_CASE("Test Normal punctuation+join", "[short]")
{
  int a = 0;
  string configName = "", outPrefix = "";
  configName = "config_Normal.csv";
  ConfigMapPtr cfg = newConfigMap();
  cfg->fromFile(configName);
  a = runTestBenchAdj(cfg, configName, outPrefix);
  REQUIRE(a == 1);
}

TEST_CASE("Test Holistic punctuation+join", "[short]")
{
  int a = 0;
  string configName = "", outPrefix = "";
  configName = "config_IMA.csv";
  ConfigMapPtr cfg = newConfigMap();
  cfg->fromFile(configName);
  a = runTestBenchAdj(cfg, configName, outPrefix);
  REQUIRE(a == 1);
}

TEST_CASE("Test running on external file", "[short]")
{
  int a = 0;
  string configName = "", outPrefix = "";
  configName = "config_fileDataLoader.csv";
  ConfigMapPtr cfg = newConfigMap();
  cfg->fromFile(configName);
  a = runTestBenchAdj(cfg, configName, outPrefix);
  REQUIRE(a == 1);
}



