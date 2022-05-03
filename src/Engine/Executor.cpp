#include <Engine/Executor.hpp>

template<class wsType>
void Executor<wsType>::Run(ResultPtr joinResult,
                           RelationsPtr relations,
                           size_t threads,
                           size_t windowLen,
                           size_t slideLen) {
  wsType windowSlider;
  struct timeval timeSys;
  Initialize(&windowSlider, relations, threads, windowLen, slideLen, joinResult, timeSys);

  size_t cnt = 0;
  INTELLI::UtilityFunctions::timerStart(joinResult);

  while (!relations->relationR.empty() || !relations->relationS.empty()) {
    size_t timeFeed = UtilityFunctions::timeLastUs(timeSys) / TIME_STEP;
    if (!relations->relationR.empty()) {
      INTELLI::TuplePtr tr = relations->relationR.front();
      if (timeFeed >= tr->subKey) {
        // cout<<to_string(timeFeed)+","+ to_string(tr->subKey)<<endl;
        relations->relationR.pop_back();
        windowSlider.feedTupleR(tr);
      }
    }
    if (!relations->relationS.empty()) {
      INTELLI::TuplePtr ts = relations->relationS.front();
      if (timeFeed >= ts->subKey) {
        relations->relationS.pop_back();
        windowSlider.feedTupleS(ts);
      }

    }
    // cout<<"process tuple"<<cnt<<endl;
    cnt++;
    usleep(TIME_STEP / 2);
  }

  cout << "end of tuple feeding" << endl;
  joinResult->joinNumber = 0;
  windowSlider.terminateJoinProcessors();
  joinResult->joinNumber = windowSlider.getJoinResult();

  INTELLI::UtilityFunctions::timerEnd(joinResult);

}

template<class wsType>
void Executor<wsType>::Initialize(wsType & windowSlider, const RelationsPtr &relations,
                                  size_t threads,
                                  size_t windowLen,
                                  size_t slideLen,
                                  ResultPtr &joinResult,
                                  timeval &timeSys) const {
  timeSys = windowSlider.getSysTime();
  size_t sLen = relations->relationS.size();
  size_t rLen = relations->relationR.size();
  windowSlider(sLen, rLen);
  joinResult->algoName = windowSlider.getName();
  cout << "execution algo:" + joinResult->algoName + ", threads=" + to_string(threads) + " windowLen="
      + to_string(windowLen) + ", slideLen=" + to_string(slideLen) << endl;
  windowSlider.setParallelSMP(threads);
  windowSlider.setTimeBased(true);
  windowSlider.setWindowLen(windowLen);
  windowSlider.setSlideLen(slideLen);
  windowSlider.setRunTimeScheduling(true);
  windowSlider.initJoinProcessors();
}
