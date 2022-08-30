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
#include <filesystem>
#include <Utils/Logger.hpp>
#include <Utils/Flags.hpp>
#include <Common/Types.hpp>
#include <Common/Stream.hpp>
#include <Engine/VerifyEngine.hpp>

using namespace std;
using namespace AllianceDB;

//Arguments.
const std::string T = "-T";
const std::string W = "-W";
const std::string S = "-S";
const std::string Engine = "-E";//0: Verify Engine.
//Required Arguments.
const std::vector<std::string> V{};

int main(int argc, char **argv) {
  auto options = Flags::Flags{argc, argv};

  if (!options.required_arguments(V)) {
    return -1;
  }

  auto threads = options.arg_as_or<int>(T, 2);
  auto window_length = options.arg_as_or<int>(W, 500);
  auto slide_length = options.arg_as_or<int>(S, 200);
  auto engine = options.arg_as_or<int>(Engine, 0);

  StreamPtr streamR = make_shared<Stream>();
  StreamPtr streamS = make_shared<Stream>();
  string pwd = std::filesystem::current_path(); //Get current directory
  string fileRName = pwd + "/datasets/" + DATASET_NAME + "-R.txt";
  string fileSName = pwd + "/datasets/" + DATASET_NAME + "-S.txt";

  streamR->Load(fileSName);
  streamS->Load(fileSName);

  switch (engine) {
    case 0: {
      VerifyEnginePtr engine = make_shared<VerifyEngine>();
      engine->Start(streamR, streamS, threads, window_length, slide_length);
      break;
    }
  }
}