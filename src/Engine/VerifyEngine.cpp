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
#include "Engine/EagerEngine.hpp"
#include "Utils/Logger.hpp"

namespace AllianceDB {

void AllianceDB::VerifyEngine::Start() {
  VerifyThread thread(0);
  thread.Start();
}

VerifyEngine::VerifyEngine(const StreamPtr R, const StreamPtr S,
                           const Param &param)
    : R(R), S(S), param(param) {}

void VerifyEngine::VerifyThread::Process() {
  INFO("VerifyThread %d Starts Running", id());

  TuplePtr curr = NextTuple();
  if (curr != NULL) {
  }
}

TuplePtr VerifyEngine::VerifyThread::NextTuple() {
  return AllianceDB::TuplePtr();
}

std::string VerifyEngine::VerifyThread::id() { return std::to_string(ID); }

VerifyEngine::VerifyThread::VerifyThread(int id) : ID(id) {}

} // AllianceDB