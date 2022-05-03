#ifndef ALIANCEDB_SRC_COMMON_TYPES_CPP_RESULT_HPP_
#define ALIANCEDB_SRC_COMMON_TYPES_CPP_RESULT_HPP_

#include <Common/Types.hpp>
namespace INTELLI {
typedef std::shared_ptr<class Result> ResultPtr;
class Result {
 public:
  int joinNumber;
  int streamSize;
  basic_string<char> algoName;
  basic_string<char> dataSetName;
  double timeTaken;
  struct timeval timeBegin;
  Result();
  Result operator++(int);
  void statPrinter();
  static std::shared_ptr<INTELLI::Result> create();
};
}
#endif //ALIANCEDB_SRC_COMMON_TYPES_CPP_RESULT_HPP_
