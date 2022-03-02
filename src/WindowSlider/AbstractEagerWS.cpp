//
// Created by tony on 2022/2/8.
//
#include <WindowSlider/AbstractEagerWS.h>
using namespace INTELLI;
AbstractEagerWS::AbstractEagerWS(size_t _sLen, size_t _rLen) {
  sLen = _sLen;
  rLen = _rLen;
  tupleQueueS = newTupleQueue(sLen);
  tupleQueueR = newTupleQueue(rLen);
  reset();
}
AbstractEagerWS::~AbstractEagerWS() {
  if (isRunning) {
    terminateJoinProcessors();
  }
}
void AbstractEagerWS::feedTupleS(TuplePtr ts) {
  if (timeBased) //use time stamp, S and R share the same time system
  {
    size_t timeNow = getTimeStamp();
  //  ts->subKey = timeNow;
    //expireS(timeNow);
    expireR(timeNow);
  } else { //use simple arrival count, the cnt of S and R are seperated
    ts->subKey = countS;
    expireR(countS);
    countS++;
  }
  tupleQueueS->push(ts);
  // too few tuples in R
  /*size_t rGet = tupleQueueR->size();

  if (rGet < threads) {
    WindowOfTuples  wr(rGet);
    // return;
    for (size_t i = 0; i < rGet; i++) {
     wr[i]=(tupleQueueR->front()[i]);
    }
    //feed window r
    jps[0].feedWindowR(wr);
    //feed tuple s
    jps[0].feedTupleS(ts);
    return;
  }*/
  //do the tuple S, window R join
  partitionSizeFinal = avgPartitionSizeFinal(tupleQueueR->size());
  size_t rBase = 0;
  for (size_t tid = 0; tid < threads; tid++) {
    // partition window R
    size_t wrLen = partitionSizeFinal[tid];
    WindowOfTuples wr(wrLen);
    for (size_t i = 0; i < wrLen; i++) {
      wr[i] = (tupleQueueR->front()[rBase + i]);
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
    size_t timeNow = getTimeStamp();
   // tr->subKey = timeNow;
    expireS(timeNow);
    //expireR(timeNow);
  } else {
    tr->subKey = countR;
    expireS(countR);
    countR++;
  }
  tupleQueueR->push(tr);

  // too few tuples in S
  /*size_t sGet = tupleQueueS->size();
  if (sGet < threads) {
    WindowOfTuples ws(sGet);
    // return;
    for (size_t i = 0; i < sGet; i++) {
      ws[i] = (tupleQueueS->front()[i]);
    }
    //feed window S
    jps[0].feedWindowS(ws);
    //feed tuple R
    jps[0].feedTupleR(tr);
    return;
  }*/
  //do the tuple R, window s join
  partitionSizeFinal = avgPartitionSizeFinal(tupleQueueS->size());
  size_t sBase = 0;
  for (size_t tid = 0; tid < threads; tid++) {
    //partition window S
    size_t wsLen = partitionSizeFinal[tid];
    WindowOfTuples ws(wsLen);
    for (size_t i = 0; i < wsLen; i++) {
      ws[i] = (tupleQueueS->front()[sBase + i]);
    }
    sBase += wsLen;
    //feed window S
    jps[tid]->feedWindowS(ws);
    //feed tuple R
    jps[tid]->feedTupleR(tr);
  }
}
vector<size_t> AbstractEagerWS::weightedPartitionSizeFinal(size_t inS) {

  return INTELLI::UtilityFunctions::weightedPartitionSizeFinal(inS,partitionWeight);;
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
  return  INTELLI::UtilityFunctions::avgPartitionSizeFinal(inS,partitionWeight);
}
void AbstractEagerWS::expireR(size_t cond) {
  size_t distance = 0;
  if (!tupleQueueR->empty()) {
    TuplePtr tr = *tupleQueueR->front();
    distance = cond - tr->subKey;
    while (distance > windowLen) {
      tupleQueueR->pop();
      if (!tupleQueueR->empty()) {
        tr = *tupleQueueR->front();
        distance = cond - tr->subKey;
      } else {
        distance = 0;
      }
    }
  }

}

void AbstractEagerWS::expireS(size_t cond) {
  size_t distance = 0;
  if (!tupleQueueS->empty()) {
    TuplePtr ts = *tupleQueueS->front();
    distance = cond - ts->subKey;
    while (distance > windowLen) {
      tupleQueueS->pop();
      if (!tupleQueueS->empty()) {
        ts = *tupleQueueS->front();
        distance = cond - ts->subKey;
      } else {
        distance = 0;
      }
    }
  }

}
void AbstractEagerWS::initJoinProcessors() {
  threads = partitionWeight.size();
  cout << "enable " << threads << " threads" << endl;
  jps = std::vector<SimpleHashJPPtr>(threads);
  for (size_t tid = 0; tid < threads; tid++) {
    jps[tid]= make_shared<SimpleHashJP>();
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