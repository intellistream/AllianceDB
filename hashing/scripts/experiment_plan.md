# Experiment Plan

Key principle:

1. Keep all parameters the same as the default value, and vary one parameter at one time.
2. Different figures vary different parameters, but for one figure, only one parameter is varied!


## Fig 1: Varying input arrival rate 

X-axis: 1 10 100 1000 infinite

Required data: test case varying step size.

## Fig 2: Varying input arrival distribution

X-axis: 0 0.2 0.4 0.8 1

Required data: test zipf distribution timestamp.

## Fig 3: Varying key-field distribution (data in stream)

Based on uniform time distribution

X-axis: 0 0.2 0.4 0.8 1

Required data: test varying key distribution.

## Fig 4: Varying window size (data in stream)

Based on uniform time distribution

X-axis: varying window size

Required data: test varying key distribution.


## Fig 5: Varying key-field distribution (data at rest)

Data without timestamp.

X-axis: 0 0.2 0.4 0.8 1

Required data: test varying key distribution.

## Fig 6: Varying window size (data at rest)

Data without timestamp.

X-axis: 0 0.2 0.4 0.8 1

Required data: test varying key distribution.

















Histogram graph

X-axis: unique, zipf(0), zipf(0.2)...

Y-axis: N/ last row of timestamp * different algorithms

Required data: test case 0 ~ 5 of all algorithms.

 