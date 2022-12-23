// Copyright (C) 2021 by the INTELLI team (https://github.com/intellistream)

#include "Common/Types.hpp"
#include "Common/Param.hpp"
#include "Common/Stream.hpp"
#include "Engine/VerifyEngine.hpp"

#include <gtest/gtest.h>

#include <filesystem>

using namespace AllianceDB;
using namespace std;

TEST(SystemTest, Verify) {
    Param param;
    param.algo = AlgoType::Verify;
    param.window_size = 500;
    param.sliding = 200;
    param.arr_rate = 0;
    StreamPtr R = make_shared<Stream>(param, StreamType::R);
    StreamPtr S = make_shared<Stream>(param, StreamType::S);
    R->Load();
    S->Load();
    auto engine = make_unique<VerifyEngine>(param, R, S);
    engine->Run();
    EXPECT_EQ(engine->Result()->Hash(), 0xbfed2395f36e8b78);
}

TEST(SystemTest, EagerHashJoin) {
    Param param;
    param.algo = AlgoType::HashJoin;
    param.window_size = 500;
    param.sliding = 200;
    param.arr_rate = 0;
    StreamPtr R = make_shared<Stream>(param, StreamType::R);
    StreamPtr S = make_shared<Stream>(param, StreamType::S);
    R->Load();
    S->Load();
    auto engine = make_unique<EagerEngine>(param, R, S);
    engine->Run();
    EXPECT_EQ(engine->Result()->Hash(), 0xbfed2395f36e8b78);
}
