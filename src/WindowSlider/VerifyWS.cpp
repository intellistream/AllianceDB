//
// Created by tony on 18/03/22.
//

#include <WindowSlider/VerifyWS.h>
using namespace INTELLI;
VerifyWS::VerifyWS(size_t sLen, size_t rLen) : AbstractWS(sLen, rLen) {
  TuplePtrQueueLocalS = newTuplePtrQueue(sLen);
  TuplePtrQueueLocalR = newTuplePtrQueue(rLen);

  reset();
  nameTag = "VerifyJoin";
  //555
}

/*
size_t AbstractEagerWS::oldestWindowBelong(size_t ts) {
  if (ts < windowLen) {
    return 0;
  }
  return ((ts - windowLen) / slideLen) + 1;
}*/
void VerifyWS::deliverTupleS(TuplePtr ts) {
  if (timeBased) //use time stamp, S and R share the same time system
  {
    size_t timeNow = ts->subKey;
    expireR(timeNow);
  } else {
    ts->subKey = countS;
    expireR(countS);
    countS++;
  }
  TuplePtrQueueLocalS->push(ts);
  //single thread join window r and tuple s
  size_t rSize = TuplePtrQueueLocalR->size();
  for (size_t i = 0; i < rSize; i++) {
    if (TuplePtrQueueLocalR->front()[i]->key == ts->key) {
      // cout<<to_string(ts->subKey)+","+to_string(TuplePtrQueueLocalR->front()[i]->subKey)<<endl;
      //cout<<"S "+to_string(ts->subKey)+"join R "+ to_string(TuplePtrQueueLocalR->front()[i]->subKey)<<endl;
      joinResults++;
    }
  }
  //usleep(1000);
  // joinResults=myAlgo->findAlgo(JOINALGO_NPJ_SINGLE)->join(TuplePtrQueueLocalR->front(),ts,TuplePtrQueueLocalR->size(),1);
  //waitAckFromJoinProcessors();
}
void VerifyWS::deliverTupleR(TuplePtr tr) {
  if (timeBased) //use time stamp
  {
    size_t timeNow = tr->subKey;
    expireS(timeNow);
  } else {
    tr->subKey = countR;
    expireS(countR);
    countR++;
  }
  TuplePtrQueueLocalR->push(tr);
  size_t sSize = TuplePtrQueueLocalS->size();
  for (size_t i = 0; i < sSize; i++) {
    if (TuplePtrQueueLocalS->front()[i]->key == tr->key) {
      //cout<<to_string(TuplePtrQueueLocalS->front()[i]->subKey)+","+to_string(tr->subKey)<<endl;
      // cout<<"S "+to_string(tr->subKey)+"join S "+ to_string(TuplePtrQueueLocalS->front()[i]->subKey)<<endl;
      joinResults++;
    }
  }
  //single thread join window s and tuple r

  //joinResults=myAlgo->findAlgo(JOINALGO_NPJ_SINGLE)->join(TuplePtrQueueLocalS->front(),tr,TuplePtrQueueLocalS->size(),1);

  //joinResults=nlj.
}

void VerifyWS::expireR(size_t ts) {
  size_t pos = 0;
  size_t startTime;

  if (slideLen == 0) {
    startTime = (ts < windowLen) ? 0 : (ts - windowLen);
  } else {
    size_t windowNo = oldestWindowBelong(ts);
    startTime = windowNo * slideLen;
  }

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
  /*windowR.reset();
  if(TuplePtrQueueLocalR->size()>0)
  {
    size_t allLen=TuplePtrQueueLocalR->size();
    //size_t validLen=0;
    for(size_t i=0;i<allLen;i++)
    {  TuplePtr tp = TuplePtrQueueLocalR->front()[i];
      if(tp->subKey<=ts)
      {
        windowR.append(tp);
      }
    }
    //windowS.append(TuplePtrQueueLocalS->front(),validLen);
  }*/
}

void VerifyWS::expireS(size_t ts) {
  size_t pos = 0;
  size_t startTime;

  if (slideLen == 0) {
    startTime = (ts < windowLen) ? 0 : (ts - windowLen);
  } else {
    size_t windowNo = oldestWindowBelong(ts);
    startTime = windowNo * slideLen;
  }
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
  /*windowS.reset();
  if(TuplePtrQueueLocalS->size()>0)
  {
    size_t allLen=TuplePtrQueueLocalS->size();
    //size_t validLen=0;
    for(size_t i=0;i<allLen;i++)
    {  TuplePtr tp = TuplePtrQueueLocalS->front()[i];
      if(tp->subKey<=ts)
      {
        windowS.append(tp);
      }
    }
    //windowS.append(TuplePtrQueueLocalS->front(),validLen);
  }*/
  /*
   for(size_t i=allLen;i>0;i--)
   {
      if(TuplePtrQueueLocalS->front()[i]->subKey<=ts)
      {
        validLen=i+1;
        break;
      }
   }*/
  // windowS.append(TuplePtrQueueLocalS->front(),validLen);

}
void VerifyWS::initJoinProcessors() {
  threads = partitionWeight.size();
  cout << "enable " << threads << " threads" << endl;
  cout << "warning, this is just a single thread WS" << endl;
  isRunning = true;
  this->startThread();
}
void VerifyWS::terminateJoinProcessors() {

  isRunning = false;
  this->joinThread();
}
void VerifyWS::waitAckFromJoinProcessors() {
}

size_t VerifyWS::getJoinResult() {
  return joinResults;
}
void VerifyWS::inlineMain() {
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
}
