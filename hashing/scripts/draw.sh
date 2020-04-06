#!/bin/bash

### Parameters

for id in {55..58} ; do
  python3 breakdown_radix.py -i $id
done

for id in {75..78} ; do
  python3 breakdown_sort_step.py.old -i $id
done

for id in {91..94} ; do
  python3 breakdown_merge_step.py -i $id
done

### APP
#python3 latency_figure_app.py
#python3 throughput_figure_app.py
#
#for id in {38..41}; do
#  python3 progressive_figure.py -i $id
#done

#for id in {38..41} ; do
#  python3 breakdown.py -i $id
#done

#for id in {38..41}; do
#  python3 disorderCDF.py -i $id
#done


##### APP Scale.


### KIM
#for idx in {1..8} ; do #
#  python3 latency_figure$idx.py
#  python3 throughput_figure$idx.py
#done
#
## 0 ~ 37
#for id in {0..37}; do
#  python3 progressive_figure.py -i $id
#done
#
#for id in {0..37}; do
#  python3 breakdown.py -i $id
#done
#
#for id in {0..37}; do
#  python3 disorderCDF.py -i $id
#done