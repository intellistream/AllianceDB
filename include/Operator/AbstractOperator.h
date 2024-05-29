/*! \file AbstractOperator.h*/
//
// Created by tony on 22/11/22.
//

#ifndef ADB_INCLUDE_OPERATOR_ABSTRACTOPERATOR_H_
#define ADB_INCLUDE_OPERATOR_ABSTRACTOPERATOR_H_

#include <Common/Window.h>
#include <Common/Tuples.h>
#include <Utils/UtilityFunctions.hpp>
#include <cassert>
#include <Utils/ConfigMap.hpp>
#include <WaterMarker/WMTable.h>
//#include <Utils/Logger.hpp>
#define OP_INFO printf
#define OP_ERROR printf
#define OP_WARNNING INTELLI_WARNING

using namespace INTELLI;
namespace OoOJoin {
/**
* @ingroup ADB_OPERATORS
* @typedef AbstractOperatorPtr
* @brief The class to describe a shared pointer to @ref AbstractOperator
*/
typedef std::shared_ptr<class AbstractOperator> AbstractOperatorPtr;
/**
 * @ingroup ADB_OPERATORS
 * @def newAbstractOperator
 * @brief (Macro) To creat a new @ref AbstractOperator under shared pointer.
 */
#define newAbstractOperator std::make_shared<OoOJoin::AbstractOperator>
/**
 * @ingroup ADB_OPERATORS
 * @def timeTrackingStart
 * @brief (Macro) start a time tracking and record it at variable v, v is not defined yet
 * @param v the variable name, must start with tt_
 */
#define timeTrackingStart(v); tsType v;v=UtilityFunctions::timeLastUs(timeBaseStruct);
/**
 * @ingroup ADB_OPERATORS
 * @def timeTrackingStartNoClaim
 * @brief (Macro) start a time tracking and record it at variable v, v is previously defined
 * @param v the variable name, must start with tt_
 */
#define timeTrackingStartNoClaim(v); v=UtilityFunctions::timeLastUs(timeBaseStruct);
/**
 * @ingroup ADB_OPERATORS
 * @def timeTrackingEnd
 * @brief (Macro) end a time tracking of variable v and return
 * @param v the variable name, must start with tt_
 * @return the time last
 */
#define timeTrackingEnd(v) (UtilityFunctions::timeLastUs(timeBaseStruct)-v)

/**
 * @ingroup ADB_OPERATORS
 * @class AbstractOperator Operator/AbstractOperator.h
 * @brief The abstraction to describe a join operator, providing virtual function of using the operation
 * @note require configurations:
 * - "windowLen" U64: The length of window, real-world time in us
 * - "slideLen" U64: The length of slide, real world time in us
 * - "sLen" U64: The length of S buffer
 * - "rLen" U64: The length of R buffer
 * - "timeStep" U64. Internal simulation step in us
 */
/**
* @todo Finish the watermark generator part
*/
class AbstractOperator {
 protected:
  struct timeval timeBaseStruct{};
  size_t windowLen = 0;
  size_t slideLen = 0;
  size_t sLen = 0, rLen = 0;
  int threads = 0;
  tsType timeStep = 0;
  tsType timeBreakDownAll = 0;
  uint64_t joinSum = 0;
  /**
   *@brief set the final processed time for all tuples
   */
  void setFinalProcessedTime();

 public:
  ConfigMapPtr config = nullptr;

  AbstractOperator() = default;

  ~AbstractOperator() = default;

  /**
   * @brief Set the window parameters of the whole operator
   * @param _wl window length
   * @param _sli slide length
   */
  void setWindow(size_t _wl, size_t _sli) {
    windowLen = _wl;
    slideLen = _sli;
  }

  /**
   * @brief Set buffer length of S and R buffer
   * @param _sLen the length of S buffer
   * @param _rLen the length of R buffer
   */
  void setBufferLen(size_t _sLen, size_t _rLen) {
    sLen = _sLen;
    rLen = _rLen;
  }

  /**
   * @brief Synchronize the time structure with outside setting
   * @param tv The outside time structure
   */
  void syncTimeStruct(struct timeval tv) {
    timeBaseStruct = tv;
  }

  /**
  * @brief Set the config map related to this operator
  * @param cfg The config map
   * @return bool whether the config is successfully set
  */
  virtual bool setConfig(ConfigMapPtr cfg);

  /**
  * @brief feed a tuple s into the Operator
  * @param ts The tuple
   * @warning The current version is simplified and assuming only used in SINGLE THREAD!
   * @return bool, whether tuple is fed.
  */
  virtual bool feedTupleS(TrackTuplePtr ts);

  /**
    * @brief feed a tuple R into the Operator
    * @param tr The tuple
    * @warning The current version is simplified and assuming only used in SINGLE THREAD!
    *  @return bool, whether tuple is fed.
    */
  virtual bool feedTupleR(TrackTuplePtr tr);

  /**
   * @brief start this operator
   * @return bool, whether start successfully
   */
  virtual bool start();

  /**
   * @brief stop this operator
   * @return bool, whether start successfully
   */
  virtual bool stop();

  /**
   * @brief get the joined sum result
   * @return The result
   */
  virtual size_t getResult();

  /**
   * @brief get the joined sum result under AQP estimation
   * @return The result
   */
  virtual size_t getAQPResult();

  /**
   * @brief get the break down information of processing time
   * @warning should check the nullptr of output
   * @return The ConfigMapPtr which contains breakdown information, null if no breakdown supported
   */
  virtual ConfigMapPtr getTimeBreakDown();
  /**
   * @brief get the throughput under lazy running
   * @return the throughput
   */
  virtual double getLazyRunningThroughput();
};

}
#endif //INTELLISTREAM_INCLUDE_OPERATOR_ABSTRACTOPERATOR_H_
