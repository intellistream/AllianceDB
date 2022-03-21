//
// Created by tony on 2022/2/9.
//
#include<JoinProcessor/HandShakeJP.h>
using namespace INTELLI;
void HandShakeJP::setupQueue() {
  TuplePtrQueueInS = newTuplePtrQueue(sQueue);
  TuplePtrQueueInR = newTuplePtrQueue(rQueue);
  TuplePtrQueueLocalS= newTuplePtrQueue(sQueue);
  TuplePtrQueueLocalR = newTuplePtrQueue(rQueue);
  cmdQueueIn = newCmdQueue(1);
  cmdQueueOut = newCmdQueue(1);

}
void HandShakeJP::inlineMain() {
//first bind to the core
  UtilityFunctions::bind2Core(cpuBind);
  setupQueue();
  waitInitBar();
  cout << "jp "+ to_string(sysId)+" is ready,S="+ to_string( timeOffsetS)+ ", R= "+ to_string(timeOffsetR)+ "\r\n";

  while (1) {
//cmd
    if (TuplePtrQueueInS->empty() && TuplePtrQueueInR->empty()) {
      if (!cmdQueueIn->empty()) {

        join_cmd_t cmdIn = *cmdQueueIn->front();
        cmdQueueIn->pop();
        if (cmdIn == CMD_STOP) {
          sendAck();
          cout << "join processor " << sysId << " end" << endl;
          return;
        }
      }
    }
    while (!TuplePtrQueueInR->empty()) { //cout<<"r is full"<<endl;
      paraseTupleR();
    }
    while (!TuplePtrQueueInS->empty()) {//cout<<" s is full"<<endl;
      paraseTupleS();
    }
  }
}
void HandShakeJP::paraseTupleR() {
  TuplePtr trIn = *TuplePtrQueueInR->front();
  TuplePtrQueueInR->pop();
  size_t tNow;
  if (isTimeBased()) {
    tNow = getTimeStamp();
//cout<<tNow<<endl;
//trIn->subKey=tNow;
    expireS(tNow);

  }
//join the trIn with S window
//build S
  size_t sSize = TuplePtrQueueLocalS->size();
/*hashtable hashtableS;
for (size_t i = 0; i < sSize; i++) {
  TuplePtr ts = TuplePtrQueueLocalS->front()[i];
  hashtableS.emplace(ts->key, 0, ts->subKey);
}
//probe R
auto findMatchS = hashtableS.find(trIn->key);
if (findMatchS != hashtableS.end()) {
  size_t matches = findMatchS->second.size();
  joinedResult += matches;
 // printf("JP%ld:R[%ld](%ld) find %ld S matches\r\n", sysId, trIn->subKey + 1, trIn->key, matches);
}*/
  for (size_t i = 0; i < sSize; i++) {
    TuplePtr ts = TuplePtrQueueLocalS->front()[i];
    if (ts->key == trIn->key) {
      joinedResult++;
    }
  }

  TuplePtrQueueLocalR->push(trIn);
  expireR(tNow);
// late expire of R
  if (!isTimeBased()) {

/*if (TuplePtrQueueLocalR->size()>windowLen)
{   TuplePtr tr = *TuplePtrQueueLocalR->front();
  if(leftJP!= nullptr)
  {
    leftJP->feedTupleR(tr);
  }
  TuplePtrQueueLocalR->pop();
}*/
    expireR(countR);
  }
}
void HandShakeJP::paraseTupleS() {
  TuplePtr tsIn = *TuplePtrQueueInS->front();
  TuplePtrQueueInS->pop();
  size_t tNow;
  if (isTimeBased()) {
    tNow = getTimeStamp();
//tsIn->subKey=tNow;

    expireR(tNow);
  }
//join the tsIn with R window
//build R
  size_t rSize = TuplePtrQueueLocalR->size();
/*hashtable hashtableR;
for (size_t i = 0; i < rSize; i++) {
  TuplePtr tr= TuplePtrQueueLocalR->front()[i];
  hashtableR.emplace(tr->key, 0, tr->subKey);
}
//probe S
auto findMatchR = hashtableR.find(tsIn->key);
if (findMatchR != hashtableR.end()) {
  size_t matches = findMatchR->second.size();
  joinedResult += matches;
 // printf("JP%ld:R[%ld](%ld) find %ld S matches\r\n", sysId, tsIn->subKey + 1, tsIn->key, matches);
}*/
  for (size_t i = 0; i < rSize; i++) {
    TuplePtr tr = TuplePtrQueueLocalR->front()[i];
    if (tr->key == tsIn->key) {
      joinedResult++;
    }
  }
  TuplePtrQueueLocalS->push(tsIn);
  expireS(tNow);
// late expire of S
  if (!isTimeBased()) {

/* if (TuplePtrQueueLocalS->size()>windowLen)
 {   TuplePtr ts = *TuplePtrQueueLocalS->front();
   if(rightJP!= nullptr)
   {
     rightJP->feedTupleS(ts);
   }
   TuplePtrQueueLocalS->pop();
 }*/
    expireS(countS);
  }

}
void HandShakeJP::expireS(size_t cond) {
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

void HandShakeJP::expireR(size_t cond) {
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
