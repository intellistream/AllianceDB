//
// Created by tony on 11/03/22.
//

#include <JoinProcessor/AbstractLazyJP.h>
#include <JoinAlgo/NPJ.h>
using namespace INTELLI;
#include <string.h>
bool AbstractLazyJP::belongs2Me(size_t timeDivSys) {
  if (timeDivSys >= tsBegin && timeDivSys < tsEnd) {

    return true;
  } else {

    return false;
  }
}
bool AbstractLazyJP::checkTupleS(TuplePtr ts) {
  size_t timeDivSys = UtilityFunctions::to_periodical(ts->subKey, period);
  if (!belongs2Me(timeDivSys)) {
    statusS = LWJ_PROCESSING;
    return false;
  }
  statusS = LWJ_COLLECTING;
  if (timeDivSys >= tsOverlap) {
    windowS.append(ts);
  } else {
    windowSOverLap.append(ts);
  }
  //windowS.append(ts);
  return true;
}
bool AbstractLazyJP::checkTupleR(TuplePtr tr) {
  size_t timeDivSys = UtilityFunctions::to_periodical(tr->subKey, period);
  if (!belongs2Me(timeDivSys)) {
    statusR = LWJ_PROCESSING;
    return false;
  }
  // isOverlappingRPrev=isOverlappingR;
  statusR = LWJ_COLLECTING;
  if (timeDivSys >= tsOverlap) {
    windowR.append(tr);
  } else {
    windowROverLap.append(tr);
  }
  //windowR.append(tr);
  return true;
}
void AbstractLazyJP::moveStoBuffer() {

  while (!TuplePtrQueueInS->empty()) {

    TuplePtr tsIn = *TuplePtrQueueInS->front();
    //size_t timeDivTuple=UtilityFunctions::to_periodical(tsIn->subKey,period);
    /*if(sysId==0)
    {
      cout<<to_string(timeDivTuple)+","+to_string(sysId*slide+windowLen)<<endl;
    }*/

    TuplePtrQueueInS->pop();
    //to check the overlapping
    checkTupleS(tsIn);
    //windowS.append(tsIn);
    //  cout<<"jp "+ to_string(sysId)+":S not ready"+ to_string(windowS.size())<<endl;
  }
}
void AbstractLazyJP::moveRtoBuffer() {
  //move R to buffer
  while (!TuplePtrQueueInR->empty()) {

    TuplePtr trIn = *TuplePtrQueueInR->front();
    TuplePtrQueueInR->pop();
    checkTupleR(trIn);
    // size_t timeDivTuple=UtilityFunctions::to_periodical(trIn->subKey,period);
    /* if(sysId==1)
     {
       cout<<to_string(timeDivTuple)+","+to_string(sysId*slide+windowLen)<<endl;
     }*/
    /* if(trIn== nullptr)
     {
       rWindowReady= true;
       return;
     }*/

    //windowR.append(trIn);
    //  cout<<"jp "+ to_string(sysId)+":R not ready"+to_string(windowR.size())<<endl;
  }
}
void AbstractLazyJP::inlineMain() {
  cout << "join processor" + to_string(sysId) + " (lazyJP) start" << endl;


  // tsEnd=sysId * slideLenGlobal+windowLen;

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
        == LWJ_PROCESSING || statusR
        == LWJ_PROCESSING) {  //cout<<"jp "+to_string(sysId)+"joins "+to_string(windowS.size())+","+to_string(windowR.size())<<endl;
      size_t sLen = windowS.size();
      size_t rLen = windowR.size();
      size_t sLenOver = windowSOverLap.size();
      size_t rLenOver = windowROverLap.size();
      //non over lapping part

      //s_over with r_non
      if (sLenOver && rLen) {
        joinedResult +=
            myAlgo->findAlgo(JOINALGO_NPJ_SINGLE)->join(windowSOverLap.data(), windowR.data(), sLenOver, rLen, 4);
      }
      //s_non with r_over
      if (sLen && rLenOver) {
        joinedResult +=
            myAlgo->findAlgo(JOINALGO_NPJ_SINGLE)->join(windowS.data(), windowROverLap.data(), sLen, rLenOver, 4);
      }

      if (sLenOver && rLenOver) {
        joinedResult += myAlgo->findAlgo(JOINALGO_NPJ_SINGLE)->join(windowSOverLap.data(),
                                                                    windowROverLap.data(),
                                                                    sLenOver,
                                                                    rLenOver,
                                                                    4);
      }
      if (sLen && rLen) { //add the cost model for algorithm decision in the future
        joinedResult += myAlgo->findAlgo(JOINALGO_NPJ_SINGLE)->join(windowS.data(), windowR.data(), sLen, rLen, 4);
        windowS.reset();
        windowR.reset();
        windowSOverLap.reset();
        windowROverLap.reset();
      }

    }

  }

}