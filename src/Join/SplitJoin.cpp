#include "Join/SplitJoin.hpp"
#include "Utils/Logger.hpp"

#include <cassert>
#include <cmath>
#include <memory>
#include <thread>

using namespace std;
using namespace AllianceDB;

SplitJoin::SplitJoin(Context &ctx) : ctx(ctx), distributor(std::make_shared<Distributor>(ctx.param))
{
    auto num_workers  = ctx.param.num_workers;
    auto param        = ctx.param;
    uint32 sub_window = param.window / num_workers;
    for (int i = 0; i < num_workers; ++i)
    {
        distributor->JCs.push_back(std::make_shared<JoinCore>(ctx.param));
        distributor->JCs[i]->sub_window = sub_window;
        distributor->JCs[i]->JC_id      = i;
        distributor->JCs[i]->window_id  = 0;
        distributor->JCs[i]->res        = ctx.res;
    }
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
    for (auto &jc : JCs) jc->Start();
    auto func = [this]() { this->Run(); };
    t         = make_shared<thread>(func);
    assert(t);
}

void SplitJoin::Distributor::Run()
{
    while (true)
    {
        if (!status)
        {
            while (!tuples.empty())
            {
                TuplePtr tuple;
                tuples.pop(tuple);
                int idx = int(tuple->ts) % int(nums_of_JCs);
                JCs[idx]->inputs_store.push(tuple);
                for (auto i = 0; i < nums_of_JCs; ++i)
                {
                    JCs[i]->inputs_find.push(tuple);
                }
            }
            break;
        }
        while (!tuples.empty())
        {
            TuplePtr tuple;
            tuples.pop(tuple);
            int idx = int(tuple->ts) % int(nums_of_JCs);
            JCs[idx]->inputs_store.push(tuple);
            for (auto i = 0; i < nums_of_JCs; ++i)
            {
                JCs[i]->inputs_find.push(tuple);
            }
        }
    }
}

void SplitJoin::Distributor::Wait()
{
    status = false;
    if (t) t->join();
    for (int i = 0; i < nums_of_JCs; ++i)
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

void SplitJoin::JoinCore::Run()
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
            Find(tuple);
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
        Find(tuple);
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
        //        if (right_region.size() == sub_window)
        //        {
        //            map_idx_right.erase(right_region[0]->key);
        //            right_region.erase(right_region.begin());
        //        }
        right_region.push_back(tuple);
        // map_idx_right.emplace(tuple->key, right_region.size() - 1);
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
        // map_idx_left.emplace(tuple->key, left_region.size() - 1);
        map_idx_left[tuple->key].push_back(left_region.size() - 1);
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
        for (int i = 0; i < result->second.size(); ++i)
        {
            LOG(param.log,
                "find one matched tuples, it is in the %lld window, their key = %llu, tuple1 value "
                "= %llu, "
                "tuple2 value = %llu, tuple1 "
                "ts = %zu, tuple2 ts = %zu",
                window_id, tuple->key, tuple->val, left_region[result->second[i]]->val, tuple->ts,
                left_region[result->second[i]]->ts)
            res->Emit(window_id, left_region[result->second[i]], tuple);
        }
        //        while (result != map_idx_left.end() && result->first == tuple->key) {
        //            LOG(param.log,
        //                "find one matched tuples, it is in the %lld window, their key = %llu,
        //                tuple1 value = %llu, " "tuple2 value = %llu, tuple1 " "ts = %zu, tuple2 ts
        //                = %zu", window_id, tuple->key, tuple->val,
        //                left_region[result->second]->val, tuple->ts,
        //                left_region[result->second]->ts)
        //            res->Emit(window_id, left_region[result->second], tuple);
        //            result++;
        //        }
    }
    else
    {
        auto result = map_idx_right.find(tuple->key);
        if (result == map_idx_right.end())
        {
            return;
        }
        for (int i = 0; i < result->second.size(); ++i)
        {
            LOG(param.log,
                "find one matched tuples, it is in the %lld window, their key = %llu, tuple1 value "
                "=  %llu, "
                "tuple2 value =  %llu, tuple1 "
                "ts = %zu, tuple2 ts = %zu",
                window_id, tuple->key, right_region[result->second[i]]->val, tuple->val,
                right_region[result->second[i]]->ts, tuple->ts)
            res->Emit(window_id, tuple, right_region[result->second[i]]);
        }
        //        while (result != map_idx_right.end() && result->first == tuple->key)
        //        {
        //            LOG(param.log,
        //                "find one matched tuples, it is in the %lld window, their key = %llu,
        //                tuple1 value =  %llu, " "tuple2 value =  %llu, tuple1 " "ts = %zu, tuple2
        //                ts = %zu", window_id, tuple->key, right_region[result->second]->val,
        //                tuple->val, right_region[result->second]->ts, tuple->ts)
        //            res->Emit(window_id, tuple, right_region[result->second]);
        //            result++;
        //        }
    }
}

void SplitJoin::JoinCore::Wait()
{
    this->status = false;
    if (t) t->join();
}