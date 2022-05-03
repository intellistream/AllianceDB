#include <JoinProcessor/AbstractLazyJP.h>
#include <JoinAlgo/NPJ.h>
using namespace INTELLI;
#include <string.h>
#include <Common/Tuple.hpp>

bool AbstractLazyJP::largerThanMe(size_t timeDivSys) {
  if (timeDivSys >= tsEnd) {
    return true;
  }
  return false;
}
bool AbstractLazyJP::smallerThanMe(size_t timeDivSys) {
  if (timeDivSys < tsBegin) {
    return true;
  }
  return false;
}
bool AbstractLazyJP::checkTupleS(TuplePtr ts) {
  // size_t timeDivSys = UtilityFunctions::to_periodical(ts->subKey, period);
  size_t timeDivSys = ts->subKey;
  if (smallerThanMe(timeDivSys)) {
    statusS = LWJ_IDLE;
    return false;
  }
  if (largerThanMe(timeDivSys)) {
    statusS = LWJ_PROCESSING;
    return false;
  }
  // Therefore, only tBegin<=ts<tEnd will be collected
  statusS = LWJ_COLLECTING;
  // There is no overlapping for the starting window
  if (sysId == 0 && timeDivSys < period) {
    windowS.append(ts);
    return true;
  }
  if (timeDivSys >= tsOverlap) {
    // non-overlapping area
    windowS.append(ts);
  } else {
    // over lapping area
    windowSOverLap.append(ts);
  }
  return true;
}
bool AbstractLazyJP::checkTupleR(TuplePtr tr) {
  //size_t timeDivSys = UtilityFunctions::to_periodical(tr->subKey, period);
  size_t timeDivSys = tr->subKey;
  if (smallerThanMe(timeDivSys)) {
    statusR = LWJ_IDLE;
    return false;
  }
  if (largerThanMe(timeDivSys)) {
    statusR = LWJ_PROCESSING;
    return false;
  }
  // Therefore, only tBegin<=tr<tEnd will be collected
  statusR = LWJ_COLLECTING;
  // There is no overlapping for the starting window
  if (sysId == 0 && timeDivSys < period) {
    windowR.append(tr);
    return true;
  }
  if (timeDivSys >= tsOverlap) {
    // non-overlapping area
    windowR.append(tr);
  } else {
    // over lapping area
    windowROverLap.append(tr);
  }
  //windowR.append(tr);
  return true;
}
void AbstractLazyJP::moveStoBuffer() {

  while (!TuplePtrQueueInS->empty()) {
    TuplePtr tsIn = *TuplePtrQueueInS->front();
    TuplePtrQueueInS->pop();
    checkTupleS(tsIn);

  }
}

void AbstractLazyJP::moveRtoBuffer() {
  //move R to buffer
  while (!TuplePtrQueueInR->empty()) {

    TuplePtr trIn = *TuplePtrQueueInR->front();
    TuplePtrQueueInR->pop();
    checkTupleR(trIn);

  }
}
void AbstractLazyJP::inlineMain() {
  //cout << "join processor" + to_string(sysId) + " (lazyJP) start" << endl;

  cout << "jp " + to_string(sysId) + "start at" + to_string(tsBegin) + ", overlap until" + to_string(tsOverlap)
      + ", end at" + to_string(tsEnd) + ",period=" + to_string(period) << endl;

  // NPJSingle algo;
  while (1) { //IDLE
    //should stop

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
    moveStoBuffer();
    moveRtoBuffer();

    // we can process
    if (statusS
        == LWJ_PROCESSING && statusR
        == LWJ_PROCESSING) {  //cout<<"jp "+to_string(sysId)+"joins "+to_string(windowS.size())+","+to_string(windowR.size())<<endl;
      size_t sLen = windowS.size();
      size_t rLen = windowR.size();
      size_t sLenOver = windowSOverLap.size();
      size_t rLenOver = windowROverLap.size();
      /*if(!sLen&&!rLen)
       {
         cout<<"zero length"<<endl;
         //continue;
       }*/
      //non over lapping part

      //s_over with r_non
      if (sLenOver && rLen) {
        joinedResult +=
            myAlgo->findAlgo(JOINALGO_NPJ)->join(windowSOverLap.data(), windowR.data(), sLenOver, rLen, 4);
      }
      //s_non with r_over
      if (sLen && rLenOver) {
        joinedResult +=
            myAlgo->findAlgo(JOINALGO_NPJ)->join(windowS.data(), windowROverLap.data(), sLen, rLenOver, 4);
      }

      /*if (sLenOver && rLenOver) {
         joinedResult += myAlgo->findAlgo(JOINALGO_NPJ_SINGLE)->join(windowSOverLap.data(),
                                                                     windowROverLap.data(),
                                                                     sLenOver,
                                                                     rLenOver,
                                                                     4);
       }*/
      if (sLen && rLen) { //add the cost model for algorithm decision in the future
        joinedResult += myAlgo->findAlgo(JOINALGO_NPJ)->join(windowS.data(), windowR.data(), sLen, rLen, 4);
        //child->queueWindow(windowSOverLap,windowS,windowSOverLap,windowR);

      }
      windowS.reset();
      windowR.reset();
      windowSOverLap.reset();
      windowROverLap.reset();
      tsBegin += period;
      tsEnd += period;
      tsOverlap += period;
      statusS = LWJ_IDLE;
      statusR = LWJ_IDLE;
    }

  }

}