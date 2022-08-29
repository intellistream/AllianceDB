#ifndef ALIANCEDB_SRC_COMMON_TYPES_CPP_RESULT_HPP_
#define ALIANCEDB_SRC_COMMON_TYPES_CPP_RESULT_HPP_

#include <Common/Types.hpp>
namespace AllianceDB {
class Result {
 public:
  int joinNumber;
  int streamSize;
  std::string algoName;
  std::string dataSetName;
  double timeTaken;
  struct timeval timeBegin;
  explicit Result();
  Result operator++(int);
  void statPrinter();
};
}
#endif //ALIANCEDB_SRC_COMMON_TYPES_CPP_RESULT_HPP_
