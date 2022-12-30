#ifndef ALIANCEDB_INCLUDE_COMMON_CONTEXT_HPP_
#define ALIANCEDB_INCLUDE_COMMON_CONTEXT_HPP_

#include "Common/Param.hpp"
#include "Common/Result.hpp"
#include "Common/Stream.hpp"

namespace AllianceDB
{
struct Context
{
    const Param &param;
    ResultPtr res;
    StreamPtr sr, ss;
    size_t num_windows;
    Context(const Param &param) : param(param), res(std::make_shared<JoinResult>(param)) {}
};

}  // namespace AllianceDB

#endif