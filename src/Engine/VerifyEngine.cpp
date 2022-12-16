/*
 * Copyright 2022 IntelliStream team (https://github.com/intellistream)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Engine/VerifyEngine.hpp"
#include "Common/Result.hpp"
#include "Engine/EagerEngine.hpp"
#include "Utils/Logger.hpp"

using namespace std;
using namespace AllianceDB;

VerifyEngine::VerifyEngine(const StreamPtr R, const StreamPtr S,
                           const Param &param)
    : R(R), S(S), param(param), result(make_shared<JoinResult>()) {}

void VerifyEngine::Run() {
  INFO("VerifyEngine Starts Running");
  const auto &r_tuples = R->Tuples();
  const auto &s_tuples = S->Tuples();
  auto n = std::max(r_tuples.size(), s_tuples.size());
  for (size_t i = 0; i < n; i += param.sliding) {
    INFO("VerifyEngine: %d/%d", i, n);
    auto r_end = std::min(i + param.window_size, r_tuples.size());
    auto s_end = std::min(i + param.window_size, s_tuples.size());
    std::unordered_map<KeyType, std::vector<TuplePtr>> r_map;
    for (auto j = i; j < r_end; j++) {
      r_map[r_tuples[j]->key].push_back(r_tuples[j]);
    }
    for (auto j = i; j < s_end; j++) {
      auto &s_tuple = s_tuples[j];
      if (r_map.find(s_tuple->key) != r_map.end()) {
        for (auto &r_tuple : r_map[s_tuple->key]) {
          result->Add(i / param.sliding, r_tuple, s_tuple);
        }
      }
    }
  }
}

void VerifyEngine::Start() {
  // Start the Run() thread asynchronously.
  t = std::thread(&VerifyEngine::Run, this);
}

bool VerifyEngine::Join() { t.join(); }

ResultPtr VerifyEngine::Result() { return result; }
