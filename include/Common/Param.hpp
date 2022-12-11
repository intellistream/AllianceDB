#ifndef ALIANCEDB_INCLUDE_COMMON_PARAM_HPP_
#define ALIANCEDB_INCLUDE_COMMON_PARAM_HPP_

#include "Common/Types.hpp"

struct Param{
    std::string bin_dir;
    uint32 window_size;
    uint32 sliding;
    uint32 arr_rate;
    uint32 max_threads;
};

#endif