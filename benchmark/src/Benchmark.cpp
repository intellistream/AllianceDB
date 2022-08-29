// Copyright (C) 2021 by the IntelliStream team (https://github.com/intellistream)

/**
 * @brief This is the main entry point of the entire program.
 * We use this as the entry point for benchmarking.
 */
#include <filesystem>
#include <Utils/Logger.hpp>
#include <Utils/Flags.h>
#include <Utils/UtilityFunctions.hpp>
#include <Common/Types.hpp>
#include <Common/DatasetTool.hpp>
#include <WindowSlider/AbstractLazyWS.h>
#include <Engine/Execute.hpp>

using namespace std;
using namespace AllianceDB;

//Arguments.
const std::string T = "-T";
const std::string W = "-W";
const std::string S = "-S";
//Required Arguments.
const std::vector<std::string> V{};

int main(int argc, char **argv) {
  //Setup Logs
  setupLogging("benchmark.log", LOG_DEBUG);
  auto options = Flags::Flags{argc, argv};

  if (!options.required_arguments(V)) {
    return -1;
  }

  auto threads = options.arg_as_or<int>("-T", 2);
  auto window_length = options.arg_as_or<int>("-W", 500);
  auto slide_length = options.arg_as_or<int>("-S", 200);

  RelationCouple relationCouple = RelationCouple();
  string pwd = std::filesystem::current_path(); //Get current directory
  string fileRName = pwd + "/datasets/" + DATASET_NAME + "-R.txt";
  string fileSName = pwd + "/datasets/" + DATASET_NAME + "-S.txt";
  DatasetTool dataSet;
  dataSet.load3VText(relationCouple.relationR, fileRName);
  dataSet.load3VText(relationCouple.relationS, fileSName);

  Result exeResult = Result();
  Execute<AbstractLazyWS> execute;
  execute.Run(exeResult, relationCouple, threads, window_length, slide_length);
}