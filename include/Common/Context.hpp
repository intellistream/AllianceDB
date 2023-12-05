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

#ifndef ALIANCEDB_INCLUDE_COMMON_CONTEXT_HPP_
#define ALIANCEDB_INCLUDE_COMMON_CONTEXT_HPP_

#include "Common/Param.hpp"
#include "Common/Result.hpp"
#include "Common/Stream.hpp"
#include "Utils/Timer.hpp"

namespace AllianceDB
{
struct Context
{
    const Param &param;
    ResultPtr joinResults;
    StreamPtr streamR, streamS;

    std::chrono::high_resolution_clock::time_point startTime;
    std::chrono::high_resolution_clock::time_point endTime;

    Context(const Param &param, StreamPtr R, StreamPtr S)
        : param(param), joinResults(std::make_shared<JoinResult>(param)), streamR(R), streamS(S)
    {}
};

}  // namespace AllianceDB

#endif