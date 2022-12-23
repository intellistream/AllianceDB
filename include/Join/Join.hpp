#ifndef ALLIANCEDB_INCLUDE_JOIN_JOIN_HPP_
#define ALLIANCEDB_INCLUDE_JOIN_JOIN_HPP_

#include "Common/Result.hpp"
#include "Common/Window.h"

namespace AllianceDB {

class JoinAlgo {
public:
  virtual void Run(size_t rbegin, size_t rend, size_t sbegin, size_t send, WindowJoinResult &result) = 0;
};

using JoinPtr = std::shared_ptr<JoinAlgo>;

} // namespace AllianceDB

#endif