/*! \file AbstractLazyJP.h*/

//
// Created by tony on 11/03/22.
//

#ifndef _JOINPROCESSOR_ABSTRACTLAZYJP_H_
#define _JOINPROCESSOR_ABSTRACTLAZYJP_H_
#include <JoinProcessor/AbstractJP.h>
#include <JoinAlgo/JoinAlgoTable.h>
#include <JoinAlgo/NPJ.h>

namespace INTELLI {
class AbstractLazyJP;
/**
 * @ingroup JOINPROCESSOR JoinProcessors
 * @{
 */
/**
 * @defgroup JOINPROCESSOR_LAZY lazy join processor
 * @{
 * The lazy processors is controlled by lazy window sliders
 * @see WINDOWSLIDER_LAZY
 * @}
 * @}
 */
/**
 * @ingroup JOINPROCESSOR_LAZY
 * @enum lwj_status_t JoinProcessor/AbstractLazyJP.h
 * @brief The status of lazy join processor
 */
typedef enum {
  /**
   * IDLE, no collecting and no processing
   */
  LWJ_IDLE = 0,
  /**
   * COLLECTING tuples, meaning this window is not prepared
   */
  LWJ_COLLECTING,
  /**
   * PROCESSING tuples
   */
  LWJ_PROCESSING,
} lwj_status_t;

/**
 * @ingroup JOINPROCESSOR_LAZY
  * @class AbstractLazyJP JoinProcessor/AbstractLazyJP.h
  * @brief The basic class of lazy join processor
  */
typedef std::shared_ptr<AbstractLazyJP> AbstractLazyJPPtr;
#define  newAbstractLazyJP() make_shared<AbstractLazyJP>()
class AbstractLazyJP : public AbstractJP {
 protected:
  /**
   * @brief non-overlapping area of window S
   */
  C20Buffer<TuplePtr> windowS;
  /**
   * @brief non-overlapping area of window R
   */
  C20Buffer<TuplePtr> windowR;
  /**
   * @brief Overlapping area of window S, with 'previous' window
   */
  C20Buffer<TuplePtr> windowSOverLap;
  /**
  * @brief Overlapping area of window R, with 'previous' window
  */
  C20Buffer<TuplePtr> windowROverLap;
  lwj_status_t statusS = LWJ_IDLE;
  lwj_status_t statusR = LWJ_IDLE;
  // size_t slide = 0;
  //size_t windowLen = 0;

  size_t period = 0;
  size_t tsOverlap = 0;
  size_t tsEnd = 0;
  size_t tsBegin = 0;
  /*bool sWindowReady = false;
  bool rWindowReady = false;
  bool isLastJp = false;*/
  bool checkTupleS(TuplePtr ts);
  bool checkTupleR(TuplePtr tr);
  virtual void inlineMain();
  // bool belongs2Me(size_t timeDivSys);
  bool largerThanMe(size_t timeDivSys);
  bool smallerThanMe(size_t timeDivSys);
  /**
  * @brief move the S from the input queue to the window buffer
   * @param
  */
  void moveStoBuffer();
  /**
   * @brief move the R from the input queue to the window buffer
   */
  void moveRtoBuffer();
 public:
  AbstractLazyJP() {}
  ~ AbstractLazyJP() {}
  /**
    * @brief init the join processor with buffer/queue length and id
    * @param sLen The length of S queue and buffer
    * @param rLen The length of S queue and buffer
    * @param _sysId The system id
    */
  virtual void init(size_t sLen, size_t rLen, size_t _sysId) {
    AbstractJP::init(sLen, rLen, _sysId);

    windowS = C20Buffer<TuplePtr>(sLen);
    windowR = C20Buffer<TuplePtr>(rLen);
    windowSOverLap = C20Buffer<TuplePtr>(sLen);
    windowROverLap = C20Buffer<TuplePtr>(rLen);
  }
  /**
   * @brief set the parameters of lazy window
   * @param sli The slide
   * @param wlen The window length
   * @param per The period in system
   */
  void setLazyWindow(size_t sli, size_t wlen, size_t per) {
    // slide = sli;
    windowLen = wlen;
    setGlobalWindow(wlen, sli);
    period = per;
    tsBegin = sysId * slideLenGlobal;
    tsEnd = tsBegin + windowLenGlobal;
    tsOverlap = tsEnd - slideLenGlobal;
  }

};

}
/**
 * @}
 * @}
 */
#endif //ALIANCEDB_INCLUDE_JOINPROCESSOR_ABSTRACTLAZYJP_H_
