#ifndef ALIANCEDB_INCLUDE_COMMON_PARAM_HPP_
#define ALIANCEDB_INCLUDE_COMMON_PARAM_HPP_

#include "Common/Types.hpp"

#include <iostream>

namespace AllianceDB
{
struct Param
{
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
    void Print()
    {
        std::cout << "algo: " << algo_names[static_cast<uint32>(algo)] << std::endl;
        std::cout << "num_tuples: " << num_tuples << std::endl;
        std::cout << "window: " << window << std::endl;
        std::cout << "sliding: " << sliding << std::endl;
        std::cout << "rate: " << rate << std::endl;
        std::cout << "num_workers: " << num_workers << std::endl;
        std::cout << "r: " << r << std::endl;
        std::cout << "s: " << s << std::endl;
    }
};

}  // namespace AllianceDB

#endif