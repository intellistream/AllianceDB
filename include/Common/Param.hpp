#ifndef ALIANCEDB_INCLUDE_COMMON_PARAM_HPP_
#define ALIANCEDB_INCLUDE_COMMON_PARAM_HPP_

#include "Common/Algo.hpp"
#include "Common/Types.hpp"

namespace AllianceDB {

struct Param {
  AlgoType algo;
  std::string bin_dir;
  uint32 window_size;
  uint32 sliding;
  uint32 arr_rate;
  uint32 max_threads;
  std::string r = "Test1-R.txt", s = "Test1-S.txt";
};

} // namespace AllianceDB

#endif