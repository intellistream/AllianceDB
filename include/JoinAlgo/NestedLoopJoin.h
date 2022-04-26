/*! \file NestedLoopJoin.h*/
#ifndef _JOINALGO_NESTEDLOOPJOIN_H_
#define _JOINALGO_NESTEDLOOPJOIN_H_
#include <JoinAlgo/AbstractJoinAlgo.h>

namespace INTELLI {
/**
* @ingroup INTELLI_JOINALGOS
* @{
* @defgroup INTELLI_JOINALGOS_NLJ The simplest nested loop join
* @{
*Just compare keys in the loop and test if they are matched.
*@}
 * @}
*/
/**
 * @ingroup INTELLI_JOINALGOS_NLJ
 * @class NestedLoopJoin
 *  @brief The top class package of Nested Loop Join providing a "join function"
 *  @note Just single thread, the threads parameter is useless
 */
class NestedLoopJoin : public AbstractJoinAlgo {
 public:
  NestedLoopJoin() {
    setAlgoName("NestedLoopJoin");
  }
  ~NestedLoopJoin() {}
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
   * @brief The function to execute join, batch of both
   * @param ts The tuples of stream S
   * @param tr The tuples of stream R
   * @param threads The parallel threads
   * @return The joined tuples
   * @todo Add AMP and NUMA support in the future, so far only generic SMP
   */
  virtual size_t join(TuplePtrQueue ts, TuplePtrQueue tr, int threads = 1);
  /**
  * @brief The function to execute join, batch of one, tuple of another
  * @param ts The tuples of stream S
  * @param tr The tuple of stream R
  * @param threads The parallel threads
  * @return The joined tuples
  */
  virtual size_t join(TuplePtrQueue ts, TuplePtr tr, int threads = 1);
};
typedef std::shared_ptr<NestedLoopJoin> NestedLoopJoinPtr;
#define  newNestedLoopJoin() make_shared<NestedLoopJoin>()
}
#endif //ALIANCEDB_INCLUDE_JOINALGO_NESTEDLOOPJOIN_H_
