//
// Created by tony on 2022/2/9.
//
#pragma once
#ifndef JOINPROCESSOR_HANDSHAKEHASHJP_H_
#define JOINPROCESSOR_HANDSHAKEHASHJP_H_
#include <JoinProcessor/SimpleHashJP.h>
#include <memory>
using namespace std;
using namespace INTELLI;
namespace INTELLI {
class HandShakeHashJP;
typedef std::shared_ptr<HandShakeHashJP> HandShakeHashJPPtr;
/*class:HandShakeHashJP
description: join processor for handshake hash join
note: S->, R<-
date:20220228
*/
class HandShakeHashJP : public SimpleHashJP {
 protected:
  /* data */
  HandShakeHashJPPtr leftJP = nullptr;
  HandShakeHashJPPtr rightJP = nullptr;
  TuplePtrQueue selfWindowS, selfWindowR;
  size_t countR = 0, countS = 0;
  size_t timeOffsetS, timeOffsetR;
  size_t rQueue = 0, sQueue = 0;
  void setupQueue();
  void expireS(size_t cond);
  void expireR(size_t cond);

 public:
  void setLeft(HandShakeHashJPPtr l) {
    leftJP = l;
  }
  void setRight(HandShakeHashJPPtr r) {
    rightJP = r;
  }
  void start() {
    countR = 0;
    countS = 0;
    SimpleHashJP::start();
  }
  void inlineRun();
  HandShakeHashJP(/* args */) {

  }
  ~HandShakeHashJP() {

  }
  void setTimeOffset(size_t ts, size_t tr) {
    timeOffsetS = ts;
    timeOffsetR = tr;
  }
  void paraseTupleS();
  void paraseTupleR();
  void setNeighborJP(HandShakeHashJPPtr l, HandShakeHashJPPtr r) {
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
