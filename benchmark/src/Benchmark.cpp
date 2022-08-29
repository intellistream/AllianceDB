// Copyright (C) 2021 by the IntelliStream team (https://github.com/intellistream)

/**
 * @brief This is the main entry point of the entire program.
 * We use this as the entry point for benchmarking.
 */
#include <filesystem>
#include <Utils/Logger.hpp>
#include <Utils/Flags.hpp>
#include <Common/Types.hpp>
#include <Common/Stream.hpp>
#include <Engine/EagerEngine.hpp>

using namespace std;
using namespace AllianceDB;

//Arguments.
const std::string T = "-T";
const std::string W = "-W";
const std::string S = "-S";
const std::string Engine = "-E";//True:Eager; False:Lazy
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
  auto engine = options.arg_as_or<bool>(Engine, true);

  StreamPtr streamR = make_shared<Stream>();
  StreamPtr streamS = make_shared<Stream>();
  string pwd = std::filesystem::current_path(); //Get current directory
  string fileRName = pwd + "/datasets/" + DATASET_NAME + "-R.txt";
  string fileSName = pwd + "/datasets/" + DATASET_NAME + "-S.txt";

  streamR->Load(fileSName);
  streamS->Load(fileSName);

  if (engine) {
    EagerEnginePtr engine = make_shared<EagerEngine>();
    engine->Run();
  }
}