#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include <Common/LinearVAE.h>
#include <vector>
#include <OoOJoin.h>
//using namespace std;
using namespace OoOJoin;
//using namespace MSWJ;
#include "TestFunction.cpp"


TEST_CASE("Test LazyIAWJSel running on random, watermarkTime = 10", "[short]")
{
  int a = 0;
  string configName, outPrefix = "";
  configName = "config_Normal.csv";
  ConfigMapPtr cfg = newConfigMap();
  cfg->fromFile(configName);
  cfg->edit("watermarkTimeMs", (uint64_t) 10);
  cfg->edit("operator","LazyIAWJSel");
  a = runTestBenchAdj(cfg, configName, outPrefix);
  REQUIRE(a == 1);
}