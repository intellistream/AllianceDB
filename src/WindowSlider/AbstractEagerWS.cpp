//
// Created by tony on 2022/2/8.
//
#include <WindowSlider/AbstractEagerWS.h>
using namespace INTELLI;
AbstractEagerWS::AbstractEagerWS(size_t _sLen, size_t _rLen): AbstractWS(_sLen,_rLen) {

  TuplePtrQueueLocalS = newTuplePtrQueue(sLen);
  TuplePtrQueueLocalR = newTuplePtrQueue(rLen);
  //reset();
  nameTag="CellJoin";
}
AbstractEagerWS::~AbstractEagerWS() {
  if (isRunning) {
    terminateJoinProcessors();
  }
}
size_t AbstractEagerWS::oldestWindowBelong(size_t ts) {
  if(ts<windowLen)
  {
    return 0;
  }
  return ((ts-windowLen)/slideLen)+1;
}
void AbstractEagerWS::deliverTupleS(TuplePtr ts) {
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

  partitionSizeFinal = avgPartitionSizeFinal( TuplePtrQueueLocalR->size());
  size_t rBase = 0;
  for (size_t tid = 0; tid < threads; tid++) {
    // partition window R
    size_t wrLen = partitionSizeFinal[tid];
    WindowOfTuples wr(wrLen);
    for (size_t i = 0; i < wrLen; i++) {
      wr[i] = ( TuplePtrQueueLocalR->front()[rBase + i]);
    }
    rBase += wrLen;
    //feed window r
    jps[tid]->feedWindowR(wr);
    //feed tuple S
    jps[tid]->feedTupleS(ts);
  }
  //waitAckFromJoinProcessors();
}
void AbstractEagerWS::deliverTupleR(TuplePtr tr)  {
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
  partitionSizeFinal = avgPartitionSizeFinal( TuplePtrQueueLocalS->size());
  size_t sBase = 0;
  for (size_t tid = 0; tid < threads; tid++) {
    //partition window S
    size_t wsLen = partitionSizeFinal[tid];
    WindowOfTuples ws(wsLen);
    for (size_t i = 0; i < wsLen; i++) {
      ws[i] = ( TuplePtrQueueLocalS->front()[sBase + i]);
    }
    sBase += wsLen;
    //feed window S
    jps[tid]->feedWindowS(ws);
    //feed tuple R
    jps[tid]->feedTupleR(tr);
  }
}
vector<size_t> AbstractEagerWS::weightedPartitionSizeFinal(size_t inS) {

  return INTELLI::UtilityFunctions::weightedPartitionSizeFinal(inS, partitionWeight);;
}

vector<size_t> AbstractEagerWS::avgPartitionSizeFinal(size_t inS) {

  /*size_t partitions = partitionWeight.size();
  vector<size_t> partitionSizeFinals = vector<size_t>(partitions);
  size_t divideLen = inS / partitions;
  size_t tEnd = 0;

  for (size_t i = 0; i < partitions - 1; i++) {
    tEnd += divideLen;
    partitionSizeFinals[i] = divideLen;
  }
  partitionSizeFinals[partitions - 1] = inS - tEnd;
  return partitionSizeFinals;*/
  return INTELLI::UtilityFunctions::avgPartitionSizeFinal(inS, partitionWeight);
}
void AbstractEagerWS::expireR(size_t ts) {
  size_t pos = 0;
  size_t windowNo= oldestWindowBelong(ts);
  size_t startTime=windowNo*slideLen;

  if (! TuplePtrQueueLocalR->empty()) {
    TuplePtr tr = * TuplePtrQueueLocalR->front();
    pos = tr->subKey;
  //  cout<<"pos="+ to_string(pos)+", startTime="+ to_string(slideLen)<<endl;
    while (pos <startTime) {
      TuplePtrQueueLocalR->pop();
      if (! TuplePtrQueueLocalR->empty()) {
        tr = * TuplePtrQueueLocalR->front();
        pos = tr->subKey;
      } else {
        pos = startTime;
      }
    }
  }

}

void AbstractEagerWS::expireS(size_t ts) {
  size_t pos = 0;
  size_t windowNo= oldestWindowBelong(ts);
  size_t startTime=windowNo*slideLen;
  if (! TuplePtrQueueLocalS->empty()) {
    TuplePtr ts = * TuplePtrQueueLocalS->front();
    pos = ts->subKey;
    while (pos < startTime) {
      TuplePtrQueueLocalS->pop();
      if (! TuplePtrQueueLocalS->empty()) {
        ts = * TuplePtrQueueLocalS->front();
        pos =ts->subKey;
      } else {
        pos = startTime;
      }
    }
  }

}
void AbstractEagerWS::initJoinProcessors() {
  threads = partitionWeight.size();
  cout << "enable " << threads << " threads" << endl;
  jps = std::vector<CellJoinJPPtr>(threads);
  for (size_t tid = 0; tid < threads; tid++) {
    jps[tid] = make_shared<CellJoinJP>();
    jps[tid]->init(sLen, rLen, tid);
    if (isRunTimeScheduling()) {
      jps[tid]->setCore(tid);
    }

    jps[tid]->startThread();
  }
  isRunning = true;
  this->startThread();
}
void AbstractEagerWS::terminateJoinProcessors() {
  for (size_t tid = 0; tid < threads; tid++) {
    //join_cmd_t cmd=CMD_STOP;
    jps[tid]->inputCmd(CMD_STOP);
  }
  waitAckFromJoinProcessors();
  for (size_t tid = 0; tid < threads; tid++) {
    jps[tid]->joinThread();
  }
  isRunning = false;
  this->joinThread();
}
void AbstractEagerWS::waitAckFromJoinProcessors() {
  for (size_t tid = 0; tid < threads; tid++) {

    join_cmd_t cmd = jps[tid]->waitResponse();
    if (cmd != CMD_ACK) {
      cout << "wrong ack from " << tid << endl;
    } else {
      // cout<<"Right ack from "<<tid<<endl;
    }
  }
}

size_t AbstractEagerWS::getJoinResult() {
  size_t ru = 0;
  for (size_t tid = 0; tid < threads; tid++) {
    ru += jps[tid]->getJoinedResult();
    cout << "JP" << tid << " : " << jps[tid]->getJoinedResult() << endl;
  }
  return ru;
}
void AbstractEagerWS::inlineMain() {
  while (isRunning)
  {
    while (!TuplePtrQueueInS->empty())
    {
      TuplePtr ts = *TuplePtrQueueInS->front();
      TuplePtrQueueInS->pop();
      deliverTupleS(ts);
    }
    while (!TuplePtrQueueInR->empty())
    {
      TuplePtr tr = *TuplePtrQueueInR->front();
      TuplePtrQueueInR->pop();
      deliverTupleR(tr);
    }
  }
}
