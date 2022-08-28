//
// Created by tony on 11/03/22.
//
#include <JoinAlgo/JoinAlgoTable.h>
#include <JoinAlgo/NestedLoopJoin.h>
#include <JoinAlgo/NPJ.h>
using namespace AllianceDB;
JoinAlgoTable::JoinAlgoTable() {
//  NPJ npj;
  algos = {newAbstractJoinAlgo(), \
          newNestedLoopJoin(), \
          newNPJ(), \
         newNPJSingle()
  };
  //cout<<algos[1]->getAlgoName()<<endl;
}
