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
#include "Join/SplitJoin.hpp"
#include "Utils/Logger.hpp"

using namespace std;
using namespace AllianceDB;

EagerEngine::EagerEngine(const Param &param) : param(param) {}

JoinPtr EagerEngine::New()
{
    switch (param.algo)
    {
    case AlgoType::HandshakeJoin:
    {
        return make_shared<HandshakeJoin>(param, windows.size());
    }
    case AlgoType::SplitJoin:
    {
        return make_shared<SplitJoin>(param, windows.size());
    }
    default:
    {
        FATAL("Unsupported algorithm %d", param.algo);
    }
    }
}

void EagerEngine::Run(Context &ctx)
{
    auto sr = ctx.sr, ss = ctx.ss;
    // TODO: use rate limiter to control the speed of the input
    while (sr->HasNext() && ss->HasNext())
    {
        auto nextS = ss->Next(), nextR = sr->Next();
        if (nextR->ts % param.sliding == 0 && windows.size() < param.num_windows)
        {
            windows.push_back(New());
            windows.back()->Start(ctx);
            LOG("algo[%d/%d] started", windows.size() - 1, windows.size());
        }
        int idx;
        if (nextR->ts < param.window)
        {
            idx = 0;
        }
        else
        {
            idx = (nextR->ts - param.window) / param.sliding + 1;
        }
        for (; idx < windows.size(); idx++)
        {
            windows[idx]->Feed(nextR);
            windows[idx]->Feed(nextS);
        }
    }
    for (int i = 0; i < windows.size(); ++i)
    {
        windows[i]->Wait();
        LOG("algo[%d/%d] joined", i, windows.size());
    }
}
