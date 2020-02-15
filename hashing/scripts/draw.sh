#!/bin/bash

#for idx in 1 2 3 4 5 6 ; do #
#  python latency_figure$idx.py
#  python throughput_figure$idx.py
#done

#for id in {10..15}; do
#  python progressive_figure.py -i $id
#done


for id in {19..24}; do
  python progressive_figure.py -i $id
done
