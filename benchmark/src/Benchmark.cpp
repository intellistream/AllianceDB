// Copyright (C) 2021 by the IntelliStream team (https://github.com/intellistream)

/**
 * @brief This is the main entry point of the entire program.
 * We use this as the entry point for benchmarking.
 */
#include <Utils/Logger.hpp>
#include <Utils/UtilityFunctions.hpp>
#include <Common/Types.h>
#include <Common/DatasetTool.h>
#include <JoinMethods/OneWayHashJoin.h>
#include <JoinMethods/CellJoin.h>
#include <JoinMethods/HandShakeJoin.h>
#include <Utils/SPSCQueue.hpp>
#include <Utils/MicroDataSet.h>
#include <WindowSlider/AbstractEagerWS.h>
using namespace std;
using namespace INTELLI;

#ifndef EXEC
#define EXEC execute(joinResult, relationCouple)
#endif

#ifndef TESTMODULE
#define TESTMODULE test(joinResult, relationCouple)
#endif
int main() {
  //Setup Logs
  /*setupLogging("benchmark.log", LOG_DEBUG);
  INTELLI::Result joinResult = INTELLI::Result();
  INTELLI::RelationCouple relationCouple = INTELLI::RelationCouple();
  string pwd = getcwd(NULL, 0); //Get current directory
  string fileRName = pwd + "/datasets/" + DATASET_NAME + "-R.txt";
  string fileSName = pwd + "/datasets/" + DATASET_NAME + "-S.txt";
  INTELLI::DatasetTool dataSet;
  dataSet.load3VText(relationCouple.relationR, fileRName);
  dataSet.load3VText(relationCouple.relationS, fileSName);
  joinResult.streamSize = relationCouple.relationR.size();
  INTELLI::UtilityFunctions::timerStart(joinResult);
  INTELLI::ALGO_CLASS::TESTMODULE;
  INTELLI::ALGO_CLASS::EXEC;
  //Print result number
  INTELLI::UtilityFunctions::timerEnd(joinResult);
  joinResult.statPrinter();*/
 MicroDataSet mr(999);
  DatasetTool dataSet;
  size_t dLen=10000;
  TupleQueuePtr tr= newTupleQueuePtr(dLen);
  TupleQueuePtr ts= newTupleQueuePtr(dLen);
  dataSet.combine3VVector(tr, mr.genRandInt<keyType>(dLen,50000,0),
                              vector<valueType>(dLen),
                          mr.genZipfTimeStamp<size_t>(dLen,5000,0.2)
                          );
  dataSet.store3VText(tr,"randKey-zipfTR.txt");
  MicroDataSet ms=MicroDataSet(996);
  dataSet.combine3VVector(ts, ms.genRandInt<keyType>(dLen,50000,0),
                          vector<valueType>(500),
                          ms.genZipfTimeStamp<size_t>(dLen,5000,0.2)
  );
  dataSet.store3VText(ts,"randKey-zipfTS.txt");


  //vector <int32_t> ru=ms.genRandInt<int32_t>(50,10,-10);

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