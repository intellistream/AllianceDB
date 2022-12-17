#ifndef ALIANCEDB_SRC_COMMON_TYPES_CPP_RESULT_HPP_
#define ALIANCEDB_SRC_COMMON_TYPES_CPP_RESULT_HPP_

#include "Common/Tuple.hpp"
#include "Common/Types.hpp"

#include <tuple>

namespace AllianceDB {

struct JoinResult {
  using Tuple = std::tuple<KeyType, ValType, ValType>;
  std::vector<std::vector<Tuple>> window_results;
  int joinNumber;
  int streamSize;
  std::string algoName;
  std::string dataSetName;
  double timeTaken;
  struct timeval timeBegin;
  JoinResult();
  void statPrinter();
  void Add(int wid, TuplePtr t1, TuplePtr t2);
  void Print();
  bool operator==(JoinResult &rhs) const;
  size_t Hash();
};

using ResultPtr = std::shared_ptr<JoinResult>;

} // namespace AllianceDB
#endif // ALIANCEDB_SRC_COMMON_TYPES_CPP_RESULT_HPP_
