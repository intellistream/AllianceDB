/*! \file JoinAlgoTableh*/
/**
 * @note If new algorithm is added, please change this file and its .cpp
 */
//
// Created by tony on 11/03/22.
//

#ifndef _JOINALGO_JOINALGOTABLE_H_
#define _JOINALGO_JOINALGOTABLE_H_
#include <JoinAlgo/AbstractJoinAlgo.h>
namespace INTELLI {
/**
   * @ingroup INTELLI_JOINALGOS_ABSTRACT
   * @class JoinAlgoTable JoinAlgo/JoinAlgoTable.h
   * @brief The table contains all supported algos
   * @note Please edit this class if new algorithm is added
   */
class JoinAlgoTable {
 protected:
  vector<AbstractJoinAlgoPtr> algos;
 public:
  JoinAlgoTable();
  ~JoinAlgoTable() {

  }
  /**
   * @brief find a algorithm in the table according to its index
   * @param idx The index of algorithm
   * @return The algorithm
   */
  AbstractJoinAlgoPtr findAlgo(size_t idx) {
    if (idx < algos.size()) {
      return algos[idx];
    }
    return algos[0];
  }
  /**
   * @brief find a algorithm in the table according to its index
   * @param name The name of algorithm
   * @return The algorithm
   */
  AbstractJoinAlgoPtr findAlgo(string name) {
    size_t maxIdx = algos.size();
    for (size_t i = 0; i < maxIdx; i++) {
      if (algos[i]->getAlgoName() == name) {
        return algos[i];
      }
    }
    return algos[0];
  }
  /**
  * @brief To register a new algorithm
  * @param anew The new algorithm
  * @return The index of new registered algorithm
  */
  size_t registerNewAlgo(AbstractJoinAlgoPtr anew) {
    algos.push_back(anew);
    return algos.size() - 1;
  }
};

typedef std::shared_ptr<JoinAlgoTable> JoinAlgoTablePtr;
#define  newJoinAlgoTable() make_shared<JoinAlgoTable>()
/**
 * @ingroup INTELLI_JOINALGOS_ABSTRACT
 * @enum join_algo_index_t JoinAlgo/JoinAlgoTable.h
 * @brief The system default index for join algorithms
 */
typedef enum {
  /**
   * The 0 for AbtractJoinAlgo
   */
  JOINALGO_NULL = 0,
  /**
   * =1 for @ref NPJ
   */
  JOINALGO_NPJ = 1,
  /**
  * =2 for @ref NPJSingle
  */
  JOINALGO_NPJ_SINGLE = 2,
} join_algo_index_t;
}
#endif //ALIANCEDB_INCLUDE_JOINALGO_JOINALGOTABLE_H_
