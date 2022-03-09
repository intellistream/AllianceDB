//
// Created by tony on 2022/2/8.
//

#ifndef JOINPROCESSOR_SIMPLEHASHJP_H_
#define JOINPROCESSOR_SIMPLEHASHJP_H_
#include <thread>
#include <Common/Types.h>
#include <Utils/UtilityFunctions.hpp>
#include <barrier>
using namespace INTELLI;
using namespace std;
namespace INTELLI {
/*class:SimpleHashJP
description: join processor for simple hash join
date:20220209
*/
class SimpleHashJP {
 private:
  /* data */
  struct timeval timeStart;
  // hashtable hashtableS,hashtableR;
 protected:
  virtual void inlineRun();
  BarrierPtr initBar = nullptr;
  std::shared_ptr<std::thread> threadPtr;
  size_t joinedResult = 0;
  size_t sysId = 0;
  //size_t timeStart=0;
  // cmd linked to window slider
  CmdQueuePtr cmdQueueIn;
  CmdQueuePtr cmdQueueOut;
  //tuple Queue
  TuplePtrQueue TuplePtrQueueLocalS;
  TuplePtrQueue TuplePtrQueueLocalR;
  WindowQueue windowQueueS;
  WindowQueue windowQueueR;
  size_t windowLen = 0;
  bool timeBased = false;
  int cpuBind = -1;
  //response of my self
  void sendResponseCmd(join_cmd_t cmd) {
    cmdQueueOut->push(cmd);
  }
  void sendAck() {
    sendResponseCmd(CMD_ACK);
    //  cout<<"ack sent from"<<sysId<<endl;
  }
 public:

  SimpleHashJP() {

  }
  void init(size_t sLen, size_t rLen, size_t _sysId) {
    TuplePtrQueueLocalS = newTuplePtrQueue(sLen);
    TuplePtrQueueLocalR = newTuplePtrQueue(rLen);
    windowQueueS = newWindowQueue(sLen);
    windowQueueR = newWindowQueue(rLen);
    cmdQueueIn = newCmdQueue(1);
    cmdQueueOut = newCmdQueue(1);
    joinedResult = 0;
    sysId = _sysId;
  }
  /*SimpleHashJP(size_t sLen,size_t rLen,size_t _sysId)
  {

  }*/
  ~SimpleHashJP() {

  }
  void setCore(int id) {
    cpuBind = id;
  }
  //start the JP thread
  void start() {
    auto fun = [this]() {
      inlineRun();
    };
    threadPtr = std::make_shared<std::thread>(fun);
  }
  //join the JP thread
  void join() {
    threadPtr->join();
  }
  //detach the JP thread
  void detach() {
    threadPtr->detach();
  }
  size_t getJoinedResult() {
    return joinedResult;
  }
  //input a cmd by outside
  void inputCmd(join_cmd_t cmd) {
    cmdQueueIn->push(cmd);
  }
  //for outside cmd sender to receive response
  join_cmd_t waitResponse() {
    while (cmdQueueOut->empty()) {}
    join_cmd_t ru = *cmdQueueOut->front();
    cmdQueueOut->pop();
    return ru;
  }
  //outside feed a Tuple S
  void feedTupleS(TuplePtr ts) {
    TuplePtrQueueLocalS->push(ts);
  }
  void feedTupleR(TuplePtr tr) {
    TuplePtrQueueLocalR->push(tr);
  }
  //outside feed a window
  void feedWindowS(WindowOfTuples ws) {
    windowQueueS->push(ws);
  }
  void feedWindowR(WindowOfTuples wr) {
    windowQueueR->push(wr);
  }
  // configure the window type
  void setTimeBased(bool ts) {
    timeBased = ts;
  }
  //read the window type
  bool isTimeBased() {
    return timeBased;
  }
  // set the length of window
  void setWindowLen(size_t wl) {
    windowLen = wl;
  }
  //set the base line of time
  void setTimeStart(struct timeval t) {
    timeStart = t;
  }
  //
  size_t getTimeStamp() {
    return UtilityFunctions::timeLastUs(timeStart) / TIME_STEP;
  }
  //init Barrier if need
  void setInitBar(BarrierPtr barPrev) {
    initBar = barPrev;
  }
  void waitInitBar(void) {
    if (initBar) {
      initBar->arrive_and_wait();
    }
  }
};
typedef std::shared_ptr<SimpleHashJP> SimpleHashJPPtr;
}
#endif //HYBRID_JOIN_SRC_JOINPROCESSOR_SIMPLEHASHJP_H_
