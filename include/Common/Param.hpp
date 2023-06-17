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

#ifndef ALIANCEDB_INCLUDE_COMMON_PARAM_HPP_
#define ALIANCEDB_INCLUDE_COMMON_PARAM_HPP_

#include "Common/Types.hpp"

#include <iostream>

namespace AllianceDB {
struct Param {
  AlgoType algo = AlgoType::Verify;
  std::string bin_dir;
  uint32 window_length;
  uint32 sliding_size;
  uint32 rate;
  uint32 num_threads;
  uint32 lazy;
  uint32 num_tuples = 0;
  uint32 num_windows = 0;
  std::string r = "Test1-R.txt", s = "Test1-S.txt";
  void Print() {
    std::cout << "algo: " << algo_names[static_cast<uint32>(algo)] << std::endl;
    std::cout << "num_tuples: " << num_tuples << std::endl;
    std::cout << "window: " << window_length << std::endl;
    std::cout << "sliding_size: " << sliding_size << std::endl;
    std::cout << "rate: " << rate << std::endl;
    std::cout << "num_threads: " << num_threads << std::endl;
    std::cout << "r: " << r << std::endl;
    std::cout << "s: " << s << std::endl;
  }
};

}  // namespace AllianceDB

#endif