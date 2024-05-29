//
// Created by 86183 on 2023/1/10.
//
#include <iostream>

namespace MSWJ {
// Search granularity
int g{10000};

// Adaptive interval
int L{500000};

// Maximum delay
int maxDelay = INT16_MAX;

// User expected recall rate
double userRecall{0.4};

// User defined time P
int P{600000};

// Basic window size
int b{1000};

// Confidence value for estimating the credibility of R, range (0,1)
double confidenceValue{0.5};

// Number of streams
int streamCount{2};

}
