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

EagerEngine::EagerEngine(Context &ctx)
    : param(ctx.param), sr(ctx.sr), ss(ctx.ss), res(ctx.res), ctx(ctx)
{
    res->window_results.resize(max(sr->NumTuples(), ss->NumTuples()) / param.sliding + 1);
    switch (param.algo)
    {
    case AlgoType::HandshakeJoin:
    {
        algo.push_back(make_shared<HandshakeJoin>(ctx));
        break;
    }
    case AlgoType::SplitJoin:
    {
        algo.push_back(make_shared<SplitJoin>(ctx));
        INFO("make SplitJoiner success");
        break;
    }
    default: ERROR("Unsupported algorithm %d", param.algo);
    }
}

void EagerEngine::Run()
{
    INFO("engine start run");
    // TODO: use rate limiter to control the speed of the input
    while (sr->HasNext() && ss->HasNext())
    {
        // use engine to split window and maintain existing joiner
        auto nextS = ss->Next();
        auto nextR = sr->Next();
        INFO("push tuple from R %d", nextR->ts);
        INFO("push tuple from S %d", nextS->ts);
        if (nextR->ts >= param.window && (nextR->ts - param.window) % param.sliding == 0)
        {
            // To Do: add a lock
            INFO("algo number = %d", algo.size());
            INFO("start erase joiner");
            algo.erase(algo.begin());
            INFO("complete erase joiner");
        }
        if (nextR->ts > 0 && nextR->ts % param.sliding == 0)
        {
            switch (param.algo)
            {
            case AlgoType::HandshakeJoin:
            {
                algo.push_back(make_shared<HandshakeJoin>(ctx));
                break;
            }
            case AlgoType::SplitJoin:
            {
                algo.push_back(make_shared<SplitJoin>(ctx));
                INFO("make new SplitJoiner success");
                break;
            }
            default: ERROR("Unsupported algorithm %d", param.algo);
            }
        }
        int idx = (nextR->ts - param.window) % param.sliding + 1;
        for (; idx < algo.size(); ++idx)
        {
            algo[idx]->Feed(nextR);
            algo[idx]->Feed(nextS);
        }
    }
    for (int i = 0; i < algo.size(); ++i)
    {
        algo[i]->Wait();
    }
}

ResultPtr EagerEngine::Result() { return res; }