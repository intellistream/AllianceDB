#!/bin/bash
## 0 ~ 41


#
for idx in {1..8} ; do #
  python latency_figure$idx.py
  python throughput_figure$idx.py
done

for id in {0..41}; do
  python progressive_figure.py -i $id
done

for id in {0..41}; do
  python breakdown.py -i $id
done