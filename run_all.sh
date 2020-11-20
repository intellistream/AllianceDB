#!/bin/bash
exp_dir="/data1/xtra"
L3_cache_size=20971520

# read arguments
helpFunction()
{
   echo ""
   echo "Usage: $0 -d exp_dir -c L3_cache_size"
   echo -e "\t-d the experiment results directory"
   echo -e "\t-c the L3 cache size of the current CPU"
   exit 1 # Exit script after printing help
}

while getopts "d:c:" opt
do
   case "$opt" in
      d ) exp_dir="$OPTARG" ;;
      c ) L3_cache_size="$OPTARG" ;;
      ? ) helpFunction ;; # Print helpFunction in case parameter is non-existent
   esac
done

# Print helpFunction in case parameters are empty
if [ -z "$exp_dir" ] || [ -z "$L3_cache_size" ]
then
   echo "Some or all of the parameters are empty";
   helpFunction
fi

# Begin script in case all parameters are correct
echo "$exp_dir"
echo "$L3_cache_size"

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
# set all scripts exp dir
sed -i -e "s/exp_dir = .*/exp_dir = "\"${exp_dir//\//\\/}\""/g" ./hashing/scripts/*.py

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