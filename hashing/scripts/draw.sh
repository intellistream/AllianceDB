#!/bin/bash
#
#for idx in {1..8} ; do #
#  python latency_figure$idx.py
#  python throughput_figure$idx.py
#done

for id in {41..41}; do
  python progressive_figure.py -i $id
done
