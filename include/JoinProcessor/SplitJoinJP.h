/*! \file SplitJoinJP.h*/
//
// Created by tony on 18/03/22.
//

#ifndef _JOINPROCESSOR_SPLITJOINJP_H_
#define _JOINPROCESSOR_SPLITJOINJP_H_
#include <thread>
#include <Common/Types.hpp>
#include <Utils/UtilityFunctions.hpp>
#include <barrier>
#include <JoinProcessor/AbstractJP.h>

using namespace AllianceDB;
using namespace std;
namespace AllianceDB {
/**
 * @ingroup JOINPROCESSOR_EAGER
  * @class SplitJoinJP JoinProcessor/SplitJoinJP.h
  * @brief The class of  split join join processor
  */
class SplitJoinJP : public AbstractJP {
 protected:
  virtual void inlineMain();
  /**
   * @brief local queue storage of S, used for manage S window
   */
  TuplePtrQueue TuplePtrQueueLocalS;
  /**
 * @brief local queue storage of R, used for manage R window
 */
  TuplePtrQueue TuplePtrQueueLocalR;
  /*
   *  @brief A 'current window' copy of TuplePtrQueueLocalS when join a tuple
   */
  //C20Buffer<TuplePtr> windowS;
  /*
   *  @brief A 'current window' copy of TuplePtrQueueLocalR when join a tuple
   */
  // C20Buffer<TuplePtr> windowR;
  size_t sCnt = 0;
  size_t sMax = 0;
  void expireS(size_t cond);
  void expireR(size_t cond);
  void joinS(TuplePtr ts);
  void joinR(TuplePtr tr);
 public:
  SplitJoinJP() {

  }
  ~SplitJoinJP() {

  }
  /**
 * @brief init the join processor with buffer/queue length and id
 * @param sLen The length of S queue and buffer
 * @param rLen The length of r queue and buffer
 * @param _sysId The system id
 */
  void init(size_t sLen, size_t rLen, size_t _sysId) {
    AbstractJP::init(sLen, rLen, _sysId);
    TuplePtrQueueLocalS = newTuplePtrQueue(sLen);
    TuplePtrQueueLocalR = newTuplePtrQueue(rLen);
    //windowS = C20Buffer<TuplePtr>(sLen);
    // windowR = C20Buffer<TuplePtr>(rLen);
    sCnt = 0;
  }
  /**
   * @brief Set the max value of sCnt variable
   * @param ms The max value
   */
  void setMaxSCnt(size_t ms) {
    sMax = ms;
  }
  /*
   * @brief feed a tuple s into the s input queue
   * @param ts The tuple
   *//*
  virtual void feedTupleS(TuplePtr ts) {
    sCnt++;
    if (sCnt == sysId + 1) //should process this S
    {
      TuplePtrQueueInS->push(ts);
    }
    if (sCnt == sMax) {
      sCnt = 0;
    }
  }*/
};
typedef std::shared_ptr<SplitJoinJP> SplitJoinJPPtr;
}
#endif //ALIANCEDB_INCLUDE_JOINPROCESSOR_SPLITJP_H_
