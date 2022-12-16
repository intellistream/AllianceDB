#ifndef ALIANCEDB_SRC_COMMON_TYPES_CPP_RESULT_HPP_
#define ALIANCEDB_SRC_COMMON_TYPES_CPP_RESULT_HPP_

#include "Common/Tuple.hpp"
#include "Common/Types.hpp"

namespace AllianceDB {

struct JoinResult {
  struct ResultTuple {
    KeyType key;
    ValType val1, val2;
    bool operator<(ResultTuple &other) {
      if (key != other.key)
        return key < other.key;
      if (val1 != other.val1)
        return val1 < other.val1;
      return val2 < other.val2;
    }
  };
  std::vector<std::vector<ResultTuple>> window_results;
  int joinNumber;
  int streamSize;
  std::string algoName;
  std::string dataSetName;
  double timeTaken;
  struct timeval timeBegin;
  JoinResult();
  JoinResult operator++(int);
  void statPrinter();
  void Add(int wid, TuplePtr t1, TuplePtr t2);
  void Print();
  bool operator==(JoinResult &rhs) const;
};

using ResultPtr = std::shared_ptr<JoinResult>;

} // namespace AllianceDB
#endif // ALIANCEDB_SRC_COMMON_TYPES_CPP_RESULT_HPP_
