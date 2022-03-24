//
// Created by tony on 2022/2/9.
//
#include<JoinProcessor/HandShakeJP.h>
using namespace INTELLI;
void HandShakeJP::setupQueue() {
  TuplePtrQueueInS = newTuplePtrQueue(sQueue);
  TuplePtrQueueInR = newTuplePtrQueue(rQueue);
  TuplePtrQueueLocalS = newTuplePtrQueue(sQueue);
  TuplePtrQueueForwardS = newTuplePtrQueue(sQueue);
  TuplePtrQueueLocalR = newTuplePtrQueue(rQueue);
  cmdQueueIn = newCmdQueue(1);
  cmdQueueOut = newCmdQueue(1);
  if (rightJP != nullptr) {
    sRecvAck = newCmdQueue(sQueue);
  } else {
    sRecvAck = newCmdQueue(1);
  }
  sRecvAck = newCmdQueue(sQueue);
  myAlgo = newJoinAlgoTable();

}
void HandShakeJP::inlineMain() {
//first bind to the core
  UtilityFunctions::bind2Core(cpuBind);
  setupQueue();
  waitInitBar();
  //cout << "jp "+ to_string(sysId)+" is ready,S="+ to_string( timeOffsetS)+ ", R= "+ to_string(timeOffsetR)+"windowLen="+ to_string(windowLen)+ "\r\n";

  while (1) {
//cmd
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
    // s receive ack
    while (!sRecvAck->empty()) { //cout<<"parase ack"<<endl;
      sRecvAck->pop();
      if (!TuplePtrQueueForwardS->empty()) {//cout<<"parase ack"+ to_string(sysId)<<endl;
        TuplePtrQueueForwardS->pop();
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
  if (trIn->subKey + timeOffsetR > timeOffsetS) {
    expireS(trIn->subKey + timeOffsetR - timeOffsetS);
  }
  //expireS(trIn->subKey+timeOffsetS-timeOffsetR);
//join the trIn with S window
//build S
  // size_t sSize = TuplePtrQueueLocalS->size();

  /* for (size_t i = 0; i < sSize; i++) {
     TuplePtr ts = TuplePtrQueueLocalS->front()[i];
     if (ts->key == trIn->key) {
       joinedResult++;
     }
   }*/
  joinedResult +=
      myAlgo->findAlgo(JOINALGO_NESTEDLOOP)->join(TuplePtrQueueLocalS, trIn, 2);
  joinedResult +=
      myAlgo->findAlgo(JOINALGO_NESTEDLOOP)->join(TuplePtrQueueForwardS, trIn, 2);

  TuplePtrQueueLocalR->push(trIn);
  expireR(trIn->subKey);
// late expire of R
}
void HandShakeJP::paraseTupleS() {
  TuplePtr tsIn = *TuplePtrQueueInS->front();
  TuplePtrQueueInS->pop();
  if (leftJP != nullptr) { // to indicate the s is received
    leftJP->indicateSRecv();
  }
  if (tsIn->subKey + timeOffsetS > timeOffsetR) {
    expireR(tsIn->subKey + timeOffsetS - timeOffsetR);
  }
  //expireR(tsIn->subKey+timeOffsetR-timeOffsetS);
  //size_t rSize = TuplePtrQueueLocalR->size();

  /*for (size_t i = 0; i < rSize; i++) {
    TuplePtr tr = TuplePtrQueueLocalR->front()[i];
    if (tr->key == tsIn->key) {
      joinedResult++;
    }
  }*/
  joinedResult +=
      myAlgo->findAlgo(JOINALGO_NESTEDLOOP)->join(TuplePtrQueueLocalR, tsIn, 2);
  TuplePtrQueueLocalS->push(tsIn);
  expireS(tsIn->subKey);

}
void HandShakeJP::expireS(size_t cond) {
  size_t pos = 0;
  // size_t windowNo = oldestWindowBelong(cond);
  //size_t startTime = windowNo * slideLenGlobal;
  size_t startTime = (cond < windowLen) ? 0 : cond - windowLen;
  if (!TuplePtrQueueLocalS->empty()) {
    TuplePtr ts = *TuplePtrQueueLocalS->front();
    pos = ts->subKey;
    while (pos < startTime) {
      TuplePtrQueueLocalS->pop();
      if (rightJP != nullptr) {
        //move to forwarded
        TuplePtrQueueForwardS->push(ts);
        rightJP->feedTupleS(ts);
      }
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
  // size_t windowNo = oldestWindowBelong(cond);
  // size_t startTime = windowNo * slideLenGlobal;
  size_t startTime = (cond < windowLen) ? 0 : cond - windowLen;
  if (!TuplePtrQueueLocalR->empty()) {
    TuplePtr tr = *TuplePtrQueueLocalR->front();
    pos = tr->subKey;
    //  cout<<"pos="+ to_string(pos)+", startTime="+ to_string(slideLen)<<endl;
    while (pos < startTime) {
      TuplePtrQueueLocalR->pop();
      if (leftJP != nullptr) {//cout<<"send R to left"<<endl;
        leftJP->feedTupleR(tr);
      }
      if (!TuplePtrQueueLocalR->empty()) {
        tr = *TuplePtrQueueLocalR->front();
        pos = tr->subKey;
      } else {
        pos = startTime;
      }
    }
  }
}
