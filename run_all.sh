#!/bin/bash
exp_dir="/data1/xtra"
L3_cache_size=20971520

# benchmark.sh -e $exp_secction -d $exp_dir -c $L3_cache_size
#APP_BENCH,MICRO_BENCH,SCALE_STUDY,PROFILE_MICRO,PROFILE,PROFILE_MEMORY_CONSUMPTION,PROFILE_PMU_COUNTERS,PROFILE_TOPDOWN
#helpFunction()
#{
#   echo ""
#   echo "Usage: $0 -e exp_section -d exp_dir -c L3_cache_size"
#   echo -e "\t-e the experiment section: APP_BENCH|MICRO_BENCH|SCALE_STUDY|PROFILE_MICRO|PROFILE|PROFILE_MEMORY_CONSUMPTION|PROFILE_PMU_COUNTERS|PROFILE_TOPDOWN"
#   echo -e "\t-d the experiment results directory"
#   echo -e "\t-c the L3 cache size of the current CPU"
#   exit 1 # Exit script after printing help
#}

#while getopts "e:d:c:" opt
#do
#   case "$opt" in
#      e ) exp_secction="$OPTARG" ;;
#      d ) exp_dir="$OPTARG" ;;
#      c ) L3_cache_size="$OPTARG" ;;
#      ? ) helpFunction ;; # Print helpFunction in case parameter is non-existent
#   esac
#done
#
## Print helpFunction in case parameters are empty
#if [ -z "$exp_secction" ] || [ -z "$exp_dir" ] || [ -z "$L3_cache_size" ]
#then
#   echo "Some or all of the parameters are empty";
#   helpFunction
#fi

# Begin script in case all parameters are correct
#echo "$exp_secction"
#echo "$exp_dir"
#echo "$L3_cache_size"

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

exp_secction="PROFILE_MEMORY_CONSUMPTION,PROFILE_PMU_COUNTERS"

# execute experiment
cd ./sorting/scripts || exit
bash benchmark.sh -e $exp_secction -d $exp_dir -c $L3_cache_size
cd - || exit
cd ./hashing/scripts || exit
bash benchmark.sh -e $exp_secction -d $exp_dir -c $L3_cache_size

exp_secction="PROFILE_MEMORY_CONSUMPTION,PROFILE_PMU_COUNTERS"

cd - || exit
cd ./sorting/scripts || exit
bash benchmark.sh -e $exp_secction -d $exp_dir -c $L3_cache_size
cd - || exit
cd ./hashing/scripts || exit
bash benchmark.sh -e $exp_secction -d $exp_dir -c $L3_cache_size

# draw figures
bash draw.sh
python3 jobdone.py