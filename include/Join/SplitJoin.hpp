#ifndef ALLIANCEDB_INCLUDE_JOIN_SPLITJOIN_HPP_
#define ALLIANCEDB_INCLUDE_JOIN_SPLITJOIN_HPP_

#include <queue>
#include <unordered_map>
#include <vector>
#include "Common/Context.hpp"
#include "Common/Tuple.hpp"
#include "Join/Join.hpp"
#include "Utils/Queue.hpp"

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
        std::unordered_map<uint64_t, std::vector<uint64_t>> map_idx_right, map_idx_left;
        bool status;
        size_t sub_window;
        ThreadPtr t;
        ResultPtr res;
        int64_t JC_id, window_id;
        spsc_queue<TuplePtr> inputs_find, inputs_store;
        JoinCore(const Param &param);
        void Run();
        void Start();
        void Store(TuplePtr tuple);
        void Find(TuplePtr tuple);
        void Wait();
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
        Distributor(const Param &param);
        void Run();
        void Start();
        void Wait();
    };

    using DistributorPtr = std::shared_ptr<Distributor>;
    DistributorPtr distributor;

private:
    Context &ctx;
};
}  // namespace AllianceDB

#endif