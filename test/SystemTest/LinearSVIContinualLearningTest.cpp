#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include <Common/LinearVAE.h>
#include <vector>
#include <OoOJoin.h>
#include "TestFunction.cpp"
/*TEST_CASE("Test AIOperator on Pretrain mode")
{
  int a = 1;
  string configName, outPrefix = "";
  configName = "config_AI_TRAIN.csv";
  ConfigMapPtr cfg = newConfigMap();
  cfg->fromFile(configName);
  cfg->edit("operator", "AI");
  cfg->edit("aiMode", "pretrain");
  cfg->edit("appendSel", (uint64_t) 1);
  cfg->edit("appendSkew", (uint64_t) 1);
  cfg->edit("appendRate", (uint64_t) 1);
  cfg->edit("exitAfterPretrain",(uint64_t)1);
  runTestBenchPretrain(cfg, configName, outPrefix);
  REQUIRE(a == 1);
}*/

TEST_CASE("Test LInearSVIOperator on continual learning mode")
{
  int a = 1;
  string configName, outPrefix = "";
  configName = "config_AI_INFERENCE.csv";
  ConfigMapPtr cfg = newConfigMap();
  cfg->fromFile(configName);
  cfg->edit("operator", "LinearSVI");
  cfg->edit("aiMode", "continual_learning");
  cfg->edit("appendSel", (uint64_t) 0);
  cfg->edit("appendSkew", (uint64_t) 0);
  cfg->edit("appendRate", (uint64_t) 0);
  runTestBenchAdj(cfg, configName, outPrefix);
  REQUIRE(a == 1);
}

