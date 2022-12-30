#ifndef ALIANCEDB_INCLUDE_COMMON_PARAM_HPP_
#define ALIANCEDB_INCLUDE_COMMON_PARAM_HPP_

#include "Common/Types.hpp"

namespace AllianceDB
{
struct Param
{
    FILE *log;
    AlgoType algo = AlgoType::Verify;
    std::string bin_dir;
    uint32 window;
    uint32 sliding;
    uint32 rate;
    uint32 num_workers;
    uint32 lazy;
    uint32 num_tuples  = 0;
    uint32 num_windows = 0;
    std::string r = "Test1-R.txt", s = "Test1-S.txt";
};

}  // namespace AllianceDB

#endif