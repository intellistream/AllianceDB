
#ifndef JOINMETHODS_ABSTRACTJOINMETHOD_H_
#define JOINMETHODS_ABSTRACTJOINMETHOD_H_

#include <Common/Types.hpp>
#include <Common/Result.hpp>
#include <Common/Tuple.hpp>
#include <Common/RelationCouple.hpp>
#include <WindowSlider/AbstractEagerWS.h>
using namespace INTELLI;
namespace INTELLI {
template<class wsType=AbstractEagerWS>
class AbstractJoinMethod {
 public:
  static void test(Result &joinResult, RelationCouple &relationCouple) {
    //wsType windowSlider;
    size_t sLen = relationCouple.relationS.size();
    size_t rLen = relationCouple.relationR.size();
    wsType windowSlider(sLen, rLen);
    joinResult.algoName = windowSlider.getName();
    windowSlider.setParallelSMP(THREAD_NUMBER);
    // windowSlider.setStopCondition(0, sLen, rLen);
    windowSlider.setTimeBased(true);
    windowSlider.setWindowLen(WINDOW_SIZE);
    windowSlider.setSlideLen(WINDOW_SIZE);
    windowSlider.setRunTimeScheduling(true);
    windowSlider.initJoinProcessors();
    //size_t timeBase=windowSlider.getStartTime();
    //cout<<"system start at "<<timeBase<<endl;
    struct timeval timeSys = windowSlider.getSysTime();
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
        }
      }
      if (!relationCouple.relationS.empty()) {
        INTELLI::TuplePtr ts = relationCouple.relationS.front();
        if (timeFeed >= ts->subKey) {
          relationCouple.relationS.pop();
          windowSlider.feedTupleS(ts);
        }

      }
      // cout<<"process tuple"<<cnt<<endl;
      cnt++;
      usleep(TIME_STEP / 2);
    }
    cout << "end of tuple feeding" << endl;
    joinResult.joinNumber = 0;
    windowSlider.terminateJoinProcessors();
    joinResult.joinNumber = windowSlider.getJoinResult();
    INTELLI::UtilityFunctions::timerEnd(joinResult);
  }
};
}

#endif //HYBRID_JOIN_SRC_JOINMETHODS_ABSTRACTJOINMETHOD_H_
