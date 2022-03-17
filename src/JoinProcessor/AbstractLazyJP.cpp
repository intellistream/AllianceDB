//
// Created by tony on 11/03/22.
//

#include <JoinProcessor/AbstractLazyJP.h>
#include <JoinAlgo/NPJ.h>
using namespace INTELLI;
#include <string.h>
void AbstractLazyJP::moveStoBuffer() {

  while (!TuplePtrQueueInS->empty()) {

    TuplePtr tsIn = *TuplePtrQueueInS->front();
    //size_t timeDivTuple=UtilityFunctions::to_periodical(tsIn->subKey,period);
    /*if(sysId==0)
    {
      cout<<to_string(timeDivTuple)+","+to_string(sysId*slide+windowLen)<<endl;
    }*/
    TuplePtrQueueInS->pop();
    windowS.append(tsIn);
    //  cout<<"jp "+ to_string(sysId)+":S not ready"+ to_string(windowS.size())<<endl;
  }
}
void AbstractLazyJP::moveRtoBuffer() {
  //move R to buffer
  while (!TuplePtrQueueInR->empty()) {

    TuplePtr trIn = *TuplePtrQueueInR->front();
    TuplePtrQueueInR->pop();

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

    windowR.append(trIn);
    //  cout<<"jp "+ to_string(sysId)+":R not ready"+to_string(windowR.size())<<endl;
  }
}
void AbstractLazyJP::inlineMain() {
  cout << "join processor" + to_string(sysId) + " (lazyJP) start" << endl;
  status = LWJ_IDLE;
  // NPJSingle algo;
  while (1) { //IDLE
    if (status == LWJ_IDLE) {
      //cout<<"jp "+to_string(sysId)+"IDLE"<<endl;
      if ((!TuplePtrQueueInS->empty()) || (!TuplePtrQueueInR->empty())) {
        status = LWJ_COLLECTING;
      } else if (testCmd(CMD_STOP)) {
        sendAck();
        cout << "join processor" + to_string(sysId) + " (lazyJP) stop" << endl;
        return;
      }
    } else if (status == LWJ_COLLECTING) {
      size_t timeStampSys;
      size_t rLen = TuplePtrQueueInR->size();
      size_t sLen = TuplePtrQueueInS->size();
      // get the max system time
      if (rLen >= 1 && sLen >= 1) {
        //so we can directly use the time stamp of the most recent tuple
        TuplePtr trMax = TuplePtrQueueInR->front()[rLen - 1];
        TuplePtr tsMax = TuplePtrQueueInS->front()[sLen - 1];
        if (trMax->subKey > tsMax->subKey) {
          timeStampSys = trMax->subKey;
        } else {
          timeStampSys = tsMax->subKey;
        }
      } else {
        // no tuple, use system API

        timeStampSys = getTimeStamp();
      }
      //size_t timeStampSys=getTimeStamp();
      size_t timeDivSys = UtilityFunctions::to_periodical(timeStampSys, period);
      //check if this window ends
      if (!isLastJp) {
        if (timeDivSys >= sysId * slide + windowLen) {
          status = LWJ_PROCESSING;
          continue;
        }
      } else {
        if (timeDivSys < windowLen && timeStampSys > period) {
          status = LWJ_PROCESSING;
          continue;
        }
      }
      //move queue to buffer
      moveStoBuffer();
      moveRtoBuffer();

    } else if (status
        == LWJ_PROCESSING) {  //cout<<"jp "+to_string(sysId)+"joins "+to_string(windowS.size())+","+to_string(windowR.size())<<endl;
      size_t sLen = windowS.size();
      size_t rLen = windowR.size();
      if (sLen && rLen) { //add the cost model for algorithm decision in the future
        joinedResult += myAlgo->findAlgo(JOINALGO_NPJ)->join(windowS.data(), windowR.data(), sLen, rLen, 4);
      }
      windowS.reset();
      windowR.reset();
      sWindowReady = false;
      rWindowReady = false;
      status = LWJ_IDLE;
    }

  }

}