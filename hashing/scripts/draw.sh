#!/bin/bash

#APP
#python latency_figure_app.py
#python throughput_figure_app.py
#
#for id in {38..41}; do
#  python progressive_figure.py -i $id
#done
#
#for id in {38..41}; do
#  python breakdown.py -i $id
#done
#
#for id in {38..41}; do
#  python disorderCDF.py -i $id
#done

### KIM 0 ~ 37
for idx in {8..8} ; do #
  python latency_figure$idx.py
  python throughput_figure$idx.py
done

for id in {32..35}; do
  python progressive_figure.py -i $id
done

for id in {32..35}; do
  python breakdown.py -i $id
done

#for id in {0..4}; do
#  python disorderCDF.py -i $id
#done