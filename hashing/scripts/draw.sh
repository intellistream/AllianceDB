#!/bin/bash

## APP
echo "Drawing Figure 5..."
python3 latency_figure_app.py
python3 throughput_figure_app.py
###
echo "Drawing Figure 6..."
for id in {38..41}; do
  python3 progressive_figure.py -i $id
done

echo "Drawing Figure 7..."
python3 breakdown_broken.py -i 38
for id in {39..41} ; do
  python3 breakdown.py -i $id
done

echo "Drawing Figure 8 (Please profile with sudo, and then set results in these two files manually)..."
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
echo "Drawing Figure 11.."
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

## Impact of key duplication
echo "Drawing Figure 14..."
python3 throughput_figure6.py
python3 latency_figure6.py
for id in 26 27; do
  python3 progressive_figure.py -i $id
done

### Sorting step study
echo "Drawing Figure 15..."
python3 breakdown_sort.py
python3 latency_sort.py
python3 progressive_sort.py

### Grouping step study
echo "Drawing Figure 16..."
python3 breakdown_group_shj.py
python3 breakdown_group_pmj.py

## Impact of physical partition
echo "Drawing Figure 17..."
python3 breakdown_p_np_study.py

## Radix bit study
echo "Drawing Figure 18..."
python3 breakdown_radix.py
python3 latency_radix.py
python3 progressive_radix.py

## SIMD_STUDY
echo "Drawing Figure 19..."
python3 breakdown_simd.py
echo "Drawing Figure 20..."
python3 profile_simd.py

#### APP Scale.
echo "Drawing Figure 21..."
python3 throughput_scale_lazy.py
python3 throughput_scale_eager.py

echo "Please draw Figure 22 after launch breakdown_NUMA.sh"
python3 throughput_scale_stock.py
python3 throughput_scale_rovio.py
python3 throughput_scale_ysb.py
python3 throughput_scale_debs.py

## Bucket size -- not in use.
#python3 breakdown_bucket.py

## Disorder rate -- not in use.
#for id in {38..41}; do
#  python3 disorderCDF.py -i $id
#done

## HS scheme -- not in use.
#python3 breakdown_hsstudy_jm.py
#python3 breakdown_hsstudy_hs.py
