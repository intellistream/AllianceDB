/*
 * Copyright 2022 IntelliStream
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

#include "Engine/EagerEngine.hpp"
#include "Join/HashJoin.hpp"
#include "Utils/Logger.hpp"

#include <memory>
#include <thread>

using namespace std;
using namespace AllianceDB;

EagerEngine::EagerEngine(const Param &param, const StreamPtr sr,
                         const StreamPtr ss)
    : param(param), sr(sr), ss(ss), res(make_shared<JoinResult>()),
      wr(make_shared<Window>(sr->NumTuples())),
      ws(make_shared<Window>(ss->NumTuples())) {
  res->window_results.resize(
      max(sr->NumTuples(), ss->NumTuples()) / param.sliding + 1);
  switch (param.algo) {
  case AlgoType::HashJoin: {
    algo = make_shared<HashJoin>(param, ws, wr);
    break;
  }
  default:
    ERROR("Unsupported algorithm %d", param.algo);
  }
}

void EagerEngine::Run() {
  while (sr->HasNext() || ss->HasNext()) {
    if (sr->HasNext()) {
      wr->at(cntr++) = sr->Next();
      // INFO("EagerEngine: R %d", wr->size());
    }
    if (IsFullR() && IsFullS()) {
      LaunchJoin();
    }
    if (ss->HasNext()) {
      ws->at(cnts++) = ss->Next();
      // INFO("EagerEngine: S %d", ws->size());
    }
    if (IsFullR() && IsFullS()) {
      LaunchJoin();
    }
  }
  for (auto &t : join_threads) {
    t->join();
  }
}

void EagerEngine::Start() { run_thread = thread(&EagerEngine::Run, this); }

void EagerEngine::Wait() { run_thread.join(); }

void EagerEngine::LaunchJoin() {
  size_t rbeg = cntw * param.sliding,
         rend = min(rbeg + param.window_size, sr->NumTuples());
  size_t sbeg = cntw * param.sliding,
         send = min(sbeg + param.window_size, ss->NumTuples());
  INFO("EagerEngine: [%d,%d]%d [%d,%d]%d", rbeg, rend, cntr, sbeg, send, cnts);
  join_threads.push_back(
      make_unique<thread>(&EagerEngine::RunJoin, this, rbeg, rend, sbeg, send,
                          std::ref(res->window_results[cntw++])));
}

void EagerEngine::RunJoin(size_t rbeg, size_t rend, size_t sbeg, size_t send,
                          WindowJoinResult &res) {
  algo->Run(rbeg, rend, sbeg, send, res);
}

ResultPtr EagerEngine::Result() { return res; }

bool EagerEngine::IsFullR() {
  return (cntr >= param.window_size &&
          (cntr - param.window_size) % param.sliding == 0);
}

bool EagerEngine::IsFullS() {
  return (cnts >= param.window_size &&
          (cnts - param.window_size) % param.sliding == 0);
}