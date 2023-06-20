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

VerifyEngine::VerifyEngine(const Param &param) : param(param) {}

void VerifyEngine::Run(Context &ctx) {
  const auto &r_tuples = ctx.streamR->Tuples();
  const auto &s_tuples = ctx.streamS->Tuples();
  auto id_last_tuple = std::max(r_tuples.size(), s_tuples.size());
  for (size_t i = 0; i + param.window_length <= id_last_tuple; i += param.sliding_size) {//walk through each window.
    auto r_end = std::min(i + param.window_length, r_tuples.size());//end of stream R of the current window.
    auto s_end = std::min(i + param.window_length, s_tuples.size());//end of stream S of the current window.
    std::unordered_map<KeyType, std::vector<TuplePtr>> r_map;
    for (auto j = i; j < r_end; j++) {//walk through stream R of the current window.
      r_map[r_tuples[j]->key].push_back(r_tuples[j]);//Construct the hashmap of streamR.
    }
    for (auto j = i; j < s_end; j++) {//walk through stream S of the current window.
      auto &s_tuple = s_tuples[j];
      if (r_map.find(s_tuple->key) != r_map.end()) {//if there is a match.
        for (auto &
              r_tuple : r_map[s_tuple->key]) {//for each r_tuple that has the same key of s_tuple. This is to handle duplicate key case.
          ctx.joinResults->Emit(i / param.sliding_size, r_tuple, s_tuple);
        }
      }
    }
  }
}