#ifndef ALLIANCEDB_INCLUDE_JOIN_SPLITJOIN_HPP_
#define ALLIANCEDB_INCLUDE_JOIN_SPLITJOIN_HPP_

#include "Common/Context.hpp"
#include "Join/Join.hpp"
#include "Utils/Queue.hpp"
#include "Common/Tuple.hpp"
#include <queue>
#include <vector>
#include <unordered_map>

namespace AllianceDB
{
class SplitJoin : public JoinAlgo
{
public:
    SplitJoin(Context &ctx);
    void Feed(TuplePtr tuple);
    void Wait();

    struct JoinCore
    {
        const Param &param;
        std::vector<TuplePtr> right_region, left_region;
        std::unordered_map<uint64_t , uint64_t> map_idx_right, map_idx_left;
        bool status;
        size_t sub_window;
        ThreadPtr t;
        ResultPtr res;
        int64_t id;
        spsc_queue<TuplePtr> inputs_find, inputs_store;
        JoinCore(const Param &param);
        void Run();
        void Start();
        void Store(TuplePtr tuple);
        void Find(TuplePtr tuple);
        void Wait();
        void Send();
    };
    using JoinCorePtr = std::shared_ptr<JoinCore>;
    struct Distributor
    {
        const Param &param;
        int64_t nums_of_JCs;
        int64_t window;
        std::vector<int> record_r, record_l;
        spsc_queue<TuplePtr> tuples;
        ThreadPtr t;
        std::vector<JoinCorePtr> JCs;
        bool status;
        bool type;
        Distributor(const Param &param);
        void Run();
        void Start();
        void FetchR(TuplePtr tuple, auto idx);
        void FetchL(TuplePtr tuple, auto idx);
        void BroadcastR(TuplePtr tuple);
        void BroadcastL(TuplePtr tuple);
        void Wait();
    };

    using DistributorPtr = std::shared_ptr<Distributor>;

private:
    Context &ctx;
    DistributorPtr distributor;
};
}

#endif