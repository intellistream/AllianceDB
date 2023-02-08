#include "Join/SplitJoin.hpp"
#include "Utils/Logger.hpp"

#include <cassert>
#include <cmath>
#include <memory>
#include <thread>

using namespace std;
using namespace AllianceDB;

SplitJoin::SplitJoin(Context &ctx) : ctx(ctx)
{
    auto num_workers  = ctx.param.num_workers;
    auto param        = ctx.param;
    distributor       = std::make_shared<Distributor>(ctx.param);
    uint32 sub_window = param.window / num_workers;
    for (int i = 0; i < num_workers; ++i)
    {
        distributor->JCs.push_back(std::make_shared<JoinCore>(ctx.param));
        distributor->JCs[i]->sub_window = sub_window;
        distributor->JCs[i]->JC_id      = i;
        distributor->JCs[i]->window_id  = 0;
        distributor->JCs[i]->Start();
    }
    distributor->Start();
}

void SplitJoin::Feed(TuplePtr tuple) { distributor->tuples.push(tuple); }

void SplitJoin::Wait() { distributor->Wait(); }

SplitJoin::Distributor::Distributor(const Param &param)
    : param(param),
      tuples(param.window),
      nums_of_JCs(param.num_workers),
      window(param.window),
      status(true)
{}

void SplitJoin::Distributor::Start()
{
    auto func = [this]() { this->Run(); };
    t         = make_shared<thread>(func);
}

// distributor as a coordinator
void SplitJoin::Distributor::Run()
{
    while (true)
    {
        if (!status)
        {
            break;
        }
        if (!tuples.empty())
        {
            TuplePtr tuple;
            tuples.pop(tuple);
            int idx = int(tuple->ts) % int(nums_of_JCs);
            JCs[idx]->inputs_store.push(tuple);
            if (tuple->st == StreamType::R)
            {
                BroadcastL(tuple);
            }
            else
            {
                BroadcastR(tuple);
            }
        }
    }
}

void SplitJoin::Distributor::Wait()
{
    for (int i = 0; i < nums_of_JCs; ++i)
    {
        JCs[i]->Wait();
    }
}

void SplitJoin::Distributor::BroadcastR(TuplePtr tuple)
{
    for (auto i = 0; i < nums_of_JCs; ++i)
    {
        JCs[i]->inputs_find.push(tuple);
    }
}

void SplitJoin::Distributor::BroadcastL(TuplePtr tuple)
{
    for (auto i = 0; i < nums_of_JCs; ++i)
    {
        JCs[i]->inputs_find.push(tuple);
    }
}

SplitJoin::JoinCore::JoinCore(const Param &param)
    : param(param),
      inputs_store(param.window / param.num_workers),
      inputs_find(param.window / param.num_workers)
{}

void SplitJoin::JoinCore::Run()
{
    while (this->status)
    {
        if (!inputs_store.empty())
        {
            TuplePtr tuple;
            inputs_store.pop(tuple);
            Store(tuple);
        }
        if (!inputs_find.empty())
        {
            TuplePtr tuple;
            inputs_find.pop(tuple);
            Find(tuple);
        }
    }
}

void SplitJoin::JoinCore::Start()
{
    auto func = [this]() { this->Run(); };
    t         = make_shared<thread>(func);
}

void SplitJoin::JoinCore::Store(TuplePtr tuple)
{
    // we currently make every JoinCore will only receive same mount tuples as sub_window, so not
    // necessary to check the size of store region, but we reserve this for future work;
    if (tuple->st == StreamType::R)
    {
        if (right_region.size() == sub_window)
        {
            map_idx_right.erase(right_region[0]->key);
            right_region.erase(right_region.begin());
        }
        right_region.push_back(tuple);
        map_idx_right.emplace(tuple->key, right_region.size() - 1);
    }
    else
    {
        if (left_region.size() == sub_window)
        {
            map_idx_left.erase(left_region[0]->key);
            left_region.erase(left_region.begin());
        }
        left_region.push_back(tuple);
        map_idx_left.emplace(tuple->key, right_region.size() - 1);
    }
}

void SplitJoin::JoinCore::Find(TuplePtr tuple)
{
    if (tuple->st == StreamType::R)
    {
        auto result = map_idx_left.find(tuple->key);
        if (result == map_idx_left.end())
        {
            return;
        }
        INFO(
            "find one matched tuples, it is in the %d window, tuple1 id %d, tuple2 id %d, tuple1 "
            "ts = %d, tuple2 ts = %d",
            window_id, tuple->key, left_region[result->second]->key, tuple->ts,
            left_region[result->second]->ts);
        res->Emit(window_id, left_region[result->second], tuple);
        INFO("store matched tuples");
    }
    else
    {
        auto result = map_idx_right.find(tuple->key);
        if (result == map_idx_right.end())
        {
            return;
        }
        INFO(
            "find one matched tuples, it is in the %d window, tuple1 id %d, tuple2 id %d, tuple1 "
            "ts = %d, tuple2 ts = %d",
            window_id, right_region[result->second]->key, tuple->key,
            right_region[result->second]->ts, tuple->ts);
        res->Emit(window_id, tuple, right_region[result->second]);
        INFO("store matched tuples");
    }
}

void SplitJoin::JoinCore::Wait() { this->status = false; }