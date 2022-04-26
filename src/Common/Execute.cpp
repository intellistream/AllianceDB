#include <Common/Execute.hpp>
template<class wsType>
void Execute<wsType>::Run(Result &joinResult,
                          RelationCouple &relationCouple,
                          size_t threads,
                          size_t windowLen,
                          size_t slideLen) {
  size_t sLen = relationCouple.relationS.size();
  size_t rLen = relationCouple.relationR.size();
  wsType windowSlider(sLen, rLen);
  struct timeval timeSys = windowSlider.getSysTime();

  joinResult.algoName = windowSlider.getName();
  cout << "execution algo:" + joinResult.algoName + ", threads=" + to_string(threads) + " windowLen="
      + to_string(windowLen) + ", slideLen=" + to_string(slideLen) << endl;
  windowSlider.setParallelSMP(threads);
  // windowSlider.setStopCondition(0, sLen, rLen);
  windowSlider.setTimeBased(true);
  windowSlider.setWindowLen(windowLen);
  windowSlider.setSlideLen(slideLen);
  windowSlider.setRunTimeScheduling(true);
  windowSlider.initJoinProcessors();

  size_t cnt = 0;
  INTELLI::UtilityFunctions::timerStart(joinResult);
  while (!relationCouple.relationR.empty() || !relationCouple.relationS.empty()) {
    size_t timeFeed = UtilityFunctions::timeLastUs(timeSys) / TIME_STEP;
    if (!relationCouple.relationR.empty()) {
      INTELLI::TuplePtr tr = relationCouple.relationR.front();
      if (timeFeed >= tr->subKey) {
        // cout<<to_string(timeFeed)+","+ to_string(tr->subKey)<<endl;
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