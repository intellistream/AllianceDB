//
// Created by shuhao.zhang on 26/4/22.
//

#include "Common/Result.hpp"
#include <algorithm>

using namespace AllianceDB;
using namespace std;

JoinResult::JoinResult(const Param &param)
    : param(param), window_results(param.num_windows + 1), mu(param.num_windows + 1) {}

void JoinResult::Emit(int wid, TuplePtr t1, TuplePtr t2) {
  mu[wid].lock();
  if (window_results.size() <= wid) {
    window_results.resize(wid + 1);
  }
  window_results[wid].push_back(ResultTuple(t1->key, t1->val, t2->val));
  mu[wid].unlock();
}

void JoinResult::EmitAllWindow(TuplePtr t1, TuplePtr t2) {
  auto l = min(t1->ts, t2->ts), r = max(t1->ts, t2->ts);
  unsigned long start = 0;
  unsigned long end = l / param.sliding_size;
  if (l > param.window_length) {
    start = ((l - param.window_length) / param.sliding_size) + 1;
  }
  // find all windows these two tuples belongs to
  for (int i = start; i < param.num_windows && i <= end; ++i) {
    if ((param.sliding_size * i + param.window_length) <= r) {
      continue;
    } else {
      Emit(i, t1, t2);
    }
  }
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

size_t JoinResult::Compare(std::shared_ptr<JoinResult> join_result) {

  for (auto i = 0; i < param.num_windows; i++) {
    if (CompareWindow(window_results[i], join_result->window_results[i]) != 0)return -1;
  }
  return 0;
}
int JoinResult::CompareWindow(vector<ResultTuple> &window_results_verify, vector<ResultTuple> &window_results) {

  if (window_results.size() != window_results_verify.size()) return -1;

  std::sort(window_results_verify.begin(), window_results_verify.end());
  std::sort(window_results.begin(), window_results.end());

  for (auto i = 0; i < window_results.size(); i++) {
    if (window_results[i] != window_results_verify[i])return -1;
  }

  return 0;
}
