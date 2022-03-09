
#ifndef JOINMETHODS_ABSTRACTJOINMETHOD_H_
#define JOINMETHODS_ABSTRACTJOINMETHOD_H_

#include "Common/Types.h"
#include "WindowSlider/AbstractEagerWS.h"
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
    windowSlider.setParallelSMP(THREAD_NUMBER);
    // windowSlider.setStopCondition(0, sLen, rLen);
    windowSlider.setTimeBased(true);
    windowSlider.setWindowLen(WINDOW_SIZE);
    windowSlider.setRunTimeScheduling(true);
    windowSlider.initJoinProcessors();
    //size_t timeBase=windowSlider.getStartTime();
    //cout<<"system start at "<<timeBase<<endl;
    struct timeval timeSys = windowSlider.getSysTime();
    size_t cnt = 0;
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
    }
    cout << "end of tuple feeding" << endl;
    joinResult.joinNumber = 0;
    windowSlider.terminateJoinProcessors();
    joinResult.joinNumber = windowSlider.getJoinResult();
  }
};
}

#endif //HYBRID_JOIN_SRC_JOINMETHODS_ABSTRACTJOINMETHOD_H_
