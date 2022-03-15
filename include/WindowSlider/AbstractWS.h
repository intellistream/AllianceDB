/*! \file AbstractWS.h*/
#ifndef _WINDOWSLIDER_ABSTRACTWS_H_
#define _WINDOWSLIDER_ABSTRACTWS_H_
#include <cstdint>
#include <vector>
#include <Common/Types.h>
#include <Utils/SPSCQueue.hpp>
#include <Utils/UtilityFunctions.hpp>
#include <time.h>
#include <numeric>
using namespace INTELLI;
using namespace std;
namespace INTELLI {
/**
 *@defgroup WINDOWSLIDER WindowSliders
 * @{
 **/
 /**
  * @defgroup WINDOWSLIDER_BASE Common base for lazy and eager
  * @{
  * @}
  * @}
  */
/**
 * @ingroup WINDOWSLIDER_BASE
* @class AbstractWS WindowSlider/AbstractWS.h
* @brief An abstraction of  window slider, also inherited by both eager and lazy
* @author Tony Zeng
* @note
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
class AbstractWS {

 protected:

  INTELLI::join_type_t myType = INTELLI::CNT_BASED;
  size_t countS, countR;
  size_t windowLen = 0;
  size_t slideLen=1;
  //partition
  std::vector<size_t> partitionWeight;
  std::vector<size_t> partitionSizeFinal;
  size_t threads;
  size_t sLen, rLen;
  bool runTimeScheduling = false;
  bool timeBased = false;
  bool isRunning = false;
  struct timeval timeSys;  /*!< timeval structure from linux, <sys/time.h> */
  TuplePtrQueue TuplePtrQueueInS;
  TuplePtrQueue TuplePtrQueueInR;
 public:

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
  * @brief set the length of slide
  * @param sli The assigned length
  */
  void setSlideLen(size_t sli)
  {
    slideLen=sli;
  }
  /**
   * @brief get the length of slide
   * @result the length of slide
   */
  size_t getSlideLen()
  {
    return slideLen;
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

    gettimeofday(&timeSys, NULL);
  }

  AbstractWS() {
    reset();
  }
  /**
   * @brief to get the time stamp
   * @return the time stamp, default in 10 us
   */
  size_t getTimeStamp() {

    return UtilityFunctions::timeLastUs(timeSys) / TIME_STEP;
  }
  //init with length of queue
  /**
 * @brief to init the slider with specific length of queue
  * @param sLen the length of S queue
   * @param rLen the length of R queue
 */
  AbstractWS(size_t sLen, size_t rLen);
  //feed the tuple S
  /**
* @brief to feed a tuple s
 * @param ts the tuple s
  * @note this function is thread-safe :)
*/
  virtual void feedTupleS(TuplePtr ts)
  {
    //assert(ts);
    TuplePtrQueueInS->push(ts);
  }
  //feed the tuple R
  /**
* @brief to feed a tuple R
 * @param tr the tuple r
  * @note this function is thread-safe :)
*/
  virtual void feedTupleR(TuplePtr tr)
  {
    //assert(tr);
    TuplePtrQueueInR->push(tr);
  }
  ~AbstractWS();

  //init the join processors
  /**
 * @brief to init the initJoinProcessors
  * @note only after this is called can we start to feed tuples
 */
  virtual void initJoinProcessors()
  {

  }
  /**
 * @brief to terminate the join processors
 */
  virtual void terminateJoinProcessors()
  {

  }
  /**
 * @brief to wait the response of join processors
 */
  virtual void waitAckFromJoinProcessors()
  {

  }
  //get the join result
  /**
* @brief to get the result of join
   *  @result how many tuples are joined
 * @note only called after all join processors are stopped
   * ,use @ref terminateJoinProcessors to achieve this
*/
  virtual size_t getJoinResult()
  {
    return 0;
  }
  /**
   * @brief to get the inline timee structure, the timeSys member
   * @return the timeSys member
   */
  struct timeval getSysTime() {
    return timeSys;
  }
};
}
#endif
/**
 * @}
 */
/**
 * @}
 */
/**@}*/