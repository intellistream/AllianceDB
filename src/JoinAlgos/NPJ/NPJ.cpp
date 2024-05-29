//
// Created by tony on 04/03/22.
//
#include <JoinAlgos/NPJ/NPJ.h>
#include <Utils/UtilityFunctions.hpp>
//#include <Utils/ThreadPerf.h>
using namespace OoOJoin;

void NPJ_thread::inlineMain() {
  //first bind to the core
  // UtilityFunctions::bind2Core(cpu);
  //build S
  //ThreadPerf tp(cpu);
  //tp.start();
  table->buildTable(ts, sLen);
  //wait for the bar
  waitBuildBar();
  //tp.end();
  /*if (cpu == 0) {
    cout << ("build time ") + to_string(tp.getResultById(-1)) << endl;
  }*/
  //probe r
  //tp.start();
  for (size_t i = 0; i < rLen; i++) {
    if (joinSum) {
      result += table->probeTuple(tr[i]) * tr[i]->payload;
    } else {
      result += table->probeTuple(tr[i]);
    }
    tr[i]->processedTime = UtilityFunctions::timeLastUs(timeBaseStruct);
  }
  //tp.end();
  /*if (cpu == 0) {
    cout << ("probe time ") + to_string(tp.getResultById(-1)) << endl;
  }*/
  //result=500;
}

size_t NPJ::join(C20Buffer<OoOJoin::TrackTuplePtr> windS,
                 C20Buffer<OoOJoin::TrackTuplePtr> windR, int threads) {
  size_t ru = 0;
  size_t tsLen = windS.size();
  size_t trLen = windR.size();
  NPJTuplePtr *ts = windS.data();
  NPJTuplePtr *tr = windR.data();
  //ThreadPerf tp(-1);
  //cout<<this->getAlgoName()+"will run"<<endl;
  //tp.start();
  workers = std::vector<NPJ_thread>(threads);
  //tp.start();
  // SMP-based, average partition
  std::vector<size_t> partitionS = UtilityFunctions::avgPartitionSizeFinal(tsLen, std::vector<size_t>(threads));
  std::vector<size_t> partitionR = UtilityFunctions::avgPartitionSizeFinal(trLen, std::vector<size_t>(threads));

  size_t sBegin = 0;
  size_t rBegin = 0;
  //creat table

  MultiThreadHashTablePtr
      table = std::make_shared<MultiThreadHashTable>(tsLen / 2 + 1); // at least there is one element
  //tp.end();
  // create barrier

  INTELLI::BarrierPtr buildBar = std::make_shared<std::barrier<>>(threads);
  //cout << ("init time ") + to_string(tp.getResultById(-1)) << endl;
  for (int i = 0; i < threads; i++) {
    workers[i].init(&ts[sBegin], \
                    &tr[rBegin], \
                    partitionS[i], \
                    partitionR[i], \
                    i, \
                    table, \
                    buildBar
    );
    workers[i].setJoinSum(joinSum);
    workers[i].syncTimeStruct(timeBaseStruct);
    //workers[i].setTimeStep(timeStep);
    sBegin += partitionS[i];
    rBegin += partitionR[i];
  }
  for (int i = 0; i < threads; i++) {
    workers[i].startThread();
  }
  for (int i = 0; i < threads; i++) {
    workers[i].joinThread();
    ru += workers[i].getResult();
  }
  return ru;
}

size_t NPJSingle::join(C20Buffer<OoOJoin::TrackTuplePtr> windS,
                       C20Buffer<OoOJoin::TrackTuplePtr> windR, int threads) {
  assert(threads > 0);
  size_t result = 0;
  size_t tsLen = windS.size();
  size_t trLen = windR.size();
  NPJTuplePtr *ts = windS.data();
  NPJTuplePtr *tr = windR.data();
  MultiThreadHashTable table(tsLen / 2 + 1);
  table.buildTable(ts, tsLen);
  for (size_t i = 0; i < trLen; i++) {
    if (joinSum) {
      result += table.probeTuple(tr[i]) * tr[i]->payload;
    } else {
      result += table.probeTuple(tr[i]);
    }
  }
  auto tNow=UtilityFunctions::timeLastUs(timeBaseStruct);
  processedTime=tNow;

  return result;
}