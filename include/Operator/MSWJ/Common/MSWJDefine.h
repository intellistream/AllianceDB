//
// Created by 86183 on 2023/1/4.
//

#ifndef DISORDERHANDLINGSYSTEM_DEFINE_H
#define DISORDERHANDLINGSYSTEM_DEFINE_H

#include <unordered_map>
#include <queue>
#include <mutex>

//SystemParam of yuanzhen's paper
namespace MSWJ {
// Search granularity
extern int g;

// Adaptive interval
extern int L;

// Maximum delay
extern int maxDelay;

// User expected recall rate
extern double userRecall;

// User defined time P
extern int P;

// Basic window size
extern int b;

// Confidence value for estimating the credibility of R, range (0,1)
extern double confidenceValue;

// Number of streams
extern int streamCount;

}
#endif //DISORDERHANDLINGSYSTEM_DEFINE_H
