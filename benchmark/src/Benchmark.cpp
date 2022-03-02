// Copyright (C) 2021 by the IntelliStream team (https://github.com/intellistream)

/**
 * @brief This is the main entry point of the entire program.
 * We use this as the entry point for benchmarking.
 */
#include <Utils/Logger.hpp>
#include <Utils/UtilityFunctions.hpp>
#include <Common/Types.h>
#include <JoinMethods/CommonFunctions.h>
#include <JoinMethods/OneWayHashJoin.h>
#include <JoinMethods/CellJoin.h>
#include <JoinMethods/HandShakeJoin.h>
#include <Utils/SPSCQueue.hpp>
#include <WindowSlider/AbstractEagerWS.h>
using namespace std;
using namespace INTELLI;

int main() {
  //Setup Logs
  setupLogging("benchmark.log", LOG_DEBUG);
  INTELLI::Result joinResult = INTELLI::Result();
  INTELLI::RelationCouple relationCouple = INTELLI::RelationCouple();
  string pwd = getcwd(NULL, 0); //Get current directory
  string fileRName = pwd + "/datasets/" + DATASET_NAME + "-R.txt";
  string fileSName = pwd + "/datasets/" + DATASET_NAME + "-S.txt";
  INTELLI::CommonFunction CommonFunction;
  CommonFunction.buildRelation(relationCouple.relationR, fileRName);
  CommonFunction.buildRelation(relationCouple.relationS, fileSName);
  joinResult.streamSize = relationCouple.relationR.size();
  INTELLI::UtilityFunctions::timerStart(joinResult);
  INTELLI::ALGO_CLASS::TESTMODULE;
  //INTELLI::ALGO_CLASS::EXEC;
  //Print result number
  INTELLI::UtilityFunctions::timerEnd(joinResult);
  joinResult.statPrinter();
  /*AbstractEagerWS es(100,100);
  es.setParallelSMP(10);
  vector<size_t >a=es.avgPartitionSizeFinal(1010);
  for(size_t i=0;i<10;i++)
  {
    cout<<a[i]<<endl;
  }
  es.initJoinProcessors();
//sleep(1);
  es.terminateJoinProcessors();*/
  
  return 0;
}