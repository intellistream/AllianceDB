// Copyright (C) 2021 by the INTELLI team (https://github.com/intellistream)

#include <filesystem>
#include <gtest/gtest.h>
#include <Utils/Logger.hpp>
#include <Utils/UtilityFunctions.hpp>
#include <Common/Types.h>

#include <Utils/SPSCQueue.hpp>
#include <Utils/MicroDataSet.hpp>
#include  <Utils/ThreadPerf.h>
#include <WindowSlider/AbstractEagerWS.h>
#include <WindowSlider/SplitJoinWS.h>
#include "AbstractJoinMethod.h"
#include <Common/Types.h>
#include <Common/DatasetTool.h>
using namespace std;
using namespace std;
using namespace INTELLI;
#include <WindowSlider/AbstractLazyWS.h>

#include <Common/VerifyBench.h>
TEST(SystemTest, SimpleTest
) {
  setupLogging("benchmark.log", LOG_DEBUG);
  INTELLI::Result joinResult = INTELLI::Result();
  INTELLI::RelationCouple relationCouple = INTELLI::RelationCouple();
  string pwd = getcwd(NULL, 0); //Get current directory
  string fileRName = pwd + "/datasets/" + DATASET_NAME + "-R.txt";
  string fileSName = pwd + "/datasets/" + DATASET_NAME + "-S.txt";
  INTELLI::DatasetTool dataSet;
  dataSet.load3VText(relationCouple.relationR, fileRName);
  dataSet.load3VText(relationCouple.relationS, fileSName);
  joinResult.streamSize = relationCouple.relationR.size();
  VerifyBench<SplitJoinWS> vb_ewj;
  ASSERT_TRUE(vb_ewj.test(joinResult, relationCouple, 1));
  dataSet.load3VText(relationCouple.relationR, fileRName);
  dataSet.load3VText(relationCouple.relationS, fileSName);
  ASSERT_TRUE(vb_ewj.test(joinResult, relationCouple, 2));
  dataSet.load3VText(relationCouple.relationR, fileRName);
  dataSet.load3VText(relationCouple.relationS, fileSName);
  ASSERT_TRUE(vb_ewj.test(joinResult, relationCouple, 4));
  dataSet.load3VText(relationCouple.relationR, fileRName);
  dataSet.load3VText(relationCouple.relationS, fileSName);
  ASSERT_TRUE(vb_ewj.test(joinResult, relationCouple, 4,500,50));
  /* AbstractJoinMethod<AbstractEagerWS> ewj;
   // INTELLI::UtilityFunctions::timerStart(joinResult);
   ewj.test(joinResult, relationCouple);

   //Print result number
   // INTELLI::UtilityFunctions::timerEnd(joinResult);
   joinResult.statPrinter();
   ASSERT_TRUE(joinResult.joinNumber>0);*/

}