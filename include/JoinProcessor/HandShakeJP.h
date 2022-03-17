//
// Created by tony on 2022/2/9.
//
#pragma once
#ifndef JOINPROCESSOR_HANDSHAKEJP_H_
#define JOINPROCESSOR_HANDSHAKEJP_H_
#include <JoinProcessor/CellJoinJP.h>
#include <memory>

using namespace std;
using namespace INTELLI;
namespace INTELLI {
class HandShakeJP;
typedef std::shared_ptr<HandShakeJP> HandShakeJPPtr;
/*class:HandShakeHashJP
description: join processor for handshake hash join
note: S->, R<-
date:20220228
*/
class HandShakeJP : public CellJoinJP {
 protected:
  /* data */
  HandShakeJPPtr leftJP = nullptr;
  HandShakeJPPtr rightJP = nullptr;
  TuplePtrQueue selfWindowS, selfWindowR;
  size_t countR = 0, countS = 0;
  size_t timeOffsetS, timeOffsetR;
  size_t rQueue = 0, sQueue = 0;
  void setupQueue();
  void expireS(size_t cond);
  void expireR(size_t cond);
  virtual void inlineMain();
 public:
  void setLeft(HandShakeJPPtr l) {
    leftJP = l;
  }
  void setRight(HandShakeJPPtr r) {
    rightJP = r;
  }

  HandShakeJP(/* args */) {

  }
  ~HandShakeJP() {

  }
  void setTimeOffset(size_t ts, size_t tr) {
    timeOffsetS = ts;
    timeOffsetR = tr;
  }
  void paraseTupleS();
  void paraseTupleR();
  void setNeighborJP(HandShakeJPPtr l, HandShakeJPPtr r) {
    leftJP = l;
    rightJP = r;
  }
  void init(size_t sLen, size_t rLen, size_t _sysId) {
    rQueue = rLen;
    sQueue = sLen;
    joinedResult = 0;
    sysId = _sysId;
  }
  void feedTupleS(TuplePtr ts);

  void feedTupleR(TuplePtr tr);

};

}
#endif //HYBRID_JOIN_INCLUDE_JOINPROCESSOR_HANDSHAKEHASHJP_H_