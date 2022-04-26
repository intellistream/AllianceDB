//
// Created by shuhao.zhang on 26/4/22.
//

#include <Common/Result.hpp>

INTELLI::Result::Result() : joinNumber(0), streamSize(0), timeTaken(0) {}

INTELLI::Result INTELLI::Result::operator++(int) {
  Result tmp(*this);
  joinNumber++;
  return tmp;
}

void INTELLI::Result::statPrinter() {
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
