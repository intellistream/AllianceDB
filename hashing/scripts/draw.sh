#!/bin/bash

#for idx in 1 2 3 4 5 6 ; do #
#  python latency_figure$idx.py
#  python throughput_figure$idx.py
#done

for id in {32..35}; do
  python progressive_figure.py -i $id
done
