//
// Created by tony on 18/03/22.
//

#include <WindowSlider/SplitJoinWS.h>
SplitJoinWS::SplitJoinWS(size_t sLen, size_t rLen) : AbstractWS(sLen, rLen) {
  reset();
  nameTag = "SplitJoin";
}
void SplitJoinWS::initJoinProcessors() {
  threads = partitionWeight.size();
  cout << "enable " << threads << " threads" << endl;
  jps = std::vector<SplitJoinJPPtr>(threads);
  for (size_t tid = 0; tid < threads; tid++) {
    jps[tid] = make_shared<SplitJoinJP>();
    jps[tid]->init(sLen, rLen, tid);
    jps[tid]->setGlobalWindow(windowLen, slideLen);
    jps[tid]->setMaxSCnt(threads);
    if (isRunTimeScheduling()) {
      jps[tid]->setCore(tid);
    }
    jps[tid]->startThread();
  }
  isRunning = true;
 // this->startThread();
}
void SplitJoinWS::terminateJoinProcessors() {
  for (size_t tid = 0; tid < threads; tid++) {
    //join_cmd_t cmd=CMD_STOP;
    jps[tid]->inputCmd(CMD_STOP);
  }
  waitAckFromJoinProcessors();
  for (size_t tid = 0; tid < threads; tid++) {
    jps[tid]->joinThread();
  }
  isRunning = false;
 // this->joinThread();
}
void SplitJoinWS::waitAckFromJoinProcessors() {
  for (size_t tid = 0; tid < threads; tid++) {

    join_cmd_t cmd = jps[tid]->waitResponse();
    if (cmd != CMD_ACK) {
      cout << "wrong ack from " << tid << endl;
    } else {
      // cout<<"Right ack from "<<tid<<endl;
    }
  }
}
/*
void SplitJoinWS::inlineMain() {
  while (isRunning) {
    while (!TuplePtrQueueInS->empty()) {
      TuplePtr ts = *TuplePtrQueueInS->front();
      TuplePtrQueueInS->pop();
      deliverTupleS(ts);
    }
    while (!TuplePtrQueueInR->empty()) {
      TuplePtr tr = *TuplePtrQueueInR->front();
      TuplePtrQueueInR->pop();
      deliverTupleR(tr);
    }
  }
}*/
void SplitJoinWS::deliverTupleS(TuplePtr ts) {
  //just send to all
  for (size_t tid = 0; tid < threads; tid++) {
    jps[tid]->feedTupleS(ts);
  }
}

void SplitJoinWS::deliverTupleR(TuplePtr tr) {
  //just send to all
  for (size_t tid = 0; tid < threads; tid++) {
    jps[tid]->feedTupleR(tr);
  }
}

size_t SplitJoinWS::getJoinResult() {
  size_t ru = 0;
  for (size_t tid = 0; tid < threads; tid++) {
    ru += jps[tid]->getJoinedResult();
    //cout << "JP" << tid << " : " << jps[tid]->getJoinedResult() << endl;
  }
  return ru;
}