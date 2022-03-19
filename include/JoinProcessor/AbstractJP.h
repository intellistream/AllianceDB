/*! \file AbstractLazyJP.h*/
//
// Created by tony on 12/03/22.
//

#ifndef _JOINPROCESSOR_ABSTRACTJP_H_
#define _JOINPROCESSOR_ABSTRACTJP_H_
#include <Utils/UtilityFunctions.hpp>
#include <Common/Types.h>
#include <barrier>
#include <Utils/AbstractC20Thread.h>
#include <Utils/C20Buffers.hpp>
#include <JoinAlgo/JoinAlgoTable.h>
#include <memory>
namespace INTELLI {
class AbstractLazyJP;
/**
 * @defgroup JOINPROCESSOR JoinProcessors
 * @{
 */
/**
 * @defgroup JOINPROCESSOR_BASE common and abstract join processor
 * @{
 * The common base calss for all join processors
 * @see JOINPROCESSOR_LAZY
 * @}
 * @}
 */



/**
 * @ingroup JOINPROCESSOR_BASE
  * @class AbstractJP JoinProcessor/AbstractJP.h
  * @brief The basic class of join processor
  * @note first @ref init the JP before @ref startThread
  */
class AbstractJP : public AbstractC20Thread {
 protected:
  TuplePtrQueue TuplePtrQueueInS;
  TuplePtrQueue TuplePtrQueueInR;
  // cmd linked to window slider
  CmdQueuePtr cmdQueueIn;
  CmdQueuePtr cmdQueueOut;
  size_t sysId;
  struct timeval timeSys;  /*!< timeval structure from linux, <sys/time.h> */
  bool timeBased = false;
  size_t windowLen = 0;
  size_t windowLenGlobal = 0;
  size_t slideLenGlobal = 0;
  size_t joinedResult = 0;
  JoinAlgoTablePtr myAlgo;
  //response of my self
  void sendResponseCmd(join_cmd_t cmd) {
    cmdQueueOut->push(cmd);
  }
  void sendAck() {
    sendResponseCmd(CMD_ACK);
  }
  /**
   * @brief To test if a cmd is remained in queue and in desired cmd
   * @param cmd The desired command
   * @return true if such command exists
   */
  bool testCmd(join_cmd_t cmd) {
    if (!cmdQueueIn->empty()) {
      join_cmd_t cmdIn = *cmdQueueIn->front();
      cmdQueueIn->pop();
      if (cmdIn == cmd) {
        return true;
      }
    }
    return false;
  }
  /**
   * @brief The 'main' function of AbstractP
   * @note This is a re-implementation of AbstractC20Thread
   */
  virtual void inlineMain() {

  }
  /**
    * @brief To get the possible oldest a time stamp belongs to
    * @param ts The time stamp
    * @return The window number, start from 0
    */
  size_t oldestWindowBelong(size_t ts) {
    if (ts < windowLenGlobal) {
      return 0;
    }
    return ((ts - windowLenGlobal) / slideLenGlobal) + 1;
  }
 public:
  AbstractJP() {}
  ~ AbstractJP() {}
  /**
   * @brief input an outside command
   * @param cmd The Command
   */
  void inputCmd(join_cmd_t cmd) {
    cmdQueueIn->push(cmd);
  }
  /**
   * @brief wait and return the response of this join processor
   * @return the response command
   */
  join_cmd_t waitResponse() {
    while (cmdQueueOut->empty()) {}
    join_cmd_t ru = *cmdQueueOut->front();
    cmdQueueOut->pop();
    return ru;
  }
  /**
   * @brief init the join processor with buffer/queue length and id
   * @param sLen The length of S queue and buffer
   * @param rLen The length of R queue and buffer
   * @param _sysId The system id
   */
  virtual void init(size_t sLen, size_t rLen, size_t _sysId) {
    TuplePtrQueueInS = newTuplePtrQueue(sLen);
    TuplePtrQueueInR = newTuplePtrQueue(rLen);
    cmdQueueIn = newCmdQueue(1);
    cmdQueueOut = newCmdQueue(1);
    sysId = _sysId;
    myAlgo = newJoinAlgoTable();
  }
  /**
  * @brief to configure the window type
  * @param ts wether the slider is time-baesd
  */
  void setTimeBased(bool ts) {
    timeBased = ts;
  }
  /**
   * @brief to read the window type
   * @result wether the slider is time-baesd
   */
  bool isTimeBased() {
    return timeBased;
  }
  /**
  * @brief to set the length of window
  * @param wl the window length
  */
  void setWindowLen(size_t wl) {
    windowLen = wl;
  }
  /**
   * @brief set the window parameters of global window
   * @param wlen window length
   * @param sli slide
   */
  void setGlobalWindow(size_t wlen, size_t sli) {
    windowLenGlobal = wlen;
    slideLenGlobal = sli;
  }
  /**
   * @brief feed a tuple s into the s input queue
   * @param ts The tuple
   */
  void feedTupleS(TuplePtr ts) {
    TuplePtrQueueInS->push(ts);
  }
  /**
 * @brief feed a tuple r into the r input queue
 * @param tr The tuple
 */
  void feedTupleR(TuplePtr tr) {
    TuplePtrQueueInR->push(tr);
  }
  /**
   * @brief set the timeval struct
   * @param tv The struct to be set
   */
  void setTimeVal(struct timeval tv) {
    timeSys = tv;
  }
  /**
   * @brief Get the time stamp
   * @return The time stamp, in TIME_STEP us
   */
  size_t getTimeStamp() {
    return UtilityFunctions::timeLastUs(timeSys) / TIME_STEP;
  }
  /**
   * @brief get the join results
   * @return Tuples joined
   */
  size_t getJoinedResult() {
    return joinedResult;
  }
  int cpuBind = -1;
  /**
   * @brief bind to specific core
   * @param id The core id
   */
  void setCore(int id) {
    cpuBind = id;
  }
};

}
/**
 * @}
 * @}
 */

#endif //ALIANCEDB_SRC_JOINPROCESSOR_ABSTRACTJP_H_
