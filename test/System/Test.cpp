// Copyright (C) 2021 by the INTELLI team (https://github.com/intellistream)

#include "Common/Types.hpp"
#include "Common/Param.hpp"
#include "Common/Stream.hpp"
#include "Engine/VerifyEngine.hpp"

#include <gtest/gtest.h>

#include <filesystem>

using namespace AllianceDB;
using namespace std;

TEST(SystemTest, VerifyTest) {
    Param param;
    param.algo = AlgoType::Verify;
    param.window_size = 500;
    param.sliding = 200;
    param.arr_rate = 0;
    StreamPtr R = make_shared<Stream>(param, StreamType::R);
    StreamPtr S = make_shared<Stream>(param, StreamType::S);
    R->Load();
    S->Load();
    VerifyEnginePtr engine = make_unique<VerifyEngine>(R, S, param);
    engine->Run();
    EXPECT_EQ(engine->Result()->Hash(), 0xa22db4f952346a4a);
}
