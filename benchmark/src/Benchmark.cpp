// Copyright (C) 2021 by the IntelliStream team (https://github.com/intellistream)

/**
 * @brief This is the main entry point of the entire program.
 * We use this as the entry point for benchmarking.
 */
#include <Utils/Logger.hpp>
#include <Utils/UtilityFunctions.hpp>
#include <Common/Types.h>
#include <Common/DatasetTool.h>
#include <TestBench/OneWayHashJoin.h>
#include <TestBench/CellJoin.h>
#include <TestBench/HandShakeJoin.h>
#include <Utils/SPSCQueue.hpp>
#include <Utils/MicroDataSet.hpp>
#include  <Utils/ThreadPerf.h>
#include <WindowSlider/AbstractEagerWS.h>

#include <JoinAlgo/JoinAlgoTable.h>
#include <Utils/C20Buffers.hpp>
#include <WindowSlider/AbstractLazyWS.h>
#include <JoinProcessor/AbstractLazyJP.h>
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

  AbstractJoinMethod<AbstractEagerWS> lwj;
 // INTELLI::UtilityFunctions::timerStart(joinResult);
  lwj.test(joinResult,relationCouple);

  //Print result number
 // INTELLI::UtilityFunctions::timerEnd(joinResult);
  joinResult.statPrinter();
  /*MicroDataSet mr(999);
  DatasetTool dataSet;
  size_t dLen = 100000;
  TuplePtrQueue tr = newTuplePtrQueue(dLen);
  TuplePtrQueue ts = newTuplePtrQueue(dLen);
  ThreadPerf tp(-1, {PerfPair(COUNT_HW_CPU_CYCLES, "Cycles"), \
                    PerfPair(COUNT_HW_INSTRUCTIONS, "instructions")
                }
  );
  tp.start();
  dataSet.combine3VVector(tr, mr.genRandInt<keyType>(dLen, 5000, 0),
                          vector<valueType>(dLen),
                          mr.genZipfTimeStamp<size_t>(dLen, 5000, 0.2)
  );
  dataSet.store3VText(tr, "randKey-zipfTR.txt");
  MicroDataSet ms = MicroDataSet(996);
  dataSet.combine3VVector(ts, ms.genRandInt<keyType>(dLen, 5000, 0),
                          vector<valueType>(500),
                          ms.genZipfTimeStamp<size_t>(dLen, 5000, 0.2)
  );
  dataSet.store3VText(ts, "randKey-zipfTS.txt");
  tp.end();
  cout << tp.headStringPrintf() << endl;
  cout << tp.resultStringPrintf() << endl;
 // cout << tp.getResultByName("instructions") << endl;
  JoinAlgoTable jat;

  tp.start();
  auto npj0=jat.findAlgo(JOINALGO_NPJ);
  cout<<npj0->getAlgoName()<<endl;
  size_t matches=npj0->join(ts,tr,1);
  tp.end();
  //printf("single thread, %ld us,%ld matches\r\n",tp.getResultById(-1),matches);
  cout<<"1 thread,"+ to_string(tp.getResultById(-1))+"us "+ to_string(matches)+" matches"<<endl;
  int ths=8;
  tp.start();
  matches=npj0->join(ts,tr,ths);
  tp.end();
  cout<<to_string(ths)+" thread,"+ to_string(tp.getResultById(-1))+"us "+ to_string(matches)+" matches"<<endl;
  cout<<matches<<endl;


  //vector <int32_t> ru=ms.genRandInt<int32_t>(50,10,-10);
  //cout<<sizeof(MtBucket)<<endl;
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