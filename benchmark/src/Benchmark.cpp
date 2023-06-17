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

#include "Common/Context.hpp"
#include "Common/Stream.hpp"
#include "Common/Types.hpp"
#include "Engine/EagerEngine.hpp"
#include "Engine/LazyEngine.hpp"
#include "Engine/VerifyEngine.hpp"
#include "Utils/Flags.hpp"
#include "Utils/Logger.hpp"

#include <gflags/gflags.h>

#include <chrono>
#include <filesystem>
#include <memory>

using namespace std;
using namespace AllianceDB;

// Arguments.
DEFINE_uint32(algo, 0, "Join algo");
DEFINE_uint32(window_length, 500, "Window size");
DEFINE_uint32(sliding_size, 200, "Sliding length");
DEFINE_uint32(lazy, 0, "Lazy size");
DEFINE_uint32(rate, 0, "Arrival rate (tuples/sec) of R & S");
DEFINE_string(r, "Test1-R.txt", "File path of R stream");
DEFINE_string(s, "Test1-S.txt", "File path of S stream");
DEFINE_uint32(num_threads, 2, "Number of workers");

/**
 * @brief This is the main entry point of the entire program.
 * We use this as the entry point for benchmarking.
 */
int main(int argc, char **argv) {
  Param param;
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  param.algo = static_cast<AlgoType>(FLAGS_algo);
  param.window_length = FLAGS_window_length;
  param.sliding_size = FLAGS_sliding_size;
  param.lazy = FLAGS_lazy;
  param.rate = FLAGS_rate;
  param.num_threads = FLAGS_num_threads;
  param.r = FLAGS_r;
  param.s = FLAGS_s;
  param.bin_dir = filesystem::weakly_canonical(filesystem::path(argv[0])).parent_path();

  StreamPtr R = make_shared<Stream>(param, StreamType::R);
  StreamPtr S = make_shared<Stream>(param, StreamType::S);

  R->Load();
  S->Load();

  param.num_tuples = min(R->Tuples().size(), S->Tuples().size());
  param.num_windows = (param.num_tuples - param.window_length) / param.sliding_size
      + 1;//the total number of windows depends on the sliding_size.

  Context ctx(param, R, S);

  param.Print();

  switch (param.algo) {
    case AlgoType::Verify: {
      auto engine = make_unique<VerifyEngine>(param);
      engine->Run(ctx);
      std::cout << std::hex << ctx.joinResults->Hash() << std::dec << std::endl;
      break;
    }
    case AlgoType::HandshakeJoin:
    case AlgoType::SplitJoin:
    case AlgoType::SplitJoinOrigin: {
      auto engine = make_unique<EagerEngine>(param);
      engine->Run(ctx);
      break;
    }
    default: {
      FATAL("algo not supported")
    }
  }
  // ctx.endTime= std::chrono::high_resolution_clock::now()
  auto duration = chrono::duration_cast<chrono::nanoseconds>(ctx.endTime - ctx.startTime);
  std::cout << "hash: " << std::hex << ctx.joinResults->Hash() << std::dec << std::endl;
  std::cout << "time_ms: " << duration.count() / 1e6 << std::endl;
  std::cout << "tps: " << param.num_tuples * 2 * 1e9 / duration.count() << std::endl;

  return 0;
}
