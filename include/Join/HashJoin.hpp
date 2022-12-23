#ifndef ALLIANCEDB_INCLUDE_JOIN_HASHJOIN_HPP_
#define ALLIANCEDB_INCLUDE_JOIN_HASHJOIN_HPP_

#include "Common/Param.hpp"
#include "Common/Window.h"
#include "Join/Join.hpp"

namespace AllianceDB {

class HashJoin : public JoinAlgo {
private:
  Param param;
  Window &ws, &wr;

public:
  HashJoin(const Param &param, WindowPtr ws, WindowPtr wr);
  void Run(size_t rbegin, size_t rend, size_t sbegin, size_t send, WindowJoinResult &result) override;
};

} // namespace AllianceDB

#endif