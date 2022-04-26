/*! \file AbstractJoinAlgo.h*/
//
// Created by tony on 11/03/22.
//

#ifndef _JOINALGO_ABSTRACTJOINALGO_H_
#define _JOINALGO_ABSTRACTJOINALGO_H_
#include <Common/Types.hpp>
#include <Common/Tuple.hpp>
#include <string>
#include <memory>
namespace INTELLI {
/**
* @defgroup INTELLI_JOINALGOS The specific join algorithms
* @{
* State-of-art joins algorithms. We use a register to table called @ref JoinAlgoTable to manage and access different algos in an unified way, and user-defined
 * new algos should also be registered in that table.
 * */
/**
 * @defgroup INTELLI_JOINALGOS_ABSTRACT Common Abstraction and Interface
 * @{
 */
/**
 * @ingroup INTELLI_JOINALGOS_ABSTRACT
 * @class AbstractJoinAlgo JoinAlgo/AbstractJoinAlgo.h
 * @brief The abstraction to describe a join algorithm, providing virtual function of join
 * @note The derived new algorithm should contain the @ref join interface, for batch-batch and batch-tuple
 */
class AbstractJoinAlgo {
 protected:
  string nameTag;
 public:
  AbstractJoinAlgo() {
    setAlgoName("NULL");
  }
  ~AbstractJoinAlgo() {}

  /**
   * @brief The function to execute join, batch of both
   * @param ts The tuples of stream S
   * @param tr The tuples of stream R
   * @param threads The parallel threads
   * @return The joined tuples
   * @todo Add AMP and NUMA support in the future, so far only generic SMP
   */
  virtual size_t join(TuplePtrQueue ts, TuplePtrQueue tr, int threads = 1) {
    assert(tr);
    assert(ts);
    assert(threads);
    return 0;
  }
  /**
   * @brief The function to execute join, batch of both, legacy way
   * @param ts The tuples of stream S, legacy pointer
   * @param tr The tuples of stream R, legacy pointer
   * @param tsLen The length of S
   * @param trLen The length of R
   * @param threads The parallel threads
   * @return The joined tuples
   * @warning This is a legacy function, avoid using it if possible
   */
  virtual size_t join(TuplePtr *ts, TuplePtr *tr, size_t tsLen, size_t trLen, int threads = 1) {
    assert(tr != ts);
    assert(trLen >= 0);
    assert(tsLen >= 0);
    assert(threads);
    return 0;
  }
  /**
   * @brief The function to execute join, batch of one, tuple of another
   * @param ts The tuples of stream S, legacy pointer
   * @param tr The tuples of stream R, one tuple
   * @param tsLen The length of S
   * @param threads The parallel threads
   * @return The joined tuples
   * @warning This is a legacy function, avoid using it if possible
   */
  virtual size_t join(TuplePtr *ts, TuplePtr tr, size_t tsLen, int threads = 1) {
    assert(ts);
    assert(tr);
    assert(tsLen >= 0);
    assert(threads);
    return 0;
  }
  /**
  * @brief The function to execute join, batch of one, tuple of another
  * @param ts The tuples of stream S
  * @param tr The tuple of stream R
  * @param threads The parallel threads
  * @return The joined tuples
  */
  virtual size_t join(TuplePtrQueue ts, TuplePtr tr, int threads = 1) {
    assert(tr);
    assert(ts);
    assert(threads);
    return 0;
  }
  /**
   * @brief set the name of algorithm
   * @param name Algorithm name
   */
  void setAlgoName(string name) {
    nameTag = name;
  }
  /**
  * @brief get the name of algorithm
  * @return The name
  */
  string getAlgoName() {
    return nameTag;
  }
};
typedef std::shared_ptr<AbstractJoinAlgo> AbstractJoinAlgoPtr;
#define  newAbstractJoinAlgo() make_shared<AbstractJoinAlgo>()
/**
 * @}
 * @}
 */
}

#endif //ALIANCEDB_INCLUDE_JOINALGO_ABSTRACTJOINALGO_H_
