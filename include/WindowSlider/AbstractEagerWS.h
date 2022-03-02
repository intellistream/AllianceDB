/*! \file AbstractEagerWS.h*/

/*!
    An abstraction of eager window slider

    \return 错误码，0表示成功，其它表示失败
    \todo add the result collection in the future

*/

#ifndef WINDOWSLIDER_ABSTRACTEAGERWS_H_
#define WINDOWSLIDER_ABSTRACTEAGERWS_H_
#include <cstdint>
#include <vector>
#include <Common/Types.h>
#include <Utils/SPSCQueue.hpp>
#include <time.h>
#include <numeric>
#include <JoinProcessor/SimpleHashJP.h>

using namespace INTELLI;
using namespace std;
namespace INTELLI {
//Note: "Upon every arrival of a tuple, the opposing window is re-partitioned to perform a parallel scan"
/**
 *  @defgroup WindowSliders WindowSliders
 *  @{
* @class AbstractEagerWS WindowSlider/AbstractEagerWS.h
* @brief An abstraction of eager window slider, also inherited by other eager window slider
* @author Tony Zeng
* @note
* detailed description:
 To init and run, follow the functions below to start a WS
   \li Configure the window type, time or count, @ref setTimeBased
   \li Configure window length: @ref setWindowLen
   \li Set parallel executing behavior on SMP,@ref setParallelSMP
   \li Optional, (@ref setRunTimeScheduling)
   \li To make the parallel join processors started, @ref initJoinProcessors
   \li Feed tuples @ref feedTupleS or @ref feedTupleR
   \li Terminate, by @ref terminateJoinProcessors
*/
/*! Class that is inherited using public inheritance */
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
  struct timeval timeSys;  /*!< timeval structure from linux, <sys/time.h> */
 public:
  //generate the partition vector of offset
  vector<size_t> weightedPartitionSizeFinal(size_t inS); //reserved for AMP
  vector<size_t> avgPartitionSizeFinal(size_t inS); //for SMP

  /**
   * @brief to configure the window type
   * @param ts wether the slider is time-baesd
   */
  void setTimeBased(bool ts) {
    timeBased = ts;
  }
  /**
   * @brief to read the window type
   * @result wether the slider is time-baesd
   */
  bool isTimeBased() {
    return timeBased;
  }
  /**
   * @brief to configure the scheduling place
   * @param r wether let runtime schedule
   */
  void setRunTimeScheduling(bool r) {
    runTimeScheduling = r;
  }
  /**
   * @brief to read the scheduling type
   * @result wether the runtime schedules
   */
  bool isRunTimeScheduling() {
    return runTimeScheduling;
  }
  /**
  * @brief to set the length of window
  * @param wl the window length
  */
  void setWindowLen(size_t wl) {
    windowLen = wl;
  }
  /**
  * @brief to set the parallel level under SMP model
  * @param threads to how many threads will run the join
  */
  void setParallelSMP(size_t threads) {
    partitionWeight = std::vector<size_t>(threads);
    for (size_t i = 0; i < threads; i++) {
      partitionWeight[i] = 1;
    }
  }
  /**
 * @brief reset everything needed
 */
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
  /**
 * @brief to init the slider with specific length of queue
  * @param sLen the length of S queue
   * @param rLen the length of R queue
 */
  AbstractEagerWS(size_t sLen, size_t rLen);
  //feed the tuple S
  /**
* @brief to feed a tuple s
 * @param ts the tuple s
  * @note this function is thread-safe :)
*/
  virtual void feedTupleS(TuplePtr ts);
  //feed the tuple R
  /**
* @brief to feed a tuple R
 * @param tr the tuple r
  * @note this function is thread-safe :)
*/
  virtual void feedTupleR(TuplePtr tr);
  ~AbstractEagerWS();

  //init the join processors
  /**
 * @brief to init the initJoinProcessors
  * @note only after this is called can we start to feed tuples
 */
  void initJoinProcessors();
  /**
 * @brief to terminate the join processors
 */
  void terminateJoinProcessors();
  void waitAckFromJoinProcessors();
  //get the join result
  /**
* @brief to get the result of join
   *  @result how many tuples are joined
 * @note only called after all join processors are stopped
   * ,use @ref terminateJoinProcessors to achieve this
*/
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
/**@}*/