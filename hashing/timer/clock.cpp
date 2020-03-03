//
// Created by Shuhao Zhang on 3/3/20.
//

#include "clock.h"

#include <iostream>
#include <thread>
int main()
{
    std::cout << "testing scheduler overhead\n";
    for (auto wait_time = 100; wait_time <= 10000; wait_time += 10)
    {



        monotonic_stopwatch stopwatch;
        std::this_thread::sleep_for(std::chrono::microseconds(wait_time));
        auto actual_wait_time = stopwatch.elapsed_time<unsigned int, std::chrono::microseconds>();
        std::cout << "Scheduler overhead is roughly " << actual_wait_time - wait_time << " microseconds"
                  << " for " << wait_time << " microseconds of requested sleep time\n";
    }
}