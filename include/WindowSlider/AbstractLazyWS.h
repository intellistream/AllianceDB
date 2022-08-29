/*! \file AbstractLazyWS.h*/
//
// Created by tony on 10/03/22.
//

#ifndef _WINDOWSLIDER_ABSTRACTLAZYWS_H_
#define _WINDOWSLIDER_ABSTRACTLAZYWS_H_
#include <Common/Types.hpp>
#include <WindowSlider/AbstractWS.h>
#include <JoinProcessor/AbstractLazyJP.h>
namespace AllianceDB {
/**
 * @defgroup WINDOWSLIDER WindowSliders
 * @{
 */
/**
 * @defgroup WINDOWSLIDER_LAZY lazy window slider
 * @{
 * The lazy sliders that follow window-wide update, i.e., they can process tuples after each window is collected
 * @}
 * @}
 */
/**
 * @ingroup WINDOWSLIDER_LAZY
* @class AbstractLazyWS WindowSlider/AbstractLazyWS.h
* @brief An abstraction of lazy window slider, also inherited by other lazy window slider
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
*/
class AbstractLazyWS : public AbstractWS, public Executor {
 protected:
  vector<AbstractLazyJPPtr> jps;
  size_t period = 0;
  size_t windowCnt = 0;
  virtual void Process();

 public:
  AbstractLazyWS() {
    reset();
    nameTag = "LWJ";
  }

  /**
 * @brief to init the slider with specific length of queue
  * @param sLen the length of S queue
   * @param rLen the length of R queue
 */
  AbstractLazyWS(size_t sLen, size_t rLen) : AbstractWS(sLen, rLen) {
    nameTag = "LWJ";
  }

  //init the join processors
  /**
 * @brief to init the initJoinProcessors
  * @note only after this is called can we start to feed tuples
 */
  virtual void initJoinProcessors();

  /**
 * @brief to terminate the join processors
 */
  virtual void terminateJoinProcessors();

  /**
 * @brief to wait the response of join processors
 */
  virtual void waitAckFromJoinProcessors();

  //get the join result
  /**
* @brief to get the result of join
   *  @result how many tuples are joined
 * @note only called after all join processors are stopped
   * ,use @ref terminateJoinProcessors to achieve this
*/
  virtual size_t getJoinResult();

};
}

#endif //ALIANCEDB_SRC_WINDOWSLIDER_ABSTRACTLAZYWS_H_
