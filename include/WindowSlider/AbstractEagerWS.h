/*! \file AbstractEagerWS.h*/

/*!
    An abstraction of eager window slider


    \todo add the result collection in the future

*/

#ifndef WINDOWSLIDER_ABSTRACTEAGERWS_H_
#define WINDOWSLIDER_ABSTRACTEAGERWS_H_

#include <WindowSlider/AbstractWS.h>
#include <JoinProcessor/CellJoinJP.h>
#include <Utils/AbstractC20Thread.h>
using namespace INTELLI;
using namespace std;
namespace INTELLI {
//Note: "Upon every arrival of a tuple, the opposing window is re-partitioned to perform a parallel scan"
/**
 *@defgroup WINDOWSLIDER WindowSliders
 * @{
 **/
/**
 * @defgroup WINDOWSLIDER_EAGER eager window slider
 * @{
 * The eager sliders that follow tuple-wide update, i.e., they process each tuples upon it arrives
 * @}
 * @}
 */
/**
 * @ingroup WINDOWSLIDER_EAGER
* @class AbstractEagerWS WindowSlider/AbstractEagerWS.h
* @brief An abstraction of eager window slider (i.e., the CellJoin), also inherited by other eager window slider
* @author Tony Zeng
* @note
* detailed description:
To init and run, follow the functions below to start a WS
  \li Configure the window type, time or count, @ref setTimeBased
  \li Configure window length: @ref setWindowLen
  \li Configure slide length: @ref setSlideLen (default is 1 if not called)
  \li Set parallel executing behavior on SMP,@ref setParallelSMP
  \li Optional, (@ref setRunTimeScheduling)
  \li To make the parallel join processors started, @ref initJoinProcessors
  \li Feed tuples @ref feedTupleS or @ref feedTupleR
  \li Terminate, by @ref terminateJoinProcessors
*
*/
class AbstractEagerWS : public AbstractWS, public AbstractC20Thread {
 private:
  /* data */
  std::vector<CellJoinJPPtr> jps;
 protected:
  void expireS(size_t cond);
  void expireR(size_t cond);
  TuplePtrQueue TuplePtrQueueLocalS;
  TuplePtrQueue TuplePtrQueueLocalR;
  virtual void inlineMain();
  /**
   * @brief deliver tuple s to join processors
   * @param ts The tuple s
   */
  void deliverTupleS(TuplePtr ts);
  /**
* @brief deliver tuple r to join processors
* @param ts The tuple r
*/
  void deliverTupleR(TuplePtr tr);
  /**
   * @brief To get the possible oldest a time stamp belongs to
   * @param ts The time stamp
   * @return The window number, start from 0
   */
  size_t oldestWindowBelong(size_t ts);
 public:
  //generate the partition vector of offset
  vector<size_t> weightedPartitionSizeFinal(size_t inS); //reserved for AMP
  vector<size_t> avgPartitionSizeFinal(size_t inS); //for SMP

  AbstractEagerWS() {
    reset();
    nameTag = "CellJoin";
  }

  //init with length of queue
  /**
 * @brief to init the slider with specific length of queue
  * @param sLen the length of S queue
   * @param rLen the length of R queue
 */
  AbstractEagerWS(size_t sLen, size_t rLen);
  //feed the tuple S

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
};
}
#endif
/**
 * @}
 */
/**@}*/