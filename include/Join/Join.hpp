#ifndef ALLIANCEDB_INCLUDE_JOIN_JOIN_HPP_
#define ALLIANCEDB_INCLUDE_JOIN_JOIN_HPP_

#include "Common/Result.hpp"
#include "Common/Window.h"

namespace AllianceDB
{
class JoinAlgo
{
public:
    virtual void Feed(TuplePtr tuple) = 0;
};

using JoinPtr = std::shared_ptr<JoinAlgo>;

}  // namespace AllianceDB

#endif