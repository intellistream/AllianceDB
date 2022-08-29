//
// Created by tony on 18/03/22.
//

#include <WindowSlider/SplitJoinIRWS.h>
SplitJoinIRWS::SplitJoinIRWS(size_t sLen, size_t rLen) : SplitJoinWS(sLen, rLen) {
  nameTag = "SplitJoinIR";
}
void SplitJoinIRWS::initJoinProcessors() {
  threads = partitionWeight.size();
  cout << "enable " << threads << " threads" << endl;
  jps = std::vector<SplitJoinJPPtr>(threads);
  for (size_t tid = 0; tid < threads; tid++) {
    jps[tid] = make_shared<SplitJoinIRJP>();
    jps[tid]->init(sLen, rLen, tid);
    jps[tid]->setGlobalWindow(windowLen, slideLen);
    jps[tid]->setMaxSCnt(threads);
    if (isRunTimeScheduling()) {
      jps[tid]->setCore(tid);
    }
    jps[tid]->Start();
  }
  isRunning = true;
  // this->Start();
}