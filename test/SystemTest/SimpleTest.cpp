// Copyright (C) 2021 by the INTELLI team (https://github.com/intellistream)

#include <filesystem>
#include <gtest/gtest.h>
#include <Utils/Logger.hpp>
#include <Utils/UtilityFunctions.hpp>
#include <Common/Types.h>
#include <JoinMethods/CommonFunctions.h>
#include <JoinMethods/OneWayHashJoin.h>
#include <JoinMethods/CellJoin.h>
#include <JoinMethods/HandShakeJoin.h>

using namespace std;

TEST(SystemTest, SimpleTest
) {
//Setup Logs
  setupLogging("benchmark.log", LOG_DEBUG);
  INTELLI::Result joinResult = INTELLI::Result();
  INTELLI::RelationCouple relationCouple = INTELLI::RelationCouple();
  string pwd = getcwd(NULL, 0); //Get current directory
  string fileRName = pwd + "/datasets/" + DATASET_NAME + "-R.txt";
  string fileSName = pwd + "/datasets/" + DATASET_NAME + "-S.txt";
  INTELLI::CommonFunction CommonFunction;
  CommonFunction.
      buildRelation(relationCouple
                        .relationR, fileRName);
  CommonFunction.
      buildRelation(relationCouple
                        .relationS, fileSName);
  joinResult.
      streamSize = relationCouple.relationR.size();
  INTELLI::UtilityFunctions::timerStart(joinResult);
  INTELLI::ALGO_CLASS::EXEC;
//Print result number
  INTELLI::UtilityFunctions::timerEnd(joinResult);
  joinResult.
      statPrinter();
}