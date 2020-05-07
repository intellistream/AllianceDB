#!/bin/bash
### APP
python3 latency_figure_app.py
python3 throughput_figure_app.py
###
for id in {38..41}; do
  python3 progressive_figure.py -i $id
done
#
#python3 breakdown_broken.py -i 38
#for id in {39..41} ; do
#  python3 breakdown.py -i $id
#done

#for id in {38..41}; do
#  python3 disorderCDF.py -i $id
#done

##### APP Scale.
#python3 throughput_scale_lazy.py
#python3 throughput_scale_eager.py
#
#python3 throughput_scale_stock.py
#python3 throughput_scale_rovio.py
#python3 throughput_scale_ysb.py
#python3 throughput_scale_debs.py

### HS scheme
#python3 breakdown_hsstudy_jm.py
#python3 breakdown_hsstudy_hs.py

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

## MICRO BENCH

##### Impact of arrival rate
#python3 throughput_figure1.py
#python3 latency_figure1.py
#for id in 0 4; do
#  python3 progressive_figure.py -i $id
#done
#
##### Impact of relative arrival rate
#python3 throughput_figure2.py
#python3 latency_figure2.py
#for id in 5 9; do
#  python3 progressive_figure.py -i $id
#done
#
##### Impact of arrival distribution
#python3 throughput_figure3.py
#python3 latency_figure3.py
#for id in  10 14 ; do
#  python3 progressive_figure.py -i $id
#done

### Impact of key distribution
#python3 throughput_figure4.py
#python3 latency_figure4.py
#for id in  15 19 ; do
#  python3 progressive_figure.py -i $id
#done
#
### Impact of window size
#python3 throughput_figure5.py
#python3 latency_figure5.py
#for id in  20 24; do
#  python3 progressive_figure.py -i $id
#done

# Impact of data duplication
#python3 throughput_figure6.py
#python3 latency_figure6.py
#for id in 25 28; do
#  python3 progressive_figure.py -i $id
#done

### SIMD_STUDY
#python3 breakdown_simd.py
#python3 profile_simd.py

### Bucket size
#python3 breakdown_bucket.py

### Radix bit study
#python3 breakdown_radix.py
#python3 latency_radix.py
#python3 progressive_radix.py

#### Sorting step study
#python3 breakdown_sort.py
#python3 latency_sort.py
#python3 progressive_sort.py

#### Grouping step study
#python3 breakdown_group_shj.py
#python3 breakdown_group_pmj.py