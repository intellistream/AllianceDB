/*! \file JoinAlgoTableh*/
/**
 * @note If new algorithm is added, please change this file and its .cpp
 */
//
// Created by tony on 11/03/22.
//

#ifndef _JOINALGO_JOINALGOTABLE_H_
#define _JOINALGO_JOINALGOTABLE_H_

#include <JoinAlgos/AbstractJoinAlgo.h>
#include <JoinAlgos/NestedLoopJoin.h>
#include <JoinAlgos/NPJ/NPJ.h>
#include <map>
/**
 * add the header of other algorithms here
 */
namespace OoOJoin {
/**
   * @ingroup ADB_JOINALGOS_ABSTRACT
   * @class JoinAlgoTable JoinAlgos/JoinAlgoTable.h
   * @brief The table contains all supported algos
   * @note Please edit this class if new algorithm is added
   */
class JoinAlgoTable {
 protected:
  std::vector<AbstractJoinAlgoPtr> algos;
  std::map<std::string, AbstractJoinAlgoPtr> algoMap;
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
  AbstractJoinAlgoPtr findAlgo(std::string name) {
    /*size_t maxIdx = algos.size();
    for (size_t i = 0; i < maxIdx; i++) {
      if (algos[i]->getAlgoName() == name) {
        return algos[i];
      }
    }*/
    if (algoMap.count(name)) {
      return algoMap[name];
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

/**
 * @cite JoinAlgoTablePtr
 * @brief The class to describe a shared pointer to @ref JoinAlgoTable
 */
typedef std::shared_ptr<JoinAlgoTable> JoinAlgoTablePtr;
/**
 * @cite newJoinAlgoTable
 * @brief (Macro) To creat a new @ref JoinAlgoTable under shared pointer.
 */
#define  newJoinAlgoTable std::make_shared<JoinAlgoTable>
/**
 * @ingroup ADB_JOINALGOS_ABSTRACT
 * @enum join_algo_index_t JoinAlgos/JoinAlgoTable.h
 * @brief The system default index for join algorithms
 */
typedef enum {
  /**
   * The 0 for AbtractJoinAlgo
   */
  JOINALGO_NULL = 0,
  /**
   * The 1 for NestedLoop
   */
  JOINALGO_NESTEDLOOP = 1,
  /**
   * =2 for @ref NPJ
   */
  JOINALGO_NPJ = 2,
  /**
  * =3 for @ref NPJSingle
  */
  JOINALGO_NPJ_SINGLE = 3,
} join_algo_index_t;
}
#endif //ALIANCEDB_INCLUDE_JOINALGO_JOINALGOTABLE_H_
