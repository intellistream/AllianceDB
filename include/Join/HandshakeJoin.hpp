#ifndef ALLIANCEDB_INCLUDE_JOIN_HANDSHAKEJOIN_HPP_
#define ALLIANCEDB_INCLUDE_JOIN_HANDSHAKEJOIN_HPP_

#include "Common/Context.hpp"
#include "Join/Join.hpp"

namespace AllianceDB
{
class HandshakeJoin : public JoinAlgo
{
public:
    HandshakeJoin(Context &ctx);
    void Feed(TuplePtr tuple);
};

}  // namespace AllianceDB

#endif