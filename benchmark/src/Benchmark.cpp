// Copyright (C) 2021 by the IntelliStream team (https://github.com/intellistream)

/**
 * @brief This is the main entry point of the entire program.
 * We use this as the entry point for benchmarking.
 */
#include <Utils/Logger.hpp>
#include <Utils/UtilityFunctions.hpp>
#include <Common/Types.hpp>
#include <Common/DatasetTool.hpp>
#include <Utils/MicroDataSet.hpp>
#include <Common/Result.hpp>
#include <Common/Relations.hpp>
#include <WindowSlider/AbstractLazyWS.h>

#include <Engine/Verifier.hpp>
#include <Engine/Executor.hpp>

using namespace std;
using namespace INTELLI;

int main() {
  //Setup Logs
  setupLogging("benchmark.log", LOG_DEBUG);
  //Load Inputs
  auto relations = Relations::create();
  string pwd = getcwd(NULL, 0); //Get current directory
  string fileRName = pwd + "/datasets/" + DATASET_NAME + "-R.txt";
  string fileSName = pwd + "/datasets/" + DATASET_NAME + "-S.txt";
  DatasetTool dataSet;
  dataSet.LoadData(relations->relationR, fileRName);
  dataSet.LoadData(relations->relationS, fileSName);

  auto exeResult = Result::create();
  Executor<AbstractLazyWS> executor;
//  executor.Run(exeResult, relations, 2, 500, 200);
  executor.Run(exeResult, relations);
  auto verifyResult = Result::create();
  Verifier verifier;
//  verifier.Run(verifyResult, relations, 1, 500, 200);
  verifier.Run(verifyResult, relations);
  if (exeResult->joinNumber == verifyResult->joinNumber) {
    cout << "Congratulations, the result " + to_string(exeResult->joinNumber) + " is correct!" << endl;
    return true;
  } else {
    cout << "Ops, ot matched, expecting " + to_string(verifyResult->joinNumber) + "but return "
        + to_string(exeResult->joinNumber) << endl;
    return false;
  }
}