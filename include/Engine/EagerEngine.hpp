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

#ifndef ALLIANCEDB_SRC_ENGINE_EAGERENGINE_HPP_
#define ALLIANCEDB_SRC_ENGINE_EAGERENGINE_HPP_

#include <memory>
#include <thread>

#include "Common/Context.hpp"
#include "Common/Result.hpp"
#include "Common/Stream.hpp"
#include "Common/Window.h"
#include "Join/Join.hpp"

namespace AllianceDB
{
class EagerEngine
{
public:
    EagerEngine(const Param &param, Context &ctx);
    void Run(Context &ctx);

private:
    const Param &param;
    std::vector<JoinerPtr> Joiners;
    JoinerPtr Joiner();
};

}  // namespace AllianceDB

#endif  // ALLIANCEDB_SRC_ENGINE_EAGERENGINE_HPP_
