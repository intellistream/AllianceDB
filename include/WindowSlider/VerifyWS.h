/*! \file VerifyWS.h*/

//
// Created by tony on 18/03/22.
//

#ifndef _WINDOWSLIDER_VERIFYWS_H_
#define _WINDOWSLIDER_VERIFYWS_H_
#include <WindowSlider/AbstractWS.h>
#include <JoinProcessor/CellJoinJP.h>
#include <Utils/AbstractC20Thread.h>
namespace INTELLI {
/**
 * @ingroup WINDOWSLIDER_BASE
* @class VerifyWS WindowSlider/VerifyWS.h
* @brief The single-thread window slider used for verify results of other WS
* @author Tony Zeng
* @note
To init and run, follow the functions below to start a WS
  \li Configure the window type, time or count, @ref setTimeBased
  \li Configure window length: @ref setWindowLen
  \li Configure slide length: @ref setSlideLen (default is 1 if not called)
  \li Set parallel executing behavior on SMP,@ref setParallelSMP
  \li Optional, (@ref setRunTimeScheduling)
  \li To make the parallel join processors started, @ref initJoinProcessors
  \li Feed tuples @ref feedTupleS or @ref feedTupleR
  \li Terminate, by @ref terminateJoinProcessors
*/
class VerifyWS:public AbstractWS, public AbstractC20Thread{
 protected:
  TuplePtrQueue TuplePtrQueueLocalS;
  TuplePtrQueue TuplePtrQueueLocalR;
  size_t joinResults;
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
  void expireS(size_t cond);
  void expireR(size_t cond);
 public:
/**
 * @brief reset everything needed
 */
  void reset()
  {
    AbstractWS::reset();
    joinResults=0;
  }
  VerifyWS()
  {
    reset();
    nameTag = "VerifyJoin";
  }
  ~VerifyWS()
  {

  }
  /**
 * @brief to init the slider with specific length of queue
  * @param sLen the length of S queue
   * @param rLen the length of R queue
 */
  VerifyWS(size_t sLen, size_t rLen);
  /**
* @brief to init the initJoinProcessors
 * @note Just a format function for this slider
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
};
}
#endif //ALIANCEDB_INCLUDE_WINDOWSLIDER_VERIFYWS_H_
