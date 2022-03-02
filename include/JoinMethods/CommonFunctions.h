
// Created by Wang Chenyu on 31/8/21.

#ifndef LAZYWINDOWJOIN_COMMONFUNCTIONS_H
#define LAZYWINDOWJOIN_COMMONFUNCTIONS_H

#include <iostream>
#include <fstream>
#include <list>
#include <mutex>
#include <unordered_map>
#include <Common/Types.h>
#include <Utils/Logger.hpp>

#ifndef EXEC
#define EXEC execute(joinResult, relationCouple)
#endif

#ifndef TESTMODULE
#define TESTMODULE test(joinResult, relationCouple)
#endif
namespace INTELLI {
class CommonFunction {
  /** Build a relation with the tuples in the dataset
   * Relation should contains all the tuples that will
   * be pushed to Window R and window S
   **/
 public:
  void buildRelation(tupleQueue &relationPtr, const std::string &fileName);
};
}

#endif //LAZYWINDOWJOIN_COMMONFUNCTIONS_H
