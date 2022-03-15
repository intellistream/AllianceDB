//
// Created by tony on 2022/2/8.
//
#include <WindowSlider/AbstractEagerWS.h>
using namespace INTELLI;
AbstractEagerWS::AbstractEagerWS(size_t _sLen, size_t _rLen) {
  sLen = _sLen;
  rLen = _rLen;
  TuplePtrQueueInS = newTuplePtrQueue(sLen);
  TuplePtrQueueInR = newTuplePtrQueue(rLen);
  reset();
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
void AbstractEagerWS::feedTupleS(TuplePtr ts) {
  if (timeBased) //use time stamp, S and R share the same time system
  {
    size_t timeNow = ts->subKey;
    //  ts->subKey = timeNow;
    //expireS(timeNow);
    expireR(timeNow);
  } else { //use simple arrival count, the cnt of S and R are seperated
    ts->subKey = countS;
    expireR(countS);
    countS++;
  }
  TuplePtrQueueInS->push(ts);
  // too few tuples in R
  /*size_t rGet = TuplePtrQueueInR->size();

  if (rGet < threads) {
    WindowOfTuples  wr(rGet);
    // return;
    for (size_t i = 0; i < rGet; i++) {
     wr[i]=(TuplePtrQueueInR->front()[i]);
    }
    //feed window r
    jps[0].feedWindowR(wr);
    //feed tuple s
    jps[0].feedTupleS(ts);
    return;
  }*/
  //do the tuple S, window R join
  partitionSizeFinal = avgPartitionSizeFinal(TuplePtrQueueInR->size());
  size_t rBase = 0;
  for (size_t tid = 0; tid < threads; tid++) {
    // partition window R
    size_t wrLen = partitionSizeFinal[tid];
    WindowOfTuples wr(wrLen);
    for (size_t i = 0; i < wrLen; i++) {
      wr[i] = (TuplePtrQueueInR->front()[rBase + i]);
    }
    rBase += wrLen;
    //feed window r
    jps[tid]->feedWindowR(wr);
    //feed tuple S
    jps[tid]->feedTupleS(ts);
  }
  //waitAckFromJoinProcessors();
}
void AbstractEagerWS::feedTupleR(TuplePtr tr) {
  if (timeBased) //use time stamp
  {
    size_t timeNow = tr->subKey;
    // tr->subKey = timeNow;
    expireS(timeNow);
    //expireR(timeNow);
  } else {
    tr->subKey = countR;
    expireS(countR);
    countR++;
  }
  TuplePtrQueueInR->push(tr);

  // too few tuples in S
  /*size_t sGet = TuplePtrQueueInS->size();
  if (sGet < threads) {
    WindowOfTuples ws(sGet);
    // return;
    for (size_t i = 0; i < sGet; i++) {
      ws[i] = (TuplePtrQueueInS->front()[i]);
    }
    //feed window S
    jps[0].feedWindowS(ws);
    //feed tuple R
    jps[0].feedTupleR(tr);
    return;
  }*/
  //do the tuple R, window s join
  partitionSizeFinal = avgPartitionSizeFinal(TuplePtrQueueInS->size());
  size_t sBase = 0;
  for (size_t tid = 0; tid < threads; tid++) {
    //partition window S
    size_t wsLen = partitionSizeFinal[tid];
    WindowOfTuples ws(wsLen);
    for (size_t i = 0; i < wsLen; i++) {
      ws[i] = (TuplePtrQueueInS->front()[sBase + i]);
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

  if (!TuplePtrQueueInR->empty()) {
    TuplePtr tr = *TuplePtrQueueInR->front();
    pos = tr->subKey;
  //  cout<<"pos="+ to_string(pos)+", startTime="+ to_string(slideLen)<<endl;
    while (pos <startTime) {
      TuplePtrQueueInR->pop();
      if (!TuplePtrQueueInR->empty()) {
        tr = *TuplePtrQueueInR->front();
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
  if (!TuplePtrQueueInS->empty()) {
    TuplePtr ts = *TuplePtrQueueInS->front();
    pos = ts->subKey;
    while (pos < startTime) {
      TuplePtrQueueInS->pop();
      if (!TuplePtrQueueInS->empty()) {
        ts = *TuplePtrQueueInS->front();
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
  jps = std::vector<SimpleHashJPPtr>(threads);
  for (size_t tid = 0; tid < threads; tid++) {
    jps[tid] = make_shared<SimpleHashJP>();
    jps[tid]->init(sLen, rLen, tid);
    if (isRunTimeScheduling()) {
      jps[tid]->setCore(tid);
    }

    jps[tid]->start();
  }
  isRunning = true;
}
void AbstractEagerWS::terminateJoinProcessors() {
  for (size_t tid = 0; tid < threads; tid++) {
    //join_cmd_t cmd=CMD_STOP;
    jps[tid]->inputCmd(CMD_STOP);
  }
  waitAckFromJoinProcessors();
  for (size_t tid = 0; tid < threads; tid++) {
    jps[tid]->join();
  }
  isRunning = false;
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
