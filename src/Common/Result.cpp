//
// Created by shuhao.zhang on 26/4/22.
//

#include "Common/Result.hpp"

#include <algorithm>

using namespace AllianceDB;
using namespace std;

JoinResult::JoinResult() : joinNumber(0), streamSize(0), timeTaken(0) {}

void JoinResult::Add(int wid, TuplePtr t1, TuplePtr t2) {
  if (window_results.size() <= wid) {
    window_results.resize(wid + 1);
  }
  window_results[wid].push_back(ResultTuple(t1->key, t1->val, t2->val));
}

bool operator==(JoinResult &lhs, JoinResult &rhs) {
  if (lhs.window_results.size() != rhs.window_results.size()) {
    return false;
  }
  for (size_t i = 0; i < lhs.window_results.size(); i++) {
    if (lhs.window_results[i].size() != rhs.window_results[i].size()) {
      return false;
    }
    sort(lhs.window_results[i].begin(), lhs.window_results[i].end());
    sort(rhs.window_results[i].begin(), rhs.window_results[i].end());
    for (size_t j = 0; j < lhs.window_results[i].size(); j++) {
      if (lhs.window_results[i][j] != rhs.window_results[i][j]) {
        return false;
      }
    }
  }
  return true;
}

void JoinResult::Print() {
  for (auto i = 0; i < window_results.size(); i++) {
    sort(window_results[i].begin(), window_results[i].end());
    std::cout << "Window #" << i << std::endl;
    for (auto j = 0; j < window_results[i].size(); j++) {
      auto &t = window_results[i][j];
      std::cout << t.k << "," << t.v1 << "," << t.v2 << std::endl;
    }
  }
}

size_t JoinResult::Hash() {
  size_t hash = 0;
  for (auto i = 0; i < window_results.size(); i++) {
    sort(window_results[i].begin(), window_results[i].end());
    std::size_t seed = window_results[i].size();
    if (seed) {
      for (auto &t : window_results[i]) {
        seed ^= t.k + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= t.v1 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= t.v2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      }
      hash ^= seed;
    }
  }
  return hash;
}

void JoinResult::statPrinter() {
  // int n1 = 12;
  // int n2 = 30;
  // std::cout << BAR << std::endl;
  // std::cout << setiosflags(std::ios::right) << std::setw(n1) << "Stats"
  //           << setiosflags(std::ios::right) << std::setw(n2) << "Value"
  //           << resetiosflags(std::ios::right) << std::endl;
  // std::cout << BAR << std::endl;
  // std::string statNames[] = {"AlgoName", "DatasetName", "StreamSize",
  // "WindowSize", "Result", "TimeTaken"}; std::string values[] = {algoName,
  // DATASET_NAME, std::to_string(streamSize), std::to_string(WINDOW_SIZE),
  //                         std::to_string(joinNumber),
  //                         std::to_string(timeTaken) + "ms"};

  // for (int i = 0; i < 6; ++i) {
  //   std::cout << setiosflags(std::ios::right) << std::setw(n1) <<
  //   statNames[i]
  //             << setiosflags(std::ios::right) << std::setw(n2) << values[i]
  //             << std::endl;
  // }
  // std::cout << BAR << std::endl;
}