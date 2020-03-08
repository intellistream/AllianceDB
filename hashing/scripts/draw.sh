#!/bin/bash

APP
python latency_figure_app.py
python throughput_figure_app.py

for id in {38..41}; do
  python progressive_figure.py -i $id
done

for id in {38..41}; do
  python breakdown.py -i $id
done

for id in {38..41}; do
  python disorderCDF.py -i $id
done

### KIM
for idx in {1..8} ; do #
  python latency_figure$idx.py
  python throughput_figure$idx.py
done

## 0 ~ 37
for id in {0..37}; do
  python progressive_figure.py -i $id
done

for id in {0..37}; do
  python breakdown.py -i $id
done

for id in {0..37}; do
  python disorderCDF.py -i $id
done