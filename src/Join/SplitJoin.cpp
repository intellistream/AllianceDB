#include "Join/SplitJoin.hpp"
#include "Utils/Logger.hpp"

#include <cassert>
#include <cmath>
#include <memory>
#include <thread>

using namespace std;
using namespace AllianceDB;

SplitJoin::SplitJoin(const Param &param, size_t wid) : param(param), tuples(2 * param.window)
{
    auto num_workers  = param.num_workers;
    uint32 sub_window = param.window / num_workers;
    for (int i = 0; i < num_workers; ++i)
    {
        JCs.push_back(std::make_shared<JoinCore>(param));
        JCs[i]->sub_window = sub_window;
        JCs[i]->JC_id      = i;
        JCs[i]->window_id  = wid;
    }
}

void SplitJoin::Feed(TuplePtr tuple) { tuples.push(tuple); }

void SplitJoin::Start(Context &ctx)
{
    for (auto &jc : JCs) jc->Start(ctx);
    auto func = [this, &ctx]() { this->Run(ctx); };
    t         = make_shared<thread>(func);
    assert(t);
}

void SplitJoin::Process(Context &ctx)
{
    while (!tuples.empty())
    {
        TuplePtr tuple;
        tuples.pop(tuple);
        int idx = int(tuple->ts) % int(param.num_workers);
        // TODO: use message queue to decouple
        // JCs[idx]->inputs_store.push(tuple);
        JCs[idx]->Store(tuple);
        for (auto i = 0; i < param.num_workers; ++i)
        {
            // JCs[i]->inputs_find.push(tuple);
            JCs[i]->Find(ctx, tuple);
        }
    }
}

void SplitJoin::Run(Context &ctx)
{
    while (true)
    {
        if (!status)
        {
            Process(ctx);
            break;
        }
        Process(ctx);
    }
}

void SplitJoin::Wait()
{
    status = false;
    if (t) t->join();
    for (int i = 0; i < param.num_workers; ++i)
    {
        JCs[i]->Wait();
    }
}

SplitJoin::JoinCore::JoinCore(const Param &param)
    : param(param),
      inputs_store(param.window / param.num_workers),
      inputs_find(param.window),
      status(true)
{}

void SplitJoin::JoinCore::Run(Context &ctx)
{
    while (true)
    {
        if (!this->status)
        {
            break;
        }
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
            Find(ctx, tuple);
        }
    }
    while (!inputs_store.empty())
    {
        TuplePtr tuple;
        inputs_store.pop(tuple);
        Store(tuple);
    }
    while (!inputs_find.empty())
    {
        TuplePtr tuple;
        inputs_find.pop(tuple);
        Find(ctx, tuple);
    }
}

void SplitJoin::JoinCore::Start(Context &ctx)
{
    auto func = [this, &ctx]() { this->Run(ctx); };
    // t         = make_shared<thread>(func);
    ctx.pool.Post(func);
}

void SplitJoin::JoinCore::Store(TuplePtr tuple)
{
    // we currently make every JoinCore will only receive same mount tuples as sub_window, so not
    // necessary to check the size of store region, but we reserve this for future work;
    if (tuple->st == StreamType::R)
    {
        //        if (right_region.size() == sub_window)
        //        {
        //            map_idx_right.erase(right_region[0]->key);
        //            right_region.erase(right_region.begin());
        //        }
        right_region.push_back(tuple);
        map_idx_right[tuple->key].push_back(right_region.size() - 1);
    }
    else
    {
        //        if (left_region.size() == sub_window)
        //        {
        //            map_idx_left.erase(left_region[0]->key);
        //            left_region.erase(left_region.begin());
        //        }
        left_region.push_back(tuple);
        map_idx_left[tuple->key].push_back(left_region.size() - 1);
    }
}

void SplitJoin::JoinCore::Find(Context &ctx, TuplePtr tuple)
{
    if (tuple->st == StreamType::R)
    {
        auto result = map_idx_left.find(tuple->key);
        if (result == map_idx_left.end())
        {
            return;
        }
        for (unsigned int i : result->second)
        {
            ctx.res->Emit(window_id, left_region[i], tuple);
            LOG("window %d has found one matched tuple", window_id);
        }
    }
    else
    {
        auto result = map_idx_right.find(tuple->key);
        if (result == map_idx_right.end())
        {
            return;
        }
        for (unsigned int i : result->second)
        {
            ctx.res->Emit(window_id, tuple, right_region[i]);
            LOG("window %d has found one matched tuple", window_id);
        }
    }
}

void SplitJoin::JoinCore::Wait()
{
    this->status = false;
    if (t) t->join();
}