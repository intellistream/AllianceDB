// Copyright (C) 2021 by the IntelliStream team (https://github.com/intellistream)

/**
 * @mainpage Introduction
 * The AllianceDB offers a wide range of stream join algorithms, supporting both inter and
 * intra window join.
 * @section sec_mj Major Parts
 * Three major parts are composed to achieve ultra-fast stream window join, and they can be found in corresponding named
 * folder under include or src
 *
 * @subsection JoinAlgo
 * Here are specific algorithms to eventually deal with stream join, including hash or radix.
 * All of these algos assume there is no window, in another word,
 * they achieve the intra-window join.
 * Please refer to the @ref INTELLI_JOINALGOS module.
 *
 * @subsection JoinProcessor
 * Here is the middle layer of the whole stream window join, i.e., to bridge the "window" and "join".
 * One JoinProcessor may either eagerly do stream join,
 * or just accumulate the tuples and evoke JoinAlgo  for lazy join. JoinProcessors are managed by upper windowslider.
 * Please refer to the @ref JOINPROCESSOR module.
 *
 * @subsection WindowSlider
 * Here is the top layer on all. Typically, the WindowSliders will:
 * \li Directly receive user input by feedTupleS/R
 * \li Globally manage the sliding window
 * \li Pass tuple to and control its JoinProcessors.
 *
 * Please find them in @ref WINDOWSLIDER module
 * @section sec_other Other Parts
 * Besides the 3 above, Aliance DB has other parts to support its work, they are:
 * @subsection subsec_common Common
 * Common functions, data types for all aliance db component, and they are especially designed for aliance db.
 * Please refer to @ref Common module.
 * @subsection subsec_utils Utils
 * Similar to @ref subsec_common, but they are not specialized for aliance db. On the contrary, they may be shared by other InteliStream
 * programs or third-party programs. Please refer to @ref INTELLI_UTIL module.
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

  StreamPtr streamR = make_shared<Stream>();
  StreamPtr streamS = make_shared<Stream>();
  string pwd = std::filesystem::current_path(); //Get current directory
  string fileRName = pwd + "/datasets/" + DATASET_NAME + "-R.txt";
  string fileSName = pwd + "/datasets/" + DATASET_NAME + "-S.txt";

  streamR->Load(fileSName);
  streamS->Load(fileSName);
}