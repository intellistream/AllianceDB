/*! \file NPJ.h*/
//
// Created by tony on 04/03/22.
//

#ifndef _JOINALGO_NPJ_NPJ_H_
#define _JOINALGO_NPJ_NPJ_H_

#include <JoinAlgos/NPJ/MultiThreadHashTable.h>
#include <Utils/AbstractC20Thread.hpp>
#include <barrier>
#include <JoinAlgos/AbstractJoinAlgo.h>

namespace OoOJoin {
/**
* @ingroup ADB_JOINALGOS
* @{
* @defgroup ADB_JOINALGOS_NPJ The no partition hash join (NPJ)
* @{
*NPJ is a parallel version of the
    canonical hash join algorithm. Both input relations are divided into
equisized portions to be assigned to a number of threads. In the
build phase, all threads populate a shared hash table with all tuples
    of R. After synchronization via a barrier, all threads enter the probe
    phase, and concurrently find matching join tuples in their assigned
    portions of S.
*
*/
/**
 * @ingroup ADB_JOINALGOS_NPJ
 * @{
 */
/**
 * @class NPJ_thread
 * @brief The thread used by NPJ
 * @warning This is NOT an API class, please do not use it in user code, as the pointers are used for speeding
 */
class NPJ_thread : public AbstractC20Thread {
 private:
  NPJTuplePtr *ts, *tr;
  size_t sLen = 0, rLen = 0;
  int cpu = -1;
  MultiThreadHashTablePtr table = nullptr;
  size_t result = 0;
  BarrierPtr buildBar = nullptr;
  std::shared_ptr<std::thread> threadPtr;
  struct timeval timeBaseStruct;
  uint64_t joinSum = 0;
  //tsType timeStep = 1;
 protected:
  /**
   * @brief The 'main' function of NPJ thread
   * @note This is a re-implementation of AbstractC20Thread
   */
  void inlineMain() override;

 public:
  NPJ_thread() = default;

  ~NPJ_thread() = default;

  /**
   * @brief Synchronize the time structure with outside setting
   * @param tv The outside time structure
   */
  void syncTimeStruct(struct timeval tv) {
    timeBaseStruct = tv;
  }
  /*void setTimeStep(tsType _timeStep) {
    timeStep = _timeStep;
  }*/
  /**
   * @brief THe init function
   * @param _ts Memory pointer of S
   * @param _tr Memory pointer of S
   * @param _sLen Length of S
   * @param _rLen Length of R
   * @param _cpu Core to bind, -1 means let OS decide
   * @param _table The shared pointer of hash table
   * @param bar THe barrier used for build phase
   */
  void init(NPJTuplePtr *_ts,
            NPJTuplePtr *_tr,
            size_t _sLen,
            size_t _rLen,
            int _cpu,
            MultiThreadHashTablePtr _table,
            BarrierPtr bar) {
    ts = _ts;
    tr = _tr;
    sLen = _sLen;
    rLen = _rLen;
    cpu = _cpu;
    table = _table;
    buildBar = bar;
  }

  size_t getResult() const {
    return result;
  }

  void waitBuildBar(void) {
    if (buildBar) {
      buildBar->arrive_and_wait();
    }
  }
  void setJoinSum(uint64_t js) {
    joinSum = js;
  }
};

/**
 * @class NPJ JoinAlgos/NPJ/NPJ.h
 * @brief The top class package of NPJ, providing a "join function"
 */
class NPJ : public AbstractJoinAlgo {

 private:
  std::vector<NPJ_thread> workers;
 public:
  NPJ() {
    setAlgoName("NPJ");
  }

  ~NPJ() = default;

  /**
 * @brief The function to execute join,
 * @param windS The window of S tuples
  * @param windR The window of R tuples
  * @param threads The threads for executing this join
 * @return The joined tuples
 */
  size_t join(C20Buffer<OoOJoin::TrackTuplePtr> windS,
              C20Buffer<OoOJoin::TrackTuplePtr> windR,
              int threads = 1) override;

};

/**
 * @cite NPJPtr
 * @brief The class to describe a shared pointer to @ref NPJ
 */
typedef std::shared_ptr<NPJ> NPJPtr;
/**
 * @cite newNPJ
 * @brief (Macro) To creat a new @ref NPJ under shared pointer.
 */
#define  newNPJ std::make_shared<NPJ>

/**
 * @class NPJSingle JoinAlgos/NPJ/NPJ.h
 * @brief The top class package of single threadNPJ, providing a "join function"
 */
class NPJSingle : public AbstractJoinAlgo {

 private:

 public:
  NPJSingle() {
    setAlgoName("NPJSingle");
  }

  ~NPJSingle() = default;

  /**
  * @brief The function to execute join,
  * @param windS The window of S tuples
   * @param windR The window of R tuples
   * @param threads The threads for executing this join
  * @return The joined tuples
  */
  virtual size_t join(C20Buffer<OoOJoin::TrackTuplePtr> windS,
                      C20Buffer<OoOJoin::TrackTuplePtr> windR,
                      int threads = 1);

};

/**
 * @typedef NPJSinglePtr
 * @brief The class to describe a shared pointer to @ref NPJSingle
 */
typedef std::shared_ptr<NPJSingle> NPJSinglePtr;
/**
 * @def newNPJSingle
 * @brief (Macro) To creat a new @ref NPJSingle under shared pointer.
 */
#define  newNPJSingle std::make_shared<NPJSingle>
/***
 * @}
 */
}
#endif //ALIANCEDB_INCLUDE_JOINALGO_NPJ_NPJ_H_
