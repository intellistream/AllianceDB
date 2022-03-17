//
// Created by tony on 11/03/22.
//
#include <JoinAlgo/JoinAlgoTable.h>
#include <JoinAlgo/NPJ.h>
using namespace INTELLI;
JoinAlgoTable::JoinAlgoTable() {
//  NPJ npj;
  algos = {newAbstractJoinAlgo(), \
          newNPJ(), \
         newNPJSingle()
  };
  //cout<<algos[1]->getAlgoName()<<endl;
}
