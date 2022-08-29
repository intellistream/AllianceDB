#include <Common/Types.hpp>
#include <Common/Tuple.hpp>
#include <Common/Result.hpp>

AllianceDB::Result::Result() : joinNumber(0), streamSize(0), timeTaken(0) {}

AllianceDB::Result AllianceDB::Result::operator++(int) {
  Result tmp(*this);
  joinNumber++;
  return tmp;
}

void AllianceDB::Result::statPrinter() {
  int n1 = 12;
  int n2 = 30;
  std::cout << BAR << std::endl;
  std::cout << setiosflags(std::ios::right) << std::setw(n1) << "Stats"
            << setiosflags(std::ios::right) << std::setw(n2) << "Value"
            << resetiosflags(std::ios::right) << std::endl;
  std::cout << BAR << std::endl;
  std::string statNames[] = {"AlgoName", "DatasetName", "StreamSize", "WindowSize", "Result", "TimeTaken"};
  std::string values[] = {algoName, DATASET_NAME, std::to_string(streamSize), std::to_string(WINDOW_SIZE),
                          std::to_string(joinNumber), std::to_string(timeTaken) + "ms"};

  for (int i = 0; i < 6; ++i) {
    std::cout << setiosflags(std::ios::right) << std::setw(n1) << statNames[i]
              << setiosflags(std::ios::right) << std::setw(n2) << values[i] << std::endl;
  }
  std::cout << BAR << std::endl;
}
