#ifndef ALLIANCEDB_INCLUDE_JOIN_SPLITJOIN_HPP_
#define ALLIANCEDB_INCLUDE_JOIN_SPLITJOIN_HPP_

#include "Common/Context.hpp"
#include "Common/Tuple.hpp"
#include "Join/Join.hpp"
#include "Utils/Queue.hpp"

#include <queue>
#include <unordered_map>
#include <vector>

namespace AllianceDB
{
class SplitJoin : public JoinAlgo
{
public:
    SplitJoin(const Param &param, size_t wid);
    void Feed(TuplePtr tuple);
    void Wait();
    void Run(Context &ctx);
    void Start(Context &ctx);
    void Process(Context &ctx);
    struct JoinCore
    {
        const Param &param;
        std::vector<TuplePtr> right_region, left_region;
        std::unordered_map<KeyType, std::vector<uint32_t>> map_idx_right, map_idx_left;
        bool status;
        size_t sub_window;
        ThreadPtr t;
        int64_t JC_id, window_id;
        spsc_queue<TuplePtr> inputs_find, inputs_store;
        JoinCore(const Param &param);
        void Run(Context &ctx);
        void Start(Context &ctx);
        void Store(TuplePtr tuple);
        void Find(Context &ctx, TuplePtr tuple);
        void Wait();
    };
    using JoinCorePtr = std::shared_ptr<JoinCore>;

private:
    const Param &param;
    int64_t window;
    std::vector<int> record_r, record_l;
    spsc_queue<TuplePtr> tuples;
    ThreadPtr t;
    std::vector<JoinCorePtr> JCs;
    bool status = true;
};
}  // namespace AllianceDB

#endif