#!/bin/bash
exp_dir="/data1/xtra"
L3_cache_size=20971520

## Create directories on your machine.
mkdir -p $exp_dir/results/breakdown/partition_buildsort_probemerge_join
mkdir -p $exp_dir/results/breakdown/partition_only
mkdir -p $exp_dir/results/breakdown/partition_buildsort_only
mkdir -p $exp_dir/results/breakdown/partition_buildsort_probemerge_only
mkdir -p $exp_dir/results/breakdown/allIncludes

mkdir -p $exp_dir/results/figure
mkdir -p $exp_dir/results/gaps
mkdir -p $exp_dir/results/latency
mkdir -p $exp_dir/results/records
mkdir -p $exp_dir/results/timestamps

# copy custom pmu events to experiment dir.
# TODO: do we need to cupy a cpu-mappings.txt?
# TODO: copy datasets to the experiment dir
cp pcm* $exp_dir

exp_secction="APP_BENCH,MICRO_BENCH,SCALE_STUDY,PROFILE_MICRO,PROFILE,PROFILE_MEMORY_CONSUMPTION,PROFILE_PMU_COUNTERS"

# execute experiment
cd ./sorting/scripts || exit
bash benchmark.sh -e $exp_secction -d $exp_dir -c $L3_cache_size
cd - || exit
cd ./hashing/scripts || exit
bash benchmark.sh -e $exp_secction -d $exp_dir -c $L3_cache_size

exp_secction="PROFILE_TOPDOWN"

cd - || exit
cd ./sorting/scripts || exit
bash benchmark.sh -e $exp_secction -d $exp_dir -c $L3_cache_size
cd - || exit
cd ./hashing/scripts || exit
bash benchmark.sh -e $exp_secction -d $exp_dir -c $L3_cache_size

# draw figures
bash draw.sh
python3 jobdone.py