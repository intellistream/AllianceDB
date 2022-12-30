#include "Join/HandshakeJoin.hpp"
#include <Utils/Logger.hpp>

#include <cmath>
#include <memory>
#include <thread>

using namespace AllianceDB;
using namespace std;

HandshakeJoin::HandshakeJoin(Context &ctx) : ctx(ctx)
{
    auto num_workers = ctx.param.num_workers;
    for (auto i = 0; i < num_workers; ++i)
    {
        workers.push_back(std::make_shared<Worker>(ctx.param));
    }
    size_t subwindow = round((double)ctx.param.window / num_workers);
    size_t offs = 0, offr = 0;
    for (auto i = 0; i < num_workers; ++i)
    {
        workers[i]->res = ctx.res;
        if (i > 0) workers[i]->left = workers[i - 1];
        if (i < num_workers - 1) workers[i]->right = workers[i + 1];
        workers[i]->window = subwindow;
        if (i == num_workers - 1)
            workers[i]->window = ctx.param.window - subwindow * (num_workers - 1);
        offr             = ctx.param.window - offs - workers[i]->window;
        workers[i]->offs = offs;
        workers[i]->offr = offr;
        offs += workers[i]->window;
    }
}

void HandshakeJoin::Feed(TuplePtr tuple)
{
    if (tuple->st == StreamType::R)
    {
        workers[ctx.param.num_workers - 1]->inputr.push(tuple);
    }
    else
    {
        workers[0]->inputs.push(tuple);
    }
}

void HandshakeJoin::Wait()
{
    for (auto &w : workers)
    {
        w->SendIn(Msg::STOP);
    }
    for (auto &w : workers)
    {
        auto msg = w->RecvIn();
        if (msg != Msg::ACK)
        {
            ERROR("HandshakeJoin::Wait: unexpected message %d", msg);
        }
    }
    for (auto &w : workers)
    {
        w->Wait();
    }
}

HandshakeJoin::Worker::Worker(const Param &param)
    : param(param), inputr(param.num_tuples), inputs(param.num_tuples), msgi(1), msgo(1)
{}

void HandshakeJoin::Worker::Run()
{
    while (true)
    {
        if (inputr.empty() && inputs.empty())
        {
            if (!msgi.empty())
            {
                auto msg = msgi.front();
                msgi.pop();
                if (msg == Msg::STOP)
                {
                    SendOut(Msg::ACK);
                    return;
                }
            }
        }
        while (!inputr.empty())
        {
            auto tuple = inputr.front();
            inputr.pop();
            for (auto &t : locals)
            {
                if (t->key == tuple->key)
                {
                    res->Emit(tuple, t);
                }
            }
            localr.push_back(tuple);
            Expire(localr, tuple->ts);
        }
        while (!inputs.empty())
        {
            auto tuple = inputs.front();
            inputs.pop();
            for (auto &t : localr)
            {
                if (t->key == tuple->key)
                {
                    res->Emit(t, tuple);
                }
            }
            locals.push_back(tuple);
            Expire(locals, tuple->ts);
        }
    }
}

void HandshakeJoin::Worker::Start()
{
    auto func = [this]() { this->Run(); };
    t         = make_shared<thread>(func);
}

void HandshakeJoin::Worker::Wait() { t->join(); }

void HandshakeJoin::Worker::SendIn(Msg msg) { msgi.push(msg); }

void HandshakeJoin::Worker::SendOut(Msg msg) { msgo.push(msg); }

HandshakeJoin::Msg HandshakeJoin::Worker::RecvIn()
{
    while (msgo.empty())
    {
    }
    auto msg = msgo.front();
    msgo.pop();
    return msg;
}

void HandshakeJoin::Worker::Expire(deque<TuplePtr> &local, TsType ts)
{
    auto wid   = (ts - param.window) / param.sliding + 1;
    auto start = wid * param.sliding;
    if (!local.empty())
    {
        auto tuple = local.front();
        auto ts    = tuple->ts;
        while (ts < start)
        {
            local.pop_back();
            if (!local.empty())
            {
                tuple = local.front();
                ts    = tuple->ts;
            }
            else
            {
                ts = start;
            }
        }
    }
}