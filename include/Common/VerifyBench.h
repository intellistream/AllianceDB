
#ifndef _VERIFYBENCH_H_
#define _VERIFYBENCH_H_

#include <Common/Types.h>
#include <WindowSlider/AbstractEagerWS.h>
#include <WindowSlider/VerifyWS.h>
using namespace INTELLI;
namespace INTELLI {
/**
 * @ingroup Common
 * @defgroup INTELLI_COMMON_VERIFY The verification bench
 * @{
 * This is the verification test bench to check whether a window slider can return the right results, compared with the single-thread, simple join slider
 * @ref VerifyWS.
 * @}
 */
 /**
  * @ingroup INTELLI_COMMON_VERIFY
  * @class VerifyBench  Common/VerifyBench.h
  * @tparam wsType The class of window slider you want to test
  */
template<class wsType=AbstractEagerWS>
class VerifyBench {
 public:
  /**
   *
   * @param joinResult  The detailed result you want to collect
   * @param relationCouple The input data of relationCouple
   * @param threads The parallel threads, default THREAD_NUMBER
   * @param windowLen  The length of sliding window, default WINDOW_SIZE
   * @param slideLen The slide of window,  default WINDOW_SIZE
   * @return bool, to indicate whether the verification can pass.
   */
  static bool test(Result &joinResult, RelationCouple &relationCouple,size_t threads=THREAD_NUMBER,size_t windowLen=WINDOW_SIZE,size_t slideLen=WINDOW_SIZE) {
    //wsType windowSlider to be verified
    size_t sLen = relationCouple.relationS.size();
    size_t rLen = relationCouple.relationR.size();
    wsType windowSlider(sLen, rLen);
    joinResult.algoName = windowSlider.getName();
    cout<<"verify algo:"+  joinResult.algoName+", threads="+ to_string(threads)+" windowLen="+ to_string(windowLen)+", slideLen="+ to_string(slideLen) <<endl;
    windowSlider.setParallelSMP(threads);
    // windowSlider.setStopCondition(0, sLen, rLen);
    windowSlider.setTimeBased(true);
    windowSlider.setWindowLen(windowLen);
    windowSlider.setSlideLen(slideLen);
    windowSlider.setRunTimeScheduling(true);
    windowSlider.initJoinProcessors();

    // The verifyws as the reference

    VerifyWS vfSlider(sLen,rLen);
    vfSlider.setParallelSMP(threads);
    vfSlider.setTimeBased(true);
    vfSlider.setWindowLen(windowLen);
    vfSlider.setSlideLen(slideLen);
    vfSlider.setRunTimeScheduling(true);
    struct timeval timeSys = windowSlider.getSysTime();
    vfSlider.setSysTime(timeSys);
    vfSlider.initJoinProcessors();
    //size_t timeBase=windowSlider.getStartTime();
    //cout<<"system start at "<<timeBase<<endl;

    size_t cnt = 0;
    INTELLI::UtilityFunctions::timerStart(joinResult);
    while (!relationCouple.relationR.empty() || !relationCouple.relationS.empty()) {
      size_t timeFeed = UtilityFunctions::timeLastUs(timeSys) / TIME_STEP;
      if (!relationCouple.relationR.empty()) {
        INTELLI::TuplePtr tr = relationCouple.relationR.front();
        if (timeFeed >= tr->subKey) {
          // cout<<"feed r:"<<timeSys<<endl;
          relationCouple.relationR.pop();
          windowSlider.feedTupleR(tr);
          vfSlider.feedTupleR(tr);
        }
      }
      if (!relationCouple.relationS.empty()) {
        INTELLI::TuplePtr ts = relationCouple.relationS.front();
        if (timeFeed >= ts->subKey) {
          relationCouple.relationS.pop();
          windowSlider.feedTupleS(ts);
          vfSlider.feedTupleS(ts);
        }

      }
      // cout<<"process tuple"<<cnt<<endl;
      cnt++;
      usleep(TIME_STEP / 2);
    }
    cout << "end of tuple feeding" << endl;
    joinResult.joinNumber = 0;
    windowSlider.terminateJoinProcessors();
    vfSlider.terminateJoinProcessors();
    joinResult.joinNumber = windowSlider.getJoinResult();
    INTELLI::UtilityFunctions::timerEnd(joinResult);
    if(vfSlider.getJoinResult()==windowSlider.getJoinResult())
    { cout<<"Congratulations, the result "+ to_string(joinResult.joinNumber)+ " is correct!"<<endl;
      return true;
    }
    else
    {
      cout<<"Ops, ot matched, expecting "+ to_string(vfSlider.getJoinResult())+"but return "+to_string(windowSlider.getJoinResult())<<endl;
      return false;
    }
  }
};
}

#endif //HYBRID_JOIN_SRC_JOINMETHODS_ABSTRACTJOINMETHOD_H_
