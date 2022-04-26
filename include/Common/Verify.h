#ifndef _VERIFYBENCH_H_
#define _VERIFYBENCH_H_

#include <Common/Types.hpp>
#include <WindowSlider/AbstractEagerWS.h>
#include <WindowSlider/VerifyWS.h>
using namespace INTELLI;
namespace INTELLI {

/**
 * @ingroup INTELLI_COMMON_VERIFY
 * @class Verify  Common/Verify.h
 * @tparam wsType The class of window slider you want to test
 */
class Verify {
 public:
  /**
   *
   * @param joinResult  The detailed result you want to collect
   * @param relationCouple The input data of relationCouple
   * @param threads The parallel threads, default THREAD_NUMBER
   * @param windowLen  The length of sliding window, default WINDOW_SIZE (unit = TIME_STEP us)
   * @param slideLen The slide of window,  default WINDOW_SIZE (unit = TIME_STEP us)
   * @return bool, to indicate whether the verification can pass.
   */
  static void Run(Result &joinResult,
                   RelationCouple &relationCouple,
                   size_t windowLen = WINDOW_SIZE,
                   size_t slideLen = WINDOW_SIZE) {
    size_t sLen = relationCouple.relationS.size();
    size_t rLen = relationCouple.relationR.size();
    VerifyWS verify_ws(sLen, rLen);
    struct timeval timeSys = verify_ws.getSysTime();
    verify_ws.setParallelSMP(1);
    verify_ws.setTimeBased(true);
    verify_ws.setWindowLen(windowLen);
    verify_ws.setSlideLen(slideLen);
    verify_ws.setRunTimeScheduling(true);
    verify_ws.setSysTime(timeSys);
    verify_ws.initJoinProcessors();
    joinResult.algoName = verify_ws.getName();
    cout << "verify algo:" + joinResult.algoName + ", threads=" + to_string(1) + " windowLen="
        + to_string(windowLen) + ", slideLen=" + to_string(slideLen) << endl;
    size_t cnt = 0;
    INTELLI::UtilityFunctions::timerStart(joinResult);
    while (!relationCouple.relationR.empty() || !relationCouple.relationS.empty()) {
      size_t timeFeed = UtilityFunctions::timeLastUs(timeSys) / TIME_STEP;
      if (!relationCouple.relationR.empty()) {
        INTELLI::TuplePtr tr = relationCouple.relationR.front();
        //
        if (timeFeed >= tr->subKey) {
          // cout<<to_string(timeFeed)+","+ to_string(tr->subKey)<<endl;
          relationCouple.relationR.pop();
          verify_ws.feedTupleR(tr);
        }
      }
      if (!relationCouple.relationS.empty()) {
        INTELLI::TuplePtr ts = relationCouple.relationS.front();
        if (timeFeed >= ts->subKey) {
          relationCouple.relationS.pop();
          verify_ws.feedTupleS(ts);
        }
      }
      cnt++;
      usleep(TIME_STEP / 2);
    }
    cout << "end of tuple feeding" << endl;
    joinResult.joinNumber = 0;
    verify_ws.terminateJoinProcessors();
    joinResult.joinNumber = verify_ws.getJoinResult();
    INTELLI::UtilityFunctions::timerEnd(joinResult);
  }
};
}

#endif //HYBRID_JOIN_SRC_JOINMETHODS_ABSTRACTJOINMETHOD_H_
