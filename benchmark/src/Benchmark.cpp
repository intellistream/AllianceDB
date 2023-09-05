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

using namespace std;
using namespace AllianceDB;

Param &GetParam(char *const *argv, Param &param);

void MetricsReport(const Param &param, const Context &ctx);

// Arguments.
void VerifyResults(const Param &param, Context &ctx, Context &ctx_v);
DEFINE_uint32(verify, 1, "Verify results");
DEFINE_uint32(algo, 1, "Join algo");//"LWJ", "HandshakeJoin", "SplitJoin", "IBWJ", "HashJoin", "SplitJoinOrigin"
DEFINE_uint32(window_length, 500, "Window size");
DEFINE_uint32(sliding_size, 50, "Sliding length");
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
  param = GetParam(argv, param);

  StreamPtr R = make_shared<Stream>(param, StreamType::R);
  StreamPtr S = make_shared<Stream>(param, StreamType::S);

  R->Load();
  S->Load();

  param.num_tuples = min(R->Tuples().size(), S->Tuples().size());
  param.num_windows = (param.num_tuples - param.window_length) / param.sliding_size
      + 1;//the total number of windows depends on the sliding_size.

  // Check if there is no remainder
  bool hasNoRemainder = (param.num_tuples - param.window_length) % param.sliding_size == 0;

  // Print the result
  INFO("No remainder: %s", hasNoRemainder ? "true" : "false");

  Context ctx(param, R, S);
  Context ctx_v(param, R, S);

  param.Print();

  switch (param.algo) {
    case AlgoType::HandshakeJoin:
    case AlgoType::SplitJoin:
    case AlgoType::SplitJoinOrigin: {
      auto engine = make_unique<EagerEngine>(param, ctx);
      engine->Run(ctx);
      break;
    }
    default: {
      FATAL("algo not supported")
    }
  }

  MetricsReport(param, ctx);
  VerifyResults(param, ctx, ctx_v);

  return 0;
}
void VerifyResults(const Param &param, Context &ctx, Context &ctx_v) {
  if (param.verify) {
    auto engine = make_unique<VerifyEngine>(param);
    engine->Run(ctx_v);
    auto rt = ctx_v.joinResults->Compare(ctx.joinResults);
    INFO("Results verified to be accurate: %s", (rt == 0) ? "true" : "false")
  }
}

void MetricsReport(const Param &param, const Context &ctx) {
  auto duration = chrono::duration_cast<chrono::nanoseconds>(ctx.endTime - ctx.startTime);
  cout << "time_ms: " << duration.count() / 1e6 << endl;
  cout << "tps: " << param.num_tuples * 2 * 1e9 / duration.count() << endl;
}

Param &GetParam(char *const *argv, Param &param) {
  param.verify = FLAGS_verify;
  param.algo = static_cast<AlgoType>(FLAGS_algo);
  param.window_length = FLAGS_window_length;
  param.sliding_size = FLAGS_sliding_size;
  param.lazy = FLAGS_lazy;
  param.rate = FLAGS_rate;
  param.num_threads = FLAGS_num_threads;
  param.r = FLAGS_r;
  param.s = FLAGS_s;
  param.bin_dir = filesystem::weakly_canonical(filesystem::path(argv[0])).parent_path();
  return param;
}
