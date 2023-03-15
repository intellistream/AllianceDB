#ifndef INCLUDE_UTILS_THREADPOOL_HPP_
#define INCLUDE_UTILS_THREADPOOL_HPP_

#include <boost/asio/post.hpp>
#include <boost/asio/thread_pool.hpp>

class ThreadPool
{
public:
    ThreadPool(size_t num_threads) : num_threads(num_threads), pool(num_threads) {}
    template <typename T>
    void Post(T func)
    {
        boost::asio::post(pool, func);
    }
    void Wait() { pool.join(); }

private:
    size_t num_threads;
    boost::asio::thread_pool pool;
};

#endif