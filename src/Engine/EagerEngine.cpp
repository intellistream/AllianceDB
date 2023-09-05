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

#include "Engine/EagerEngine.hpp"
#include "Join/HandshakeJoin.hpp"
#include "Join/HashJoin.hpp"
#include "Join/SplitJoin.hpp"
#include "Join/SplitJoinOrigin.hpp"
#include "Utils/Logger.hpp"
#include <memory>
using namespace std;
using namespace AllianceDB;

EagerEngine::EagerEngine(const Param &param, Context &ctx) : param(param) {
//  if (param.algo == AlgoType::SplitJoinOrigin) {
//    Joiners.push_back(Joiner());
//    Joiners[0]->Start(ctx);
//  } else {
//    for (int i = 0; i < param.num_windows; ++i) {
//      auto joiner = Joiner();
//      joiner->Start(ctx);
//      Joiners.push_back(joiner);
//    }
//  }
}

JoinerPtr EagerEngine::Joiner() {
  switch (param.algo) {
    case AlgoType::HandshakeJoin: {
      return make_shared<HandshakeJoin>(param, Joiners.size());
    }
    case AlgoType::SplitJoin: {
      return make_shared<SplitJoin>(param, Joiners.size());
    }
    case AlgoType::SplitJoinOrigin: {
      return make_shared<SplitJoinOrigin>(param, Joiners.size());
    }
    default: {
      FATAL("Unsupported algorithm %d", param.algo);
    }
  }
}

void EagerEngine::Run(Context &ctx) {
  auto sr = ctx.streamR, ss = ctx.streamS;
  while (sr->HasNext() && ss->HasNext()) {//read the next tuple from either stream R or stream S.
    auto nextS = ss->Next(), nextR = sr->Next();
    if (param.algo == AlgoType::SplitJoinOrigin) {//there is only one engine in the traditional approach.
      if (nextR->ts == 0) {
        Joiners.push_back(Joiner());
        Joiners[0]->Start(ctx);
      }
      Joiners[0]->Feed(nextR);
      Joiners[0]->Feed(nextS);
    } else {
      if (nextR->ts % param.sliding_size == 0
          && Joiners.size() < param.num_windows) {//for each window, we create a separate engine.
        Joiners.push_back(Joiner());
        Joiners.back()->Start(ctx);
        DEBUG("algo[%d/%d] started", Joiners.size() - 1, Joiners.size());
      }
      int idx;
      if (nextR->ts < param.window_length) {
        idx = 0;
      } else {
        idx = (nextR->ts - param.window_length) / param.sliding_size + 1;
      }
      for (; idx < Joiners.size(); idx++) {
        Joiners[idx]->Feed(nextR);
        Joiners[idx]->Feed(nextS);
      }
    }
  }
  for (int i = 0; i < Joiners.size(); ++i) {
    Joiners[i]->Wait();
    DEBUG("algo[%d/%d] joined", i, Joiners.size());
  }
}