//
// Created by Shuhao Zhang on 3/1/20.
//

#ifndef ALLIANCEDB_SKEWNESS_H
#define ALLIANCEDB_SKEWNESS_H


// CPP code to find skewness
// of statistical data.

#include<bits/stdc++.h>

using namespace std;

// Function to calculate
// mean of data.
float mean(float arr[], int n) {
    float sum = 0;
    for (int i = 0; i < n; i++)
        sum = sum + arr[i];
    return sum / n;
}

// Function to calculate standard
// deviation of data.
float standardDeviation(float arr[],
                        int n) {
    float sum = 0;

    // find standard deviation
    // deviation of data.
    for (int i = 0; i < n; i++)
        sum = (arr[i] - mean(arr, n)) *
              (arr[i] - mean(arr, n));

    return sqrt(sum / n);
}

// Function to calculate skewness.
float skewness(float arr[], int n) {
    // Find skewness using above formula
    float sum = 0;
    for (int i = 0; i < n; i++)
        sum = (arr[i] - mean(arr, n)) *
              (arr[i] - mean(arr, n)) *
              (arr[i] - mean(arr, n));
    return sum / (n * standardDeviation(arr, n) *
                  standardDeviation(arr, n) *
                  standardDeviation(arr, n) *
                  standardDeviation(arr, n));
}

// Driver function
int main() {
    float arr[] = {2.5, 3.7, 6.6, 9.1,
                   9.5, 10.7, 11.9, 21.5,
                   22.6, 25.2};

    // calculate size of array.
    int n = sizeof(arr) / sizeof(arr[0]);

    // skewness Function call
    cout << skewness(arr, n);

    return 0;
}

#endif //ALLIANCEDB_SKEWNESS_H
