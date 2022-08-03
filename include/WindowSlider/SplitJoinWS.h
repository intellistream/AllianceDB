/*! \file SplitJoinWS.h*/
//
// Created by tony on 18/03/22.
//

#ifndef _WINDOWSLIDER_SPLITWS_H_
#define _WINDOWSLIDER_SPLITWS_H_

#include <WindowSlider/AbstractWS.h>
#include <Utils/AbstractC20Thread.h>
#include <JoinProcessor/SplitJoinJP.h>
using namespace INTELLI;
using namespace std;
namespace INTELLI {
/**
 * @ingroup WINDOWSLIDER_EAGER
* @class SplitJoinWS WindowSlider/SplitJoinWS.h
* @brief The eager window slider of split join
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
class SplitJoinWS : public AbstractWS {
 protected:
  /* data */
  std::vector<SplitJoinJPPtr> jps;
  // virtual void inlineMain();
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
 public:
  SplitJoinWS() {
    reset();
    nameTag = "SplitJoin";
  }
  /**
* @brief to init the slider with specific length of queue
 * @param sLen the length of S queue
  * @param rLen the length of R queue
*/
  SplitJoinWS(size_t sLen, size_t rLen);

  ~SplitJoinWS() {}
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

  /**
* @brief to feed a tuple s
* @param ts the tuple s
 * @note this function is thread-safe :)
*/
  virtual void feedTupleS(TuplePtr ts) {
    deliverTupleS(ts);
  }
  //feed the tuple R
  /**
* @brief to feed a tuple R
 * @param tr the tuple r
  * @note this function is thread-safe :)
*/
  virtual void feedTupleR(TuplePtr tr) {
    deliverTupleR(tr);
  }
};
}
#endif //ALIANCEDB_INCLUDE_WINDOWSLIDER_SPLITWS_H_
