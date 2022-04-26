/*! \file CellJoinJP.h*/
//
// Created by tony on 2022/2/8.
//

#ifndef _JOINPROCESSOR_CELLJOINJP_H_
#define _JOINPROCESSOR_CELLJOINJP_H_
#include <thread>
#include <Common/Types.hpp>
#include <Utils/UtilityFunctions.hpp>
#include <barrier>

#include <JoinProcessor/AbstractJP.h>
using namespace INTELLI;
using namespace std;
namespace INTELLI {
/**
 * @ingroup JOINPROCESSOR JoinProcessors
 * @{
 */
/**
 * @defgroup JOINPROCESSOR_EAGER eager join processor
 * @{
 * The eager processors is controlled by lazy window sliders
 * @see WINDOWSLIDER_EAGER
 * @}
 * @}
 */
/**
 * @ingroup JOINPROCESSOR_EAGER
  * @class CellJoinJP JoinProcessor/CellJoinJP.h
  * @brief The class of  cell join join processor
  */
class CellJoinJP : public AbstractJP {
 private:
  /* data */
  // struct timeval timeStart;

  // hashtable hashtableS,hashtableR;
 protected:
  virtual void inlineMain();

  WindowQueue windowQueueS;
  WindowQueue windowQueueR;

 public:

  CellJoinJP() {

  }
  /**
 * @brief init the join processor with buffer/queue length and id
 * @param sLen The length of S queue and buffer
 * @param rLen The length of R queue and buffer
 * @param _sysId The system id
 */
  void init(size_t sLen, size_t rLen, size_t _sysId) {
    AbstractJP::init(sLen, rLen, _sysId);
    windowQueueS = newWindowQueue(sLen);
    windowQueueR = newWindowQueue(rLen);
  }
  /*SimpleHashJP(size_t sLen,size_t rLen,size_t _sysId)
  {

  }*/
  ~CellJoinJP() {

  }
  //outside feed a window
  /**
   * @brief Directly feed a window of tuples
   * @param ws The window of s
   */
  void feedWindowS(WindowOfTuples ws) {
    windowQueueS->push(ws);
  }
  /**
   * @brief Directly feed a window of tuples
   * @param wr The window of r
   */
  void feedWindowR(WindowOfTuples wr) {
    windowQueueR->push(wr);
  }

};
typedef std::shared_ptr<CellJoinJP> CellJoinJPPtr;

/**
 * @}
 * @}
 */
}
#endif //HYBRID_JOIN_SRC_JOINPROCESSOR_SIMPLEHASHJP_H_
