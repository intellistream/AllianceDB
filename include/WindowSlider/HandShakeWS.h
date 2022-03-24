/*! \file HandShakeWS.h*/
//
// Created by tony on 28/02/22.
//

#ifndef _WINDOWSLIDER_HANDSHAKEWS_H_
#define _WINDOWSLIDER_HANDSHAKEWS_H_
#include <cstdint>
#include <vector>
#include <Common/Types.h>
#include <Utils/SPSCQueue.hpp>
#include <time.h>
#include <numeric>
#include <JoinProcessor/HandShakeJP.h>
#include <WindowSlider/AbstractEagerWS.h>

using namespace INTELLI;
using namespace std;
namespace INTELLI {
/**
 * @ingroup WINDOWSLIDER_EAGER
* @class  HandShakeWS WindowSlider/HandShakeWS.h
* @brief The eager window slider of handshake join
* @note
* detailed description:
To init and run, follow the functions below to start a WS
  \li Configure the window type, time or count, @ref setTimeBased
  \li Configure window length: @ref setWindowLen
  \li Configure slide length: @ref setSlideLen (Only support 0 for hand shake join, means fixed slide, i.e., every tuple)
  \li Set parallel executing behavior on SMP,@ref setParallelSMP
  \li Optional, (@ref setRunTimeScheduling)
  \li To make the parallel join processors started, @ref initJoinProcessors
  \li Feed tuples @ref feedTupleS or @ref feedTupleR
  \li Terminate, by @ref terminateJoinProcessors
*
*/
class HandShakeWS : public AbstractWS {
 private:
  /* data *
   */
 protected:
  std::vector<HandShakeJPPtr> jpPtr;
  INTELLI::BarrierPtr initBar;
 public:
  HandShakeWS(/* args */) {
    reset();
  }
  ~HandShakeWS() {

  }
  //init with length of queue
  HandShakeWS(size_t sLen, size_t rLen) : AbstractWS(sLen, rLen) {

  }

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
  //input
  //note: S->, R<-
  /**
* @brief to feed a tuple s
* @param ts the tuple s
 * @note this function is thread-safe :)
*/
  void feedTupleS(TuplePtr ts);
  /**
* @brief to feed a tuple R
 * @param tr the tuple r
  * @note this function is thread-safe :)
*/
  void feedTupleR(TuplePtr tr);
};

}
#endif //HYBRID_JOIN_INCLUDE_WINDOWSLIDER_HANDSHAKEWS_H_
