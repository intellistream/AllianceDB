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

#ifndef ALLIANCEDB_INCLUDE_ENGINE_LAZYENGINE_HPP_
#define ALLIANCEDB_INCLUDE_ENGINE_LAZYENGINE_HPP_

#include <memory>
#include <thread>

#include "Common/Param.hpp"
#include "Common/Result.hpp"
#include "Common/Stream.hpp"
#include "Common/Window.h"
#include "Join/Join.hpp"

namespace AllianceDB
{
class LazyEngine
{};

typedef std::shared_ptr<class LaziestEngine> EagerEnginePtr;
// Why not template here? Because we may need to be able to choose different
// join algorithms at runtime.
class LaziestEngine
{
private:
    Param param;
    const StreamPtr ss, sr;
    ResultPtr res;
    std::thread run_thread;
    std::vector<std::unique_ptr<std::thread>> join_threads;
    WindowPtr wr, ws;
    JoinPtr algo;
    size_t cntw = 0;
    size_t cntr = 0, cnts = 0;
    void LaunchJoin();
    bool IsFullR();
    bool IsFullS();

public:
    LaziestEngine(const Param &param, const StreamPtr sr, const StreamPtr ss);
    void Run();
    void Start();
    void Wait();
    ResultPtr Result();
    void RunJoin(size_t rbeg, size_t rend, size_t sbeg, size_t send, WindowJoinResult &res);
};

}  // namespace AllianceDB
#endif  // ALLIANCEDB_INCLUDE_ENGINE_LAZYENGINE_HPP_
