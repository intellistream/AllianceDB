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

#include <memory>
#include <thread>

#include "Join/HandshakeJoin.hpp"
#include "Join/HashJoin.hpp"
#include "Utils/Logger.hpp"

using namespace std;
using namespace AllianceDB;

EagerEngine::EagerEngine(Context &ctx) : param(ctx.param), sr(ctx.sr), ss(ctx.ss), res(ctx.res)
{
    res->window_results.resize(max(sr->NumTuples(), ss->NumTuples()) / param.sliding + 1);
    switch (param.algo)
    {
    case AlgoType::HandshakeJoin:
    {
        algo = make_shared<HandshakeJoin>(ctx);
        break;
    }
    default: ERROR("Unsupported algorithm %d", param.algo);
    }
}

void EagerEngine::Run()
{
    // TODO: use rate limiter to control the speed of the input
    while (sr->HasNext() || ss->HasNext())
    {
        if (sr->HasNext())
        {
            algo->Feed(sr->Next());
        }
        if (ss->HasNext())
        {
            algo->Feed(ss->Next());
        }
    }
    algo->Wait();
}
