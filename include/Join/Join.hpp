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

#ifndef ALLIANCEDB_INCLUDE_JOIN_JOIN_HPP_
#define ALLIANCEDB_INCLUDE_JOIN_JOIN_HPP_

#include "Common/Context.hpp"
#include "Common/Result.hpp"
#include "Common/Window.h"

namespace AllianceDB
{
// Note: every joinAlgo maintains multiple JoinCores.
class JoinAlgo
{
public:
    virtual void Feed(TuplePtr tuple) = 0;
    virtual void Wait()               = 0;
    virtual void Start(Context &ctx)  = 0;
};

using JoinerPtr = std::shared_ptr<JoinAlgo>;

}  // namespace AllianceDB

#endif