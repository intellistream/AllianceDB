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

/**
 * @brief This is the main entry point of the entire program.
 * We use this as the entry point for benchmarking.
 */
#include "Common/Param.hpp"
#include "Common/Stream.hpp"
#include "Common/Types.hpp"
#include "Engine/VerifyEngine.hpp"
#include "Utils/Flags.hpp"
#include "Utils/Logger.hpp"

#include <gflags/gflags.h>

#include <filesystem>
#include <memory>

using namespace std;
using namespace AllianceDB;

// Arguments.
DEFINE_uint32(algo, 0, "Join algo");
DEFINE_uint32(window_size, 500, "Window size");
DEFINE_uint32(sliding, 200, "Sliding length");
DEFINE_uint32(arr_rate, 0, "Arrival rate (tuples/sec)");
DEFINE_string(r, "Test1-R.txt", "File path of R stream");
DEFINE_string(s, "Test1-S.txt", "File path of S stream");
DEFINE_uint32(max_threads, 2, "Max threads number");

int main(int argc, char **argv) {
  Param param;
  param.bin_dir =
      filesystem::weakly_canonical(filesystem::path(argv[0])).parent_path();
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  param.algo = static_cast<AlgoType>(FLAGS_algo);
  param.window_size = FLAGS_window_size;
  param.sliding = FLAGS_sliding;
  param.arr_rate = FLAGS_arr_rate;
  param.max_threads = FLAGS_max_threads;
  param.r = FLAGS_r;
  param.s = FLAGS_s;

  StreamPtr R = make_shared<Stream>(param, StreamType::R);
  StreamPtr S = make_shared<Stream>(param, StreamType::S);

  R->Load();
  S->Load();

  switch (param.algo) {
  case AlgoType::Verify: {
    auto engine = make_unique<VerifyEngine>(param, R, S);
    engine->Run();
    std::cout << std::hex << engine->Result()->Hash() << std::dec << std::endl;
    engine->Result()->Print();
    break;
  }
  case AlgoType::HashJoin: {
    auto engine = make_unique<EagerEngine>(param, R, S);
    engine->Run();
    std::cout << std::hex << engine->Result()->Hash() << std::dec << std::endl;
    engine->Result()->Print();
    break;
  }
  }
  return 0;
}