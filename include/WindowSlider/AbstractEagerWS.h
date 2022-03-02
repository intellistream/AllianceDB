/*! \file AbstractEagerWS.h*/

/*!
    An abstraction of eager window slider
    \li To init and run, follow the functions below to start a WS
   * setTimeBased: configure window type
   * setWindowLen: configure window length
   * setParallelSMP/setParallelAMP:set parallel executing behavior on SMP/AMP
   * (setRunTimeScheduling)
   * initJoinProcessors: to make the parallel join processors started
   * use feedTupleS/feedTupleR to feed data
   * (terminateJoinProcessors)
    \return 错误码，0表示成功，其它表示失败
    \todo add the result collection in the future

*/

#ifndef WINDOWSLIDER_ABSTRACTEAGERWS_H_
#define WINDOWSLIDER_ABSTRACTEAGERWS_H_
#include <cstdint>
#include <vector>
#include <JoinMethods/Types.h>
#include <Utils/SPSCQueue.hpp>
#include <time.h>
#include <numeric>
#include <JoinProcessor/SimpleHashJP.h>

using namespace INTELLI;
using namespace std;
namespace INTELLI {
//Note: "Upon every arrival of a tuple, the opposing window is re-partitioned to perform a parallel scan"
class AbstractEagerWS {
 private:
  /* data */
  std::vector<SimpleHashJPPtr> jps;
 protected:
  TupleQueuePtr tupleQueueS;
  TupleQueuePtr tupleQueueR;
  INTELLI::join_type_t myType = INTELLI::CNT_BASED;
  size_t countS, countR, timeStart;

  size_t timeMax, sMax, rMax;
  size_t windowLen = 0;
  void updateWindowS();
  void updateWindowR();
  //partition
  std::vector<size_t> partitionWeight;
  std::vector<size_t> partitionSizeFinal;

  size_t threads;
  size_t sLen, rLen;
  bool runTimeScheduling = false;
  bool timeBased = false;
  bool isRunning = false;
  //window expire
  void expireS(size_t cond);
  void expireR(size_t cond);
  struct timeval timeSys;
 public:
  //generate the partition vector of offset
  vector<size_t> weightedPartitionSizeFinal(size_t inS); //reserved for AMP
  vector<size_t> avgPartitionSizeFinal(size_t inS); //for SMP
  /* note: please follow the functions below to start a WS
   * setTimeBased: configure window type
   * setWindowLen: configure window length
   * setParallelSMP/setParallelAMP:set parallel executing behavior on SMP/AMP
   * (setRunTimeScheduling)
   * initJoinProcessors: to make the parallel join processors started
   * use feedTupleS/feedTupleR to feed data
   * (terminateJoinProcessors)
   */
  // configure the window type
  void setTimeBased(bool ts) {
    timeBased = ts;
  }
  //read the window type
  bool isTimeBased() {
    return timeBased;
  }
  //runTime scheduling
  void setRunTimeScheduling(bool r) {
    runTimeScheduling = r;
  }
  bool isRunTimeScheduling() {
    return runTimeScheduling;
  }
  // set the length of window
  void setWindowLen(size_t wl) {
    windowLen = wl;
  }
  //SMP partition
  void setParallelSMP(size_t threads) {
    partitionWeight = std::vector<size_t>(threads);
    for (size_t i = 0; i < threads; i++) {
      partitionWeight[i] = 1;
    }
  }
  void reset() {
    countS = 0;
    countR = 0;
    timeStart = clock();
    gettimeofday(&timeSys,NULL);
  }

  //if _timeMax>0, the slider will use time stamp, otherwise, it just counts the s and r
  void setStopCondition(size_t _timeMax, size_t _sMax, size_t _rMax) {
    timeMax = _timeMax;
    sMax = _sMax;
    rMax = _rMax;
  }
  AbstractEagerWS() {
    reset();
  }
  size_t getTimeStamp() {

    return UtilityFunctions::timeLastUs(timeSys)/TIME_STEP;
  }
  //init with length of queue
  AbstractEagerWS(size_t sLen, size_t rLen);
  //feed the tuple S
  virtual void feedTupleS(TuplePtr ts);
  //feed the tuple R
  virtual void feedTupleR(TuplePtr tr);
  ~AbstractEagerWS();

  //init the join processors
  void initJoinProcessors();
  void terminateJoinProcessors();
  void waitAckFromJoinProcessors();
  //get the join result
  size_t getJoinResult();
  //startTime
  size_t getStartTime()
  {
    return timeStart;
  }
  struct timeval getSysTime()
  {
    return timeSys;
  }
};
}
#endif