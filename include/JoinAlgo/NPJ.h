/*! \file NPJ.h*/
//
// Created by tony on 04/03/22.
//

#ifndef _JOINALGO_NPJ_NPJ_H_
#define _JOINALGO_NPJ_NPJ_H_
#include <Common/MultiThreadHashTable.h>
#include <Utils/AbstractC20Thread.h>
#include <barrier>
#include <JoinAlgo/AbstractJoinAlgo.h>
namespace INTELLI {
/**
* @ingroup INTELLI_JOINALGOS
* @{
* @defgroup INTELLI_JOINALGOS_NPJ The no partition hash join (NPJ)
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
 * @ingroup INTELLI_JOINALGOS_NPJ
 * @{
 */
/**
 * @class NPJ_thread
 * @brief The thread used by NPJ
 * @warning This is NOT an API class, please do not use it in user code, as the pointers are used for speeding
 */
class NPJ_thread : public AbstractC20Thread {
 private:
  TuplePtr *ts, *tr;
  size_t sLen = 0, rLen = 0;
  int cpu = -1;
  MultiThreadHashTablePtr table = nullptr;
  size_t result = 0;
  BarrierPtr buildBar = nullptr;
  std::shared_ptr<std::thread> threadPtr;
 protected:
  /**
   * @brief The 'main' function of NPJ thread
   * @note This is a re-implementation of AbstractC20Thread
   */
  void inlineMain();
 public:
  NPJ_thread() {};
  ~NPJ_thread() {};
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
  void init(TuplePtr *_ts,
            TuplePtr *_tr,
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

  size_t getResult() {
    return result;
  }
  void waitBuildBar(void) {
    if (buildBar) {
      buildBar->arrive_and_wait();
    }
  }
};

/**
 * @class NPJ JoinAlgo/NPJ.h
 * @brief The top class package of NPJ, providing a "join function"
 */
class NPJ : public AbstractJoinAlgo {

 private:
  vector<NPJ_thread> workers;
 public:
  NPJ() {
    setAlgoName("NPJ");
  }
  ~NPJ() {}
  /**
   * @brief The function to execute join
   * @param ts The tuples of stream S
   * @param tr The tuples of stream R
   * @param threads The parallel threads
   * @return The joined tuples
   * @todo Add AMP and NUMA support in the future, so far only generic SMP
   */
  virtual size_t join(TuplePtrQueue ts, TuplePtrQueue tr, int threads = 1);
  /**
   * @brief The function to execute join, legacy way
   * @param ts The tuples of stream S, legacy pointer
   * @param tr The tuples of stream R, legacy pointer
   * @param tsLen The length of S
   * @param trLen The length of R
   * @param threads The parallel threads
   * @return The joined tuples
   * @warning This is a legacy function, avoid using it if possible
   */
  virtual size_t join(TuplePtr *ts, TuplePtr *tr, size_t tsLen, size_t trLen, int threads = 1);
  /**
  * @brief The function to execute join, batch of one, tuple of another
  * @param ts The tuples of stream S, legacy pointer
  * @param tr The tuples of stream R, one tuple
  * @param tsLen The length of S
  * @param threads The parallel threads
  * @return The joined tuples
  * @warning This is a legacy function, avoid using it if possible
  */
  virtual size_t join(TuplePtr *ts, TuplePtr tr, size_t tsLen, int threads = 1);

  /**
  * @brief The function to execute join, batch of one, tuple of another
  * @param ts The tuples of stream S
  * @param tr The tuple of stream R
  * @param threads The parallel threads
  * @return The joined tuples
  */
  virtual size_t join(TuplePtrQueue ts, TuplePtr tr, int threads = 1);

};

typedef std::shared_ptr<NPJ> NPJPtr;
#define  newNPJ() make_shared<NPJ>()

/**
 * @class NPJSingle JoinAlgo/NPJ.h
 * @brief The top class package of single threadNPJ, providing a "join function"
 */
class NPJSingle : public AbstractJoinAlgo {

 private:

 public:
  NPJSingle() {
    setAlgoName("NPJSingle");
  }
  ~NPJSingle() {}
  /**
   * @brief The function to execute join
   * @param ts The tuples of stream S
   * @param tr The tuples of stream R
   * @param threads The parallel threads
   * @return The joined tuples
   * @todo Add AMP and NUMA support in the future, so far only generic SMP
   */
  virtual size_t join(TuplePtrQueue ts, TuplePtrQueue tr, int threads = 1);
  /**
   * @brief The function to execute join, legacy way
   * @param ts The tuples of stream S, legacy pointer
   * @param tr The tuples of stream R, legacy pointer
   * @param tsLen The length of S
   * @param trLen The length of R
   * @param threads The parallel threads
   * @return The joined tuples
   * @warning This is a legacy function, avoid using it if possible
   */
  virtual size_t join(TuplePtr *ts, TuplePtr *tr, size_t tsLen, size_t trLen, int threads = 1);
  /**
  * @brief The function to execute join, batch of one, tuple of another
  * @param ts The tuples of stream S, legacy pointer
  * @param tr The tuples of stream R, one tuple
  * @param tsLen The length of S
  * @param threads The parallel threads
  * @return The joined tuples
  * @warning This is a legacy function, avoid using it if possible
  */
  virtual size_t join(TuplePtr *ts, TuplePtr tr, size_t tsLen, int threads = 1);

  /**
  * @brief The function to execute join, batch of one, tuple of another
  * @param ts The tuples of stream S
  * @param tr The tuple of stream R
  * @param threads The parallel threads
  * @return The joined tuples
  */
  virtual size_t join(TuplePtrQueue ts, TuplePtr tr, int threads = 1);

};

typedef std::shared_ptr<NPJSingle> NPJSinglePtr;
#define  newNPJSingle() make_shared<NPJSingle>()
/***
 * @}
 */
}
#endif //ALIANCEDB_INCLUDE_JOINALGO_NPJ_NPJ_H_
