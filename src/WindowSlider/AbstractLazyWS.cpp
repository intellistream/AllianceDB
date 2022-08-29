//
// Created by tony on 10/03/22.
//

#include <WindowSlider/AbstractLazyWS.h>
using namespace AllianceDB;
void AbstractLazyWS::initJoinProcessors() {
  windowCnt = windowLen / slideLen + 1;
  //period = (windowCnt - 1) * (slideLen) + slideLen;
  period = windowCnt * slideLen;
  // cout<<"period="+ to_string(period)<<endl;
  jps = std::vector<AbstractLazyJPPtr>(windowCnt);
  for (size_t tid = 0; tid < windowCnt; tid++) {
    jps[tid] = make_shared<AbstractLazyJP>();
    jps[tid]->init(sLen, rLen, tid);
    jps[tid]->setTimeVal(timeSys);
    jps[tid]->setLazyWindow(slideLen, windowLen, period);
    jps[tid]->Start();
  }
  isRunning = true;
  this->Start();
}

void AbstractLazyWS::terminateJoinProcessors() {
  for (size_t tid = 0; tid < windowCnt; tid++) {
    //join_cmd_t cmd=CMD_STOP;
    jps[tid]->inputCmd(CMD_STOP);
    //jps[tid]->Join();
  }
  waitAckFromJoinProcessors();
  for (size_t tid = 0; tid < windowCnt; tid++) {
    jps[tid]->Join();
  }
  isRunning = false;
  this->Join();
}
void AbstractLazyWS::waitAckFromJoinProcessors() {
  for (size_t tid = 0; tid < windowCnt; tid++) {

    join_cmd_t cmd = jps[tid]->waitResponse();
    if (cmd != CMD_ACK) {
      cout << "wrong ack from " << tid << endl;
    } else {
      // cout<<"Right ack from "<<tid<<endl;
    }
  }
}

size_t AbstractLazyWS::getJoinResult() {
  size_t ru = 0;
  for (size_t tid = 0; tid < windowCnt; tid++) {

    ru += jps[tid]->getJoinedResult();
    //cout << "JP" << tid << " : " << jps[tid]->getJoinedResult() << endl;
  }
  return ru;
}
/*
void AbstractLazyWS::feedTupleS(TuplePtr ts) {
  //assert(ts);
  size_t timeDivTuple=UtilityFunctions::to_periodical(ts->subKey,period);
  for (size_t tid = 0; tid < windowCnt; tid++) {
    size_t windowBase=slideLen*tid;
    if(timeDivTuple>=windowBase&&timeDivTuple<=windowBase+windowLen)
    {
      jps[tid]->feedTupleS(ts);
    }
  }
  //cout<<to_string(ts->subKey)+","+ to_string(getTimeStamp())<<endl;
}
void AbstractLazyWS::feedTupleR(TuplePtr tr) {
 size_t timeDivTuple=UtilityFunctions::to_periodical(tr->subKey,period);
  for (size_t tid = 0; tid < windowCnt; tid++) {
    size_t windowBase=slideLen*tid;
    if(timeDivTuple>=windowBase&&timeDivTuple<=windowBase+windowLen)
    {
      jps[tid]->feedTupleR(tr);
    }
  }
  //assert(tr);
 // cout<<to_string(tr->subKey)+","+ to_string(getTimeStamp())<<endl;
}*/

void AbstractLazyWS::Process() {
  while (isRunning) {
    while (!TuplePtrQueueInS->empty()) {
      TuplePtr ts = *TuplePtrQueueInS->front();
      TuplePtrQueueInS->pop();
      // size_t timeDivTuple = UtilityFunctions::to_periodical(ts->subKey, period);
      for (size_t tid = 0; tid < windowCnt; tid++) {

        jps[tid]->feedTupleS(ts);
      }
    }
    while (!TuplePtrQueueInR->empty()) {
      TuplePtr tr = *TuplePtrQueueInR->front();
      TuplePtrQueueInR->pop();
      for (size_t tid = 0; tid < windowCnt; tid++) {
        jps[tid]->feedTupleR(tr);
      }
    }
  }
}