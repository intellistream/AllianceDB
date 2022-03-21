//
// Created by tony on 28/02/22.
//
#include <WindowSlider/HandShakeWS.h>
using namespace INTELLI;
using namespace std;
void HandShakeWS::initJoinProcessors() {
  threads = partitionWeight.size();
  cout << "enable " << threads << " threads" << endl;
  initBar = std::make_shared<std::barrier<>>(threads + 1);
  jpPtr = std::vector<HandShakeJPPtr>(threads);
  //partition at the beginning
  //return;
  vector<size_t> parVec = UtilityFunctions::avgPartitionSizeFinal(windowLen, partitionWeight);
  size_t timeOffsetS = 0;
  size_t timeOffsetR = 0;
  for (size_t tid = 0; tid < threads; tid++) {
    jpPtr[tid] = make_shared<HandShakeJP>();
    jpPtr[tid]->init(sLen, rLen, tid);
    if (isRunTimeScheduling()) {
      jpPtr[tid]->setCore(tid);
    }
    jpPtr[tid]->setInitBar(initBar);
    jpPtr[tid]->setTimeBased(isTimeBased());
    jpPtr[tid]->setGlobalWindow(windowLen,slideLen);
    //jpPtr[tid]->setTimeStart(timeSys);
  }
  for (size_t tid = 0; tid < threads; tid++) {

    if (tid > 0) {
      jpPtr[tid]->setLeft(jpPtr[tid - 1]);
   //   printf("set left\r\n");
    }
    if (tid < threads - 1) {
      jpPtr[tid]->setRight(jpPtr[tid + 1]);
    //  printf("set right\r\n");
    }
    size_t subWindowLen = parVec[tid];
    jpPtr[tid]->setWindowLen(subWindowLen);
    timeOffsetR = windowLen - timeOffsetS - subWindowLen;
    jpPtr[tid]->setTimeOffset(timeOffsetS, timeOffsetR);
    timeOffsetS += subWindowLen;
    cout << "JP " << tid << "window len=" << parVec[tid] << endl;
    //jpPtr[tid]->start();

    jpPtr[tid]->startThread();

  }
  initBar->arrive_and_wait();
  isRunning = true;
}
void HandShakeWS::terminateJoinProcessors() {
  for (size_t tid = 0; tid < threads; tid++) {
    //join_cmd_t cmd=CMD_STOP;
    jpPtr[tid]->inputCmd(CMD_STOP);
  }
  waitAckFromJoinProcessors();
  for (size_t tid = 0; tid < threads; tid++) {
    jpPtr[tid]->joinThread();
  }
  isRunning = false;
}
void HandShakeWS::waitAckFromJoinProcessors() {
  for (size_t tid = 0; tid < threads; tid++) {

    join_cmd_t cmd = jpPtr[tid]->waitResponse();
    if (cmd != CMD_ACK) {
      cout << "wrong ack from " << tid << endl;
    } else {
      // cout<<"Right ack from "<<tid<<endl;
    }
  }
}
size_t HandShakeWS::getJoinResult() {
  size_t ru = 0;
  for (size_t tid = 0; tid < threads; tid++) {

    ru += jpPtr[tid]->getJoinedResult();
    cout << "JP" << tid << " : " << jpPtr[tid]->getJoinedResult() << endl;
  }
  return ru;
}
void HandShakeWS::feedTupleS(TuplePtr ts) {
  /*if (timeBased) //use time stamp, S and R share the same time system
  {
    size_t timeNow = getTimeStamp();
    ts->subKey = timeNow;
  } else { //use simple arrival count, the cnt of S and R are seperated
    ts->subKey = countS;
    countS++;
  }*/
  jpPtr[0]->feedTupleS(ts);
}

void HandShakeWS::feedTupleR(TuplePtr tr) {
  /*if (timeBased) //use time stamp
  {
    size_t timeNow = getTimeStamp();
    tr->subKey = timeNow;
  } else {
    tr->subKey = countR;
    countR++;
  }*/
  jpPtr[threads - 1]->feedTupleR(tr);
}