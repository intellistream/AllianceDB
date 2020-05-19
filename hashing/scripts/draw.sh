#!/bin/bash

## APP
echo "Drawing Figure 4..."
python3 latency_figure_app.py
python3 throughput_figure_app.py
###
echo "Drawing Figure 5..."
for id in {38..41}; do
  python3 progressive_figure.py -i $id
done

#### APP Scale.
echo "Drawing Figure 6..."
python3 throughput_scale_lazy.py
python3 throughput_scale_eager.py

echo "Drawing Figure 7..."
python3 breakdown_broken.py -i 38
for id in {39..41} ; do
  python3 breakdown.py -i $id
done

echo "Drawing Figure 8..."
python3 profile_ysb_partition.py
python3 profile_ysb_probe.py

## MICRO BENCH
#
### Impact of arrival rate
echo "Drawing Figure 9..."
python3 throughput_figure1.py
python3 latency_figure1.py
for id in 0 4; do
  python3 progressive_figure.py -i $id
done

### Impact of relative arrival rate
echo "Drawing Figure 10..."
python3 throughput_figure2.py
python3 latency_figure2.py
for id in 5 9; do
  python3 progressive_figure.py -i $id
done

#### Impact of arrival distribution
echo "Drawing Figure 11..."
python3 throughput_figure3.py
python3 latency_figure3.py
for id in 10 14 ; do
  python3 progressive_figure.py -i $id
done

## Impact of key distribution
echo "Drawing Figure 12..."
python3 throughput_figure4.py
python3 latency_figure4.py
for id in  15 19 ; do
  python3 progressive_figure.py -i $id
done

## Impact of window size
echo "Drawing Figure 13..."
python3 throughput_figure5.py
python3 latency_figure5.py
for id in  20 24; do
  python3 progressive_figure.py -i $id
done

## Impact of data duplication
echo "Drawing Figure 14..."
python3 throughput_figure6.py
python3 latency_figure6.py
for id in 25 28; do
  python3 progressive_figure.py -i $id
done

### SIMD_STUDY
#python3 breakdown_simd.py
#python3 profile_simd.py
#
### Bucket size
#python3 breakdown_bucket.py
#
### Radix bit study
#python3 breakdown_radix.py
#python3 latency_radix.py
#python3 progressive_radix.py
#
#### Sorting step study
#python3 breakdown_sort.py
#python3 latency_sort.py
#python3 progressive_sort.py
#
#### Grouping step study
#python3 breakdown_group_shj.py
#python3 breakdown_group_pmj.py

#for id in {38..41}; do
#  python3 disorderCDF.py -i $id
#done

## HS scheme
#python3 breakdown_hsstudy_jm.py
#python3 breakdown_hsstudy_hs.py
