//
// Created by Shuhao Zhang on 3/3/20.
//https://stackoverflow.com/questions/275004/timer-function-to-provide-time-in-nano-seconds-using-c/11485388#11485388

#ifndef ALLIANCEDB_CLOCK_H
#define ALLIANCEDB_CLOCK_H

#include <chrono>
#include "../utils/machine.h"

namespace x
{
    int test_clock();

    struct clock
    {
        typedef unsigned long long                 rep;
        typedef std::ratio<1, machine_frequencey>       period; //
        typedef std::chrono::duration<rep, period> duration;
        typedef std::chrono::time_point<clock>     time_point;
        static const bool is_steady =              true;
        typedef std::chrono::duration<double, typename x::clock::period> Cycle;
        typedef std::chrono::duration<unsigned long long, std::pico> picoseconds;
        static time_point now() noexcept
        {
            unsigned lo, hi;
            asm volatile("rdtsc" : "=a" (lo), "=d" (hi));
            return time_point(duration(static_cast<rep>(hi) << 32 | lo));
        }

    };

}  // x

#endif //ALLIANCEDB_CLOCK_H
