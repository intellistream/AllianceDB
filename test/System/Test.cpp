// Copyright (C) 2021 by the INTELLI team (https://github.com/intellistream)

#include <gtest/gtest.h>

#include <filesystem>

#include "Common/Param.hpp"
#include "Common/Stream.hpp"
#include "Common/Types.hpp"
#include "Engine/LazyEngine.hpp"
#include "Engine/VerifyEngine.hpp"

using namespace AllianceDB;
using namespace std;

TEST(SystemTest, Verify)
{
    Param param;
    param.algo    = AlgoType::Verify;
    param.window  = 500;
    param.sliding = 200;
    param.rate    = 0;
    StreamPtr R   = make_shared<Stream>(param, StreamType::R);
    StreamPtr S   = make_shared<Stream>(param, StreamType::S);
    Context ctx(param);
    ctx.sr = R;
    ctx.ss = S;
    R->Load();
    S->Load();
    auto engine = make_unique<VerifyEngine>(ctx);
    engine->Run();
    EXPECT_EQ(engine->Result()->Hash(), 0xbfed2395f36e8b78);
}

// TEST(SystemTest, LazistHashJoin)
// {
//     Param param;
//     param.algo        = AlgoType::HashJoin;
//     param.window = 500;
//     param.sliding     = 200;
//     param.rate    = 0;
//     StreamPtr R       = make_shared<Stream>(param, StreamType::R);
//     StreamPtr S       = make_shared<Stream>(param, StreamType::S);
//     R->Load();
//     S->Load();
//     auto engine = make_unique<LaziestEngine>(param, R, S);
//     engine->Run();
//     EXPECT_EQ(engine->Result()->Hash(), 0xbfed2395f36e8b78);
// }
