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
    HandshakeJoin(Context &ctx, size_t);
    void Feed(TuplePtr tuple);
    void Wait();

    static const int MAX_OUTSTANDING_ACKS = 5;
    static const int MAX_LOAD_DIFF        = 5;
    struct Worker;
    enum class Msg
    {
        STOP,
        NEW_R,
        NEW_S,
        ACK_R,
        ACK_S,
        FLUSH,
    };
    static constexpr char *MsgStr[6] = {"STOP", "NEW_R", "NEW_S", "ACK_R", "ACK_S", "FLUSH"};
    using WorkerPtr                  = std::shared_ptr<Worker>;
    using MsgQueue                   = std::shared_ptr<spsc_queue<Msg>>;
    struct Worker
    {
        const Param &param;
        WorkerPtr left = nullptr, right = nullptr;
        MsgQueue left_recv_queue, right_recv_queue;
        MsgQueue left_send_queue = nullptr, right_send_queue = nullptr;
        spsc_queue<TuplePtr> inputr, inputs;
        std::vector<TuplePtr> localr, locals;
        spsc_queue<Msg> msgi, msgo;
        size_t window;
        size_t offs, offr;
        ThreadPtr t;
        ResultPtr res;
        int id, window_id;
        size_t sentr = 0, sents = 0;
        size_t starts = 0, startr = 0;
        size_t ends = 0, endr = 0;
        bool stopr = false, stops = false;
        Worker(const Param &param);
        void Run();
        void Start();
        void Wait();
        Msg RecvIn();
        void Expire();
        bool Empty(const MsgQueue &q);
        bool Full(const MsgQueue &q);
        bool Peek(MsgQueue &q, Msg &msg);
        void ProcessLeft();
        void ProcessRight();
        void Send(MsgQueue &q, Msg msg);
        void Recv(MsgQueue &q, Msg &msg);
    };

private:
    Context &ctx;
    std::vector<WorkerPtr> workers;
    WorkerPtr dummy;
};

}  // namespace AllianceDB

#endif