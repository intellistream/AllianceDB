// Copyright (C) 2021 by the INTELLI team (https://github.com/intellistream)

#include <filesystem>
#include <gtest/gtest.h>
#include <Utils/Logger.hpp>
#include <Common/Types.hpp>
#include <Utils/MicroDataSet.hpp>
#include <Common/DatasetTool.hpp>
using namespace std;
using namespace std;
using namespace INTELLI;
#include <WindowSlider/AbstractLazyWS.h>

#include <Common/Verify.h>
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
  Verify vb_lwj;
  vb_lwj.Run(joinResult, relationCouple, 500, 500);

}