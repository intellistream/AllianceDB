//
// Created by tony on 18/03/22.
//

#include <JoinProcessor/SplitJoinJP.h>
using namespace INTELLI;
void SplitJoinJP::joinS(TuplePtr ts) {
  size_t timeNow = ts->subKey;
  expireR(timeNow);
  //single thread join window r and tuple s
  TuplePtrQueueLocalS->push(ts);
 size_t rSize = TuplePtrQueueLocalR->size();
  for (size_t i = 0; i < rSize; i++) {
    if (TuplePtrQueueLocalR->front()[i]->key == ts->key) {
      joinedResult++;
    }
  }
  /*joinedResult +=
      myAlgo->findAlgo(JOINALGO_NESTEDLOOP)->join(TuplePtrQueueLocalR->front(), ts, TuplePtrQueueLocalR->size(), 2);*/
}
void SplitJoinJP::joinR(TuplePtr tr) {
  size_t timeNow = tr->subKey;
  expireS(timeNow);
  TuplePtrQueueLocalR->push(tr);
  /*joinedResult +=
      myAlgo->findAlgo(JOINALGO_NESTEDLOOP)->join(TuplePtrQueueLocalS->front(), tr, TuplePtrQueueLocalS->size(), 2);*/
  //single thread join window s and tuple r
  size_t sSize = TuplePtrQueueLocalS->size();
   for (size_t i = 0; i < sSize; i++) {
     if (TuplePtrQueueLocalS->front()[i]->key == tr->key) {
       joinedResult++;
     }
   }

}
void SplitJoinJP::expireS(size_t cond) {
  size_t pos = 0;
  size_t windowNo = oldestWindowBelong(cond);
  size_t startTime = windowNo * slideLenGlobal;
  if (!TuplePtrQueueLocalS->empty()) {
    TuplePtr ts = *TuplePtrQueueLocalS->front();
    pos = ts->subKey;
    while (pos < startTime) {
      TuplePtrQueueLocalS->pop();
      if (!TuplePtrQueueLocalS->empty()) {
        ts = *TuplePtrQueueLocalS->front();
        pos = ts->subKey;
      } else {
        pos = startTime;
      }
    }
  }
}
void SplitJoinJP::expireR(size_t cond) {
  size_t pos = 0;
  size_t windowNo = oldestWindowBelong(cond);
  size_t startTime = windowNo * slideLenGlobal;

  if (!TuplePtrQueueLocalR->empty()) {
    TuplePtr tr = *TuplePtrQueueLocalR->front();
    pos = tr->subKey;
    //  cout<<"pos="+ to_string(pos)+", startTime="+ to_string(slideLen)<<endl;
    while (pos < startTime) {
      TuplePtrQueueLocalR->pop();
      if (!TuplePtrQueueLocalR->empty()) {
        tr = *TuplePtrQueueLocalR->front();
        pos = tr->subKey;
      } else {
        pos = startTime;
      }
    }
  }
}
void SplitJoinJP::inlineMain() {
  UtilityFunctions::bind2Core(cpuBind);
  while (1) {
    //stop cmd
    if (TuplePtrQueueInS->empty() && TuplePtrQueueInR->empty()) {
      if (!cmdQueueIn->empty()) {
        join_cmd_t cmdIn = *cmdQueueIn->front();
        cmdQueueIn->pop();
        if (cmdIn == CMD_STOP) {
          sendAck();
          // cout << "join processor " << sysId << " end" << endl;
          return;
        }
      }
    }
    //tuple S
    while (!TuplePtrQueueInS->empty()) {
      TuplePtr ts = *TuplePtrQueueInS->front();
      TuplePtrQueueInS->pop();
      sCnt++;
      if (sCnt == sysId + 1) //should process this S
      {
        joinS(ts);
      }
      if (sCnt == sMax) {
        sCnt = 0;
      }
    }
    while (!TuplePtrQueueInR->empty()) {
      TuplePtr tr = *TuplePtrQueueInR->front();
      TuplePtrQueueInR->pop();
      joinR(tr);
    }
  }
}
