//
// Created by tony on 04/03/22.
//
#include <JoinAlgo/NPJ.h>
#include <Utils/UtilityFunctions.hpp>
#include <Utils/ThreadPerf.h>

using namespace INTELLI;
void NPJ_thread::inlineMain() {
  //first bind to the core
  // UtilityFunctions::bind2Core(cpu);
  //build S
  ThreadPerf tp(cpu);
  tp.start();
  table->buildTable(ts, sLen);
  //wait for the bar
  waitBuildBar();
  tp.end();
  /*if (cpu == 0) {
    cout << ("build time ") + to_string(tp.getResultById(-1)) << endl;
  }*/
  //probe r
  tp.start();
  for (size_t i = 0; i < rLen; i++) {
    result += table->probeTuple(tr[i]);
  }
  tp.end();
  /*if (cpu == 0) {
    cout << ("probe time ") + to_string(tp.getResultById(-1)) << endl;
  }*/
  //result=500;
}
size_t NPJ::join(TuplePtrQueue ts, TuplePtrQueue tr, int threads) {
  return join(ts->front(), tr->front(), ts->size(), tr->size(), threads);
}
size_t NPJ::join(TuplePtr *ts, TuplePtr *tr, size_t tsLen, size_t trLen, int threads) {
  size_t ru = 0;

  ThreadPerf tp(-1);
  //cout<<this->getAlgoName()+"will run"<<endl;
  tp.start();
  workers = vector<NPJ_thread>(threads);
  //tp.start();
  // SMP-based, average partition
  vector<size_t> partitionS = UtilityFunctions::avgPartitionSizeFinal(tsLen, vector<size_t>(threads));
  vector<size_t> partitionR = UtilityFunctions::avgPartitionSizeFinal(trLen, vector<size_t>(threads));

  size_t sBegin = 0;
  size_t rBegin = 0;
  //creat table

  MultiThreadHashTablePtr table = make_shared<MultiThreadHashTable>(tsLen / 2 + 1); // at least there is one element
  tp.end();
  // create barrier

  INTELLI::BarrierPtr buildBar = std::make_shared<std::barrier<>>(threads);
  //cout << ("init time ") + to_string(tp.getResultById(-1)) << endl;
  for (size_t i = 0; i < threads; i++) {
    workers[i].init(&ts[sBegin], \
                    &tr[rBegin], \
                    partitionS[i], \
                    partitionR[i], \
                    i, \
                    table, \
                    buildBar
    );
    sBegin += partitionS[i];
    rBegin += partitionR[i];
  }
  for (size_t i = 0; i < threads; i++) {
    workers[i].startThread();
  }
  tp.end();

  for (size_t i = 0; i < threads; i++) {
    workers[i].joinThread();
    ru += workers[i].getResult();
  }
  return ru;
}
// one batch one tuple, only parallel build
size_t NPJ::join(TuplePtr *ts, TuplePtr tr, size_t tsLen, int threads) {
  size_t ru = 0;

  workers = vector<NPJ_thread>(threads);
  //tp.start();
  // SMP-based, average partition
  vector<size_t> partitionS = UtilityFunctions::avgPartitionSizeFinal(tsLen, vector<size_t>(threads));
  size_t sBegin = 0;
  //creat table
  MultiThreadHashTablePtr table = make_shared<MultiThreadHashTable>(tsLen / 2 + 1);
  // create barrier
  INTELLI::BarrierPtr buildBar = std::make_shared<std::barrier<>>(threads);
  //cout << ("init time ") + to_string(tp.getResultById(-1)) << endl;
  for (size_t i = 0; i < threads; i++) {
    workers[i].init(&ts[sBegin], \
                    nullptr, \
                    partitionS[i], \
                    0, \
                    i, \
                    table, \
                    buildBar
    );// no probe
    sBegin += partitionS[i];
  }
  for (size_t i = 0; i < threads; i++) {
    workers[i].startThread();
  }
  //tp.end();
  for (size_t i = 0; i < threads; i++) {
    workers[i].joinThread();
  }
  ru = table->probeTuple(tr);
  return ru;
}
size_t NPJ::join(TuplePtrQueue ts, TuplePtr tr, int threads) {
  return join(ts->front(), tr, ts->size(), threads);
}
size_t NPJSingle::join(TuplePtr *ts, TuplePtr *tr, size_t tsLen, size_t trLen, int threads) {
  assert(threads > 0);
  size_t result = 0;
  MultiThreadHashTable table(tsLen / 2 + 1);
  table.buildTable(ts, tsLen);
  for (size_t i = 0; i < trLen; i++) {
    result += table.probeTuple(tr[i]);
  }
  return result;
}
size_t NPJSingle::join(TuplePtrQueue ts, TuplePtrQueue tr, int threads) {
  return join(ts->front(), tr->front(), ts->size(), tr->size(), threads);
}
size_t NPJSingle::join(TuplePtr *ts, TuplePtr tr, size_t tsLen, int threads) {
  assert(threads > 0);
  size_t result = 0;
  MultiThreadHashTable table(tsLen / 2 + 1);
  table.buildTable(ts, tsLen);
  result += table.probeTuple(tr);
  return result;
}
size_t NPJSingle::join(TuplePtrQueue ts, TuplePtr tr, int threads) {
  return join(ts->front(), tr, ts->size(), threads);
}