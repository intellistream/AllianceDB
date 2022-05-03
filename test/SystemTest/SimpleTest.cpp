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
#include <Engine/Verifier.hpp>
#include <Common/Result.hpp>

TEST(SystemTest, SimpleTest
) {
//  setupLogging("benchmark.log", LOG_DEBUG);
//  INTELLI::Result joinResult = INTELLI::Result();
//  INTELLI::Relations relationCouple = INTELLI::Relations();
//  string pwd = getcwd(NULL, 0); //Get current directory
//  string fileRName = pwd + "/datasets/" + DATASET_NAME + "-R.txt";
//  string fileSName = pwd + "/datasets/" + DATASET_NAME + "-S.txt";
//  INTELLI::DatasetTool dataSet;
//  dataSet.LoadData(relationCouple.relationR, fileRName);
//  dataSet.LoadData(relationCouple.relationS, fileSName);
////  joinResult.streamSize = relationCouple.relationR.size();
//
//  auto verifyResult = Result::create();
//
//  Verifier vb_lwj;
//  vb_lwj.Run(verifyResult, relationCouple, 1, 500, 500);
}