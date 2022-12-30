#ifndef ALLIANCEDB_INCLUDE_JOIN_HANDSHAKEJOIN_HPP_
#define ALLIANCEDB_INCLUDE_JOIN_HANDSHAKEJOIN_HPP_

#include "Common/Context.hpp"
#include "Join/Join.hpp"
#include "Utils/Queue.hpp"

#include <queue>
#include <vector>

namespace AllianceDB
{
class HandshakeJoin : public JoinAlgo
{
public:
    HandshakeJoin(Context &ctx);
    void Feed(TuplePtr tuple);
    void Wait();

    struct Worker;
    using WorkerPtr = std::shared_ptr<Worker>;
    enum class Msg
    {
        ACK = 1,
        STOP,
        NEXT_WSTR,
        NEXT_WSTS,
        NEXT_TSWR,
        NEXT_TSTR,
        NEXT_TS,
        NEXT_TR,
    };
    struct Worker
    {
        const Param &param;
        WorkerPtr left = nullptr, right = nullptr;
        spsc_queue<TuplePtr> inputr, inputs;
        std::deque<TuplePtr> localr, locals;
        spsc_queue<Msg> msgi, msgo;
        size_t window;
        size_t offs, offr;
        ThreadPtr t;
        ResultPtr res;
        Worker(const Param &param);
        void Run();
        void Start();
        void Wait();
        void SendIn(Msg msg);
        void SendOut(Msg msg);
        Msg RecvIn();
        void Expire(std::deque<TuplePtr> &local, TsType ts);
    };

private:
    Context &ctx;
    std::vector<WorkerPtr> workers;
};

}  // namespace AllianceDB

#endif