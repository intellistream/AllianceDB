#include "Join/HandshakeJoin.hpp"
#include "Utils/Logger.hpp"

#include <cassert>
#include <cmath>
#include <memory>
#include <thread>

using namespace AllianceDB;
using namespace std;

HandshakeJoin::HandshakeJoin(const Param &param, size_t window_id) : param(param)
{
    auto num_workers = param.num_threads;
    for (auto i = 0; i < num_workers; ++i)
    {
        workers.push_back(std::make_shared<Worker>(param));
    }
    dummy            = std::make_shared<Worker>(param);
    dummy->id        = -1;
    size_t subwindow = round((double)param.window_length / num_workers);
    size_t offs = 0, offr = 0;
    for (auto i = 0; i < num_workers; ++i)
    {
        workers[i]->window_id = window_id;
        workers[i]->id        = i;
        if (i > 0)
        {
            workers[i]->left            = workers[i - 1];
            workers[i]->left_send_queue = workers[i - 1]->right_recv_queue;
        }
        if (i < num_workers - 1)
        {
            workers[i]->right            = workers[i + 1];
            workers[i]->right_send_queue = workers[i + 1]->left_recv_queue;
        }
        workers[i]->window = subwindow;
        if (i == num_workers - 1)
            workers[i]->window = param.window_length - subwindow * (num_workers - 1);
        offr             = param.window_length - offs - workers[i]->window;
        workers[i]->offs = offs;
        workers[i]->offr = offr;
        offs += workers[i]->window;
    }
    workers[0]->left                = dummy;
    workers[num_workers - 1]->right = dummy;
}

void HandshakeJoin::Feed(TuplePtr tuple)
{
    if (tuple->st == StreamType::R)
    {
        while (!workers[0]->inputr.push(tuple))
            ;
        workers[0]->Send(workers[0]->left_recv_queue, Msg::NEW_R);
    }
    else
    {
        while (!workers[param.num_threads - 1]->inputs.push(tuple))
            ;
        workers[param.num_threads - 1]->Send(workers[param.num_threads - 1]->right_recv_queue,
                                             Msg::NEW_S);
    }
}

void HandshakeJoin::Start(Context &ctx)
{
    for (auto &w : workers)
    {
        w->Start(ctx);
    }
}

void HandshakeJoin::Wait()
{
    workers[0]->Send(workers[0]->left_recv_queue, Msg::STOP);
    workers[param.num_threads - 1]->Send(workers[param.num_threads - 1]->right_recv_queue,
                                         Msg::STOP);
    for (auto &w : workers)
    {
        w->Wait();
        DEBUG("Worker joined");
    }
}

HandshakeJoin::Worker::Worker(const Param &param)
    : param(param),
      inputr(param.window_length * 2),
      inputs(param.window_length * 2),
      msgi(1),
      msgo(1),
      left_recv_queue(std::make_shared<spsc_queue<Msg>>(param.window_length * 2)),
      right_recv_queue(std::make_shared<spsc_queue<Msg>>(param.window_length * 2))
{}

void HandshakeJoin::Worker::Run(Context &ctx)
{
    // DEBUG("Worker %d started", id);
    Msg msg;
    while (true)
    {
        if (stopr && stops && Empty(left_recv_queue) && Empty(right_recv_queue))
        {
            break;
        }
        if (!Empty(left_recv_queue) &&
            (!Full(right_send_queue) || (Peek(left_recv_queue, msg) && msg == Msg::ACK_S)))
            ProcessLeft(ctx);
        if (!Empty(right_recv_queue) &&
            (!Full(left_send_queue) || (Peek(right_recv_queue, msg) && msg == Msg::ACK_R)))
            ProcessRight(ctx);
        Expire();
    }
    DEBUG("Worker %d from %d finished with %d,%d", id, window_id, sentr, sents);
}

void HandshakeJoin::Worker::Start(Context &ctx)
{
    auto func = [this, &ctx]() { this->Run(ctx); };
    t         = make_shared<thread>(func);
}

void HandshakeJoin::Worker::Wait()
{
    if (t) t->join();
}

HandshakeJoin::Msg HandshakeJoin::Worker::RecvIn()
{
    while (msgo.empty())
    {}
    auto msg = msgo.front();
    msgo.pop();
    return msg;
}

void HandshakeJoin::Worker::Expire()
{
    if (sents != ends)
    {
        if (!Full(left_send_queue) && sents - starts < MAX_OUTSTANDING_ACKS &&
            left->ends - left->starts < (ends - starts + MAX_LOAD_DIFF))
        {
            while (!left->inputs.push(locals[sents]))
                ;
            ++sents;
            Send(left_send_queue, Msg::NEW_S);
        }
    }
    if (sentr != endr)
    {
        if (!Full(right_send_queue) && sentr - startr < MAX_OUTSTANDING_ACKS &&
            right->endr - right->startr < (endr - startr + MAX_LOAD_DIFF))
        {
            while (!right->inputr.push(localr[sentr]))
                ;
            ++sentr;
            Send(right_send_queue, Msg::NEW_R);
        }
    }
}

bool HandshakeJoin::Worker::Empty(const MsgQueue &q) { return q != nullptr && q->empty(); }

bool HandshakeJoin::Worker::Full(const MsgQueue &q)
{
    return q != nullptr && q->write_available() == 0;
}

bool HandshakeJoin::Worker::Peek(MsgQueue &q, Msg &msg)
{
    if (!Empty(q))
    {
        msg = q->front();
        return true;
    }
    return false;
}

void HandshakeJoin::Worker::ProcessLeft(Context &ctx)
{
    // DEBUG("Worker %d process left", id);
    Msg msg;
    Peek(left_recv_queue, msg);
    if (msg == Msg::NEW_R)
    {
        Recv(left_recv_queue, msg);
        auto r = inputr.front();
        inputr.pop();
        for (auto s = starts; s < ends; ++s)
        {
            if (r->key == locals[s]->key)
            {
                ctx.joinResults->Emit(window_id, r, locals[s]);
            }
        }
        localr.push_back(r);
        ++endr;
        Send(left_send_queue, Msg::ACK_R);
        // DEBUG("Worker %d: R[%d][%d,%d,%d], S[%d][%d,%d,%d]", id, localr.size(), startr,
        //     endr, sentr, locals.size(), starts, ends, sents);
    }
    else if (msg == Msg::ACK_S)
    {
        Recv(left_recv_queue, msg);
        if (starts != sents)
        {
            ++starts;
        }
    }
    else if (msg == Msg::STOP)
    {
        Recv(left_recv_queue, msg);
        stopr = true;
        DEBUG("STOP left %d in %d", id, window_id);
        while (sentr != endr)
        {
            while (!right->inputr.push(localr[sentr]))
                ;
            ++sentr;
            Send(right_send_queue, Msg::NEW_R);
        }
        Send(right_send_queue, msg);
    }
    // else if (msg == Msg::FLUSH)
    // {
    //     Recv(left_recv_queue, msg);
    // //TODO: lazy flush results
    // }
}

void HandshakeJoin::Worker::ProcessRight(Context &ctx)
{
    // DEBUG("Worker %d process right", id);
    Msg msg;
    Peek(right_recv_queue, msg);
    if (msg == Msg::NEW_S)
    {
        Recv(right_recv_queue, msg);
        auto s = inputs.front();
        inputs.pop();
        const auto r_first = (id == param.num_threads - 1) ? startr : sentr;
        for (auto r = r_first; r < endr; ++r)
        {
            if (localr[r]->key == s->key)
            {
                ctx.joinResults->Emit(window_id, localr[r], s);
            }
        }
        locals.push_back(s);
        ++ends;
        Send(right_send_queue, Msg::ACK_S);
        // DEBUG("Worker %d: R[%d][%d,%d,%d], S[%d][%d,%d,%d]", id, localr.size(), startr,
        //     endr, sentr, locals.size(), starts, ends, sents);
    }
    else if (msg == Msg::ACK_R)
    {
        Recv(right_recv_queue, msg);
        if (startr != sentr)
        {
            ++startr;
        }
    }
    else if (msg == Msg::STOP)
    {
        Recv(right_recv_queue, msg);
        stops = true;
        // DEBUG("STOP right %d in %d", id, window_id);
        while (sents != ends)
        {
            {
                while (!left->inputs.push(locals[sents]))
                    ;
                ++sents;
                Send(left_send_queue, Msg::NEW_S);
            }
        }
        Send(left_send_queue, msg);
    }
    // else if (msg == Msg::FLUSH) {
    //     Recv(right_recv_queue, msg);
}

void HandshakeJoin::Worker::Send(MsgQueue &q, Msg msg)
{
    // DEBUG("Worker %d(%d) send %s, R[%d][%d,%d,%d], S[%d][%d,%d,%d]", id, window_id,
    //     MsgStr[(int)msg], localr.size(), startr, endr, sentr, locals.size(), starts, ends,
    //     sents);
    if (q == nullptr) return;
    while (!q->push(msg))
        ;
}

void HandshakeJoin::Worker::Recv(MsgQueue &q, Msg &msg)
{
    // DEBUG("Worker %d(%d) recv %s, R[%d][%d,%d,%d], S[%d][%d,%d,%d]", id, window_id,
    //     MsgStr[(int)msg], localr.size(), startr, endr, sentr, locals.size(), starts, ends,
    //     sents);
    if (q == nullptr) return;
    msg = q->front();
    q->pop();
}