//
// Created by Shuhao Zhang on 3/3/20.
//

#ifndef ALLIANCEDB_CLOCK_H
#define ALLIANCEDB_CLOCK_H


#include <chrono>
#include <atomic>


template<typename Clock = std::chrono::high_resolution_clock>
class stopwatch {
    const typename Clock::time_point start_point;
public:
    stopwatch() :
            start_point(Clock::now()) {}

    template<typename Rep = typename Clock::duration::rep, typename Units = typename Clock::duration>
    Rep elapsed_time() const {
        std::atomic_thread_fence(std::memory_order_relaxed);
        auto counted_time = std::chrono::duration_cast<Units>(Clock::now() - start_point).count();
        std::atomic_thread_fence(std::memory_order_relaxed);
        return static_cast<Rep>(counted_time);
    }



};

using precise_stopwatch = stopwatch<>;
using system_stopwatch = stopwatch<std::chrono::system_clock>;
using monotonic_stopwatch = stopwatch<std::chrono::steady_clock>;


#endif //ALLIANCEDB_CLOCK_H
