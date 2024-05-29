//
// Created by 86183 on 2023/4/18.
//

#ifndef INTELLISTREAM_VARIATIONALINFERENCE_H
#define INTELLISTREAM_VARIATIONALINFERENCE_H

#include <vector>

class VariationalInference {
 public:
  //save all keys
  std::vector<u_int64_t> keys;
  //global variable U, format: key-μ
  std::vector<std::vector<int>> U;
  //P(U) format: key-P(μ),   [1][2] = key[1]->p[2]
  std::vector<std::vector<double>> pU;
  //local variable Z, format: key-z
  std::vector<std::vector<int>> Z;
  //P(U) format: key-P(μ),   [1][2] = key[1]->p[2]
  std::vector<std::vector<double>> pZ;

  auto init() -> void;

  auto getE(std::vector<double> pX) -> double;

  auto getSigma(std::vector<double> pX) -> double;

  auto cavi(u_int64_t key) -> void;

};

#endif //INTELLISTREAM_VARIATIONALINFERENCE_H
