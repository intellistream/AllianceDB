// Copyright (C) 2021 by the INTELLI team (https://github.com/intellistream)

#include <gtest/gtest.h>

#include <filesystem>

#include "Common/Param.hpp"
#include "Common/Stream.hpp"
#include "Common/Types.hpp"
#include "Engine/EagerEngine.hpp"
#include "Engine/LazyEngine.hpp"
#include "Engine/VerifyEngine.hpp"

using namespace AllianceDB;
using namespace std;

TEST(SystemTest, Verify)
{
    Param param;
    param.algo        = AlgoType::Verify;
    param.window_length      = 500;
    param.sliding_size     = 200;
    param.rate        = 0;
    param.num_windows = 48;
    StreamPtr R       = make_shared<Stream>(param, StreamType::R);
    StreamPtr S       = make_shared<Stream>(param, StreamType::S);
    Context ctx(param, AllianceDB::StreamPtr(), AllianceDB::StreamPtr());
    ctx.streamR = R;
    ctx.streamS = S;
    R->Load();
    S->Load();
    auto engine = make_unique<VerifyEngine>(param);
    engine->Run(ctx);
    EXPECT_EQ(ctx.joinResults->Compare(std::shared_ptr<JoinResult>()), 0xbfed2395f36e8b78);
}

TEST(SystemTest, HandshakeJoin)
{
    Param param;
    param.algo        = AlgoType::HandshakeJoin;
    param.window_length      = 500;
    param.sliding_size     = 200;
    param.rate        = 0;
    param.num_threads = 5;
    param.num_windows = 48;
    StreamPtr R       = make_shared<Stream>(param, StreamType::R);
    StreamPtr S       = make_shared<Stream>(param, StreamType::S);
    Context ctx(param, AllianceDB::StreamPtr(), AllianceDB::StreamPtr());
    ctx.streamR = R;
    ctx.streamS = S;
    R->Load();
    S->Load();
    auto engine = make_unique<EagerEngine>(param);
    engine->Run(ctx);
    // engine->Result()->Print();
    EXPECT_EQ(ctx.joinResults->Compare(std::shared_ptr<JoinResult>()), 0xbfed2395f36e8b78);
}

TEST(SystemTest, SplitJoin)
{
    Param param;
    param.algo        = AlgoType::SplitJoin;
    param.window_length      = 500;
    param.sliding_size     = 200;
    param.rate        = 0;
    param.num_threads = 5;
    param.num_windows = 48;
    StreamPtr R       = make_shared<Stream>(param, StreamType::R);
    StreamPtr S       = make_shared<Stream>(param, StreamType::S);
    Context ctx(param, AllianceDB::StreamPtr(), AllianceDB::StreamPtr());
    ctx.streamR = R;
    ctx.streamS = S;
    R->Load();
    S->Load();
    auto engine = make_unique<EagerEngine>(param);
    engine->Run(ctx);
    EXPECT_EQ(ctx.joinResults->Compare(std::shared_ptr<JoinResult>()), 0xbfed2395f36e8b78);
}

TEST(SystemTest, SplitJoinOrigin)
{
    Param param;
    param.algo        = AlgoType::SplitJoinOrigin;
    param.window_length      = 500;
    param.sliding_size     = 200;
    param.num_threads = 5;
    param.num_windows = 48;
    StreamPtr R       = make_shared<Stream>(param, StreamType::R);
    StreamPtr S       = make_shared<Stream>(param, StreamType::S);
    Context ctx(param, AllianceDB::StreamPtr(), AllianceDB::StreamPtr());
    ctx.streamR = R;
    ctx.streamS = S;
    R->Load();
    S->Load();
    auto engine = make_unique<EagerEngine>(param);
    engine->Run(ctx);
    EXPECT_EQ(ctx.joinResults->Compare(std::shared_ptr<JoinResult>()), 0xbfed2395f36e8b78);
}

// TEST(SystemTest, LazistHashJoin)
// {
//     Param param;
//     param.algo        = AlgoType::HashJoin;
//     param.window = 500;
//     param.sliding_size     = 200;
//     param.rate    = 0;
//     StreamPtr R       = make_shared<Stream>(param, StreamType::R);
//     StreamPtr S       = make_shared<Stream>(param, StreamType::S);
//     R->Load();
//     S->Load();
//     auto engine = make_unique<LaziestEngine>(param, R, S);
//     engine->Run();
//     EXPECT_EQ(engine->Result()->Compare(), 0xbfed2395f36e8b78);
// }
