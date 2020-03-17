//
// Created by Shuhao Zhang on 3/3/20.
//https://stackoverflow.com/questions/275004/timer-function-to-provide-time-in-nano-seconds-using-c/11485388#11485388

#include "clock.h"

#include <iostream>
#include <thread>

int x::test_clock() {
    // Define real time units
    typedef std::chrono::duration<unsigned long long, std::pico> picoseconds;
    // or:
    // typedef std::chrono::nanoseconds nanoseconds;
    // Define double-based unit of clock tick
    typedef std::chrono::duration<double, typename x::clock::period> Cycle;
    using std::chrono::duration_cast;
    const int N = 100000000;
    // Do it
    auto t0 = x::clock::now();
    for (int j = 0; j < N; ++j)
            asm volatile("");
    auto t1 = x::clock::now();
    // Get the clock ticks per iteration
    auto ticks_per_iter = Cycle(t1-t0)/N;
    std::cout << ticks_per_iter.count() << " clock ticks per iteration\n";
    // Convert to real time units
    std::cout << duration_cast<picoseconds>(ticks_per_iter).count()
              << "ps per iteration\n";
}
