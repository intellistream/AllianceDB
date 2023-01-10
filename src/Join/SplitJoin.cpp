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
    auto num_workers = ctx.param.num_workers;
    auto param = ctx.param;
    distributor = std::make_shared<Distributor>(ctx.param);
    uint32 sub_window = round((double)param.window / num_workers), last_window = param.window - (num_workers-1) * sub_window;
    if (last_window == 0)
    {
        num_workers--;
        distributor->nums_of_JCs--;
        distributor->type = true;
        for (auto i = 0; i < num_workers; ++i)
        {
            distributor->JCs.push_back(std::make_shared<JoinCore>(ctx.param));
            distributor->JCs[i]->sub_window = sub_window;
            distributor->JCs[i]->Start();
        }
    } else {
        for (auto i = 0; i < num_workers; ++i)
        {
            distributor->JCs.push_back(std::make_shared<JoinCore>(ctx.param));
            distributor->JCs[i]->sub_window = sub_window;
            if (i == num_workers-1)
            {
                distributor->JCs[i]->sub_window = last_window;
            }
            distributor->JCs[i]->Start();
        }
    }
    distributor->Start();
}

void SplitJoin::Feed(TuplePtr tuple)
{
    distributor->tuples.push(tuple);
}

void SplitJoin::Wait()
{
    distributor->Wait();
}

SplitJoin::Distributor::Distributor(const Param &param)
    :param(param),
      tuples(param.num_tuples),
      nums_of_JCs(param.num_workers),
      window(param.window),
      type(false),
      status(true)
{}

void SplitJoin::Distributor::Start()
{
    auto func = [this](){this->Run();};
    t = make_shared<thread>(func);
}

void SplitJoin::Distributor::Run()
{
    while (true) {
        if (!status)
        {
            break;
        }
        if (!tuples.empty())
        {
            TuplePtr tuple;
            tuples.pop(tuple);
            if (tuple->st == StreamType::R)
            {
                BroadcastL(tuple);
                if (this->type) {
                    auto JC_idx = tuple->ts % nums_of_JCs;
                    JCs[JC_idx]->inputs_store.push(tuple);
                }
                else
                {
                    auto JC_idx = tuple->ts % window;
                    if (JC_idx == window-1)
                    {
                        JC_idx = nums_of_JCs-1;
                    }
                    else
                    {
                        JC_idx %= (nums_of_JCs-1);
                    }
                    JCs[JC_idx]->inputs_store.push(tuple);
                }
            }
            else
            {
                BroadcastR(tuple);
                if (this->type) {
                    auto JC_idx = tuple->ts % nums_of_JCs;
                    JCs[JC_idx]->inputs_store.push(tuple);
                }
                else
                {
                    auto JC_idx = tuple->ts % window;
                    if (JC_idx == window-1)
                    {
                        JC_idx = nums_of_JCs-1;
                    }
                    else
                    {
                        JC_idx %= (nums_of_JCs-1);
                    }
                    JCs[JC_idx]->inputs_store.push(tuple);
                }
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
      inputs_store(param.window),
      inputs_find(param.window)
{}

void SplitJoin::JoinCore::Run()
{
    LOG(param.log, "JoinCore %d started", id);
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
    auto func = [this](){this->Run();};
    t = make_shared<thread>(func);
}

void SplitJoin::JoinCore::Store(TuplePtr tuple)
{
    if (tuple->st == StreamType::R)
    {
        if (right_region.size() == sub_window)
        {
            map_idx_right.erase(right_region[0]->key);
            right_region.erase(right_region.begin());
        }
        right_region.push_back(tuple);
        map_idx_right.emplace(tuple->key, right_region.size());
    }
    else
    {
        if (left_region.size() == sub_window)
        {
            map_idx_left.erase(left_region[0]->key);
            left_region.erase(left_region.begin());
        }
        left_region.push_back(tuple);
        map_idx_left.emplace(tuple->key, right_region.size());
    }
}

void SplitJoin::JoinCore::Find(TuplePtr tuple)
{
    if (tuple->st == StreamType::R)
    {
        auto result = map_idx_right.find(tuple->key);
        if (result == map_idx_right.end())
        {
            return;
        }
        res->Emit(right_region[result->second], tuple);
    } else
    {
        auto result = map_idx_left.find(tuple->key);
        if (result == map_idx_left.end())
        {
            return;
        }
        res->Emit(tuple, left_region[result->second]);
    }
}

void SplitJoin::JoinCore::Wait()
{
    this->status = false;
}