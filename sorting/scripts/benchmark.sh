#!/bin/bash

gp=0
exp_dir="/data1/xtra"
L3_cache_size=20182588

# read arguments
helpFunction()
{
   echo ""
   echo "Usage: $0 -e exp_section -d exp_dir -c L3_cache_size"
   echo -e "\t-e the experiment section you would like to run"
   echo -e "\t-d the experiment results directory"
   echo -e "\t-c the L3 cache size of the current CPU"
   exit 1 # Exit script after printing help
}

while getopts "e:d:c:" opt
do
   case "$opt" in
      e ) exp_secction="$OPTARG" ;;
      d ) exp_dir="$OPTARG" ;;
      c ) L3_cache_size="$OPTARG" ;;
      ? ) helpFunction ;; # Print helpFunction in case parameter is non-existent
   esac
done

## Print helpFunction in case parameters are empty
#if [ -z "$exp_secction" ] || [ -z "$exp_dir" ] || [ -z "$L3_cache_size" ]
#then
#   echo "Some or all of the parameters are empty";
#   helpFunction
#fi

# Begin script in case all parameters are correct
echo "$exp_secction"
echo "$exp_dir"
echo "$L3_cache_size"

APP_BENCH=0
MICRO_BENCH=0
SCALE_STUDY=0
PROFILE_MICRO=0
PROFILE=0
PROFILE_MEMORY_CONSUMPTION=0
PROFILE_PMU_COUNTERS=0
PROFILE_TOPDOWN=0
# parse exp sections need to run
IFS=','
for exp_secions_name in $(echo "$exp_secction");
do
    echo "name = $exp_secions_name"
    case "$exp_secions_name" in
      "APP_BENCH")
        APP_BENCH=1
        ;;
      "MICRO_BENCH"):
        MICRO_BENCH=1
        ;;
      "SCALE_STUDY")
        SCALE_STUDY=1
        ;;
      "PROFILE_MICRO")
        PROFILE_MICRO=1
        ;;
      "PROFILE")
        PROFILE=1
        ;;
      "PROFILE_MEMORY_CONSUMPTION")
        PROFILE_MEMORY_CONSUMPTION=1
        ;;
      "PROFILE_PMU_COUNTERS")
        PROFILE_PMU_COUNTERS=1
        ;;
      "PROFILE_TOPDOWN")
        PROFILE_TOPDOWN=1
        ;;
    esac
done

echo "Total EXPS: ${exp_secction}"


#set -e
## Set L3 Cache according to your machine.
sed -i -e "s/#define L3_CACHE_SIZE [[:alnum:]]*/#define L3_CACHE_SIZE $L3_cache_size/g" ../utils/params.h
# set experiment dir
sed -i -e "s/#define EXP_DIR .*/#define EXP_DIR "\"${exp_dir//\//\\/}\""/g" ../joins/common_functions.h

# NORMAL MODE
sed -i -e "s/#define PERF_COUNTERS/#define NO_PERF_COUNTERS/g" ../utils/perf_counters.h
sed -i -e "s/#define PROFILE_TOPDOWN/#define NO_PROFILE_TOPDOWN/g" ../utils/perf_counters.h
sed -i -e "s/#define PROFILE_MEMORY_CONSUMPTION/#define NO_PROFILE_MEMORY_CONSUMPTION/g" ../utils/perf_counters.h
sed -i -e "s/#define NO_TIMING/#define TIMING/g" ../joins/common_functions.h


# change cpu-mapping path here, e.g. following changes $exp_dir/cpu-mapping.txt to $exp_dir/cpu-mapping.txt
#sed -i -e "s/\/data1\/xtra\/cpu-mapping.txt/\/data1\/xtra\/cpu-mapping.txt/g" ../affinity/cpu_mapping.h

compile=1
function compile() {
  if [ $compile != 0 ]; then
    cd ..
    cmake . | tail -n +90
    cd scripts
    make -C .. clean -s
    make -C .. -j4 -s
  fi
  echo tangxilin | sudo -S setcap CAP_SYS_RAWIO+eip ../sorting
}

function benchmarkRun() {
  #####native execution
  echo "==benchmark:$benchmark -a $algo -E $epsl_r -F $epsl_s -U $univ -Q $bern -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -g $gap -o $exp_dir/results/breakdown/${benchmark}_${algo}_profile_${gp}_${id}.txt -I $id -H $gp== "
  #echo 3 >/proc/sys/vm/drop_caches
  ../sorting -a $algo -E $epsl_r -F $epsl_s -U $univ -Q $bern -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -g $gap -o $exp_dir/results/breakdown/${benchmark}_${algo}_profile_${gp}_${id}.txt -I $id -H $gp > $exp_dir/results/breakdown/${benchmark}_${algo}_profile_${gp}_${id}.txt
  if [[ $? -eq 139 ]]; then echo "oops, sigsegv" exit 1; fi
}

function benchmarkProfileRun() {
  #####native execution
  echo "==benchmark:$benchmark -a $algo -E $epsl_r -F $epsl_s -U $univ -Q $bern -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -g $gap -o $exp_dir/results/breakdown/${benchmark}_${algo}_profile_${gp}_${id}.txt -I $id -H $gp== "
  #echo 3 >/proc/sys/vm/drop_caches
  if [ ! -z "$PERF_CONF" -a "$PERF_CONF"!=" " ]; then
    ../sorting -a $algo -E $epsl_r -F $epsl_s -U $univ -Q $bern -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -g $gap -o $exp_dir/results/breakdown/${benchmark}_${algo}_profile_${gp}_${id}.txt -p $PERF_CONF -I $id -H $gp > $exp_dir/results/breakdown/${benchmark}_${algo}_profile_${gp}_${id}.txt
  else
    ../sorting -a $algo -E $epsl_r -F $epsl_s -U $univ -Q $bern -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -g $gap -o $exp_dir/results/breakdown/${benchmark}_${algo}_profile_${gp}_${id}.txt -I $id -H $gp > $exp_dir/results/breakdown/${benchmark}_${algo}_profile_${gp}_${id}.txt
  fi
  if [[ $? -eq 139 ]]; then echo "oops, sigsegv" exit 1; fi
}


function perfUarchBenchmarkRun() {
  #####native execution
  echo "==benchmark:$benchmark -a $algo -E $epsl_r -F $epsl_s -U $univ -Q $bern -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -g $gap -o $exp_dir/results/breakdown/${benchmark}_${algo}_profile_${gp}_${id}.txt -I $id -H $gp== "
  if [ ! -z "$PERF_OUTPUT" -a "$PERF_OUTPUT"!=" " ]; then
    perf stat -x, -a -e CPU_CLK_UNHALTED.THREAD,IDQ_UOPS_NOT_DELIVERED.CORE,UOPS_ISSUED.ANY,UOPS_RETIRED.RETIRE_SLOTS,INT_MISC.RECOVERY_CYCLES,CYCLE_ACTIVITY.STALLS_MEM_ANY,RESOURCE_STALLS.SB -o $PERF_OUTPUT \
     ../sorting -a $algo -E $epsl_r -F $epsl_s -U $univ -Q $bern -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -g $gap -I $id -H $gp > $exp_dir/results/breakdown/${benchmark}_${algo}_profile_${gp}_${id}.txt
  else
    perf stat -e CPU_CLK_UNHALTED.THREAD,IDQ_UOPS_NOT_DELIVERED.CORE,UOPS_ISSUED.ANY,UOPS_RETIRED.RETIRE_SLOTS,INT_MISC.RECOVERY_CYCLES,CYCLE_ACTIVITY.STALLS_MEM_ANY,RESOURCE_STALLS.SB \
     ../sorting -a $algo -E $epsl_r -F $epsl_s -U $univ -Q $bern -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -g $gap -I $id -H $gp > $exp_dir/results/breakdown/${benchmark}_${algo}_profile_${gp}_${id}.txt
  fi
  if [[ $? -eq 139 ]]; then echo "oops, sigsegv" exit 1; fi
}

function perfUtilBenchmarkRun() {
  #####native execution
  echo "==benchmark:$benchmark -a $algo -E $epsl_r -F $epsl_s -U $univ -Q $bern -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -g $gap -o $exp_dir/results/breakdown/perf_$id.csv -I $id -H $gp== "
  #echo 3 >/proc/sys/vm/drop_caches
  perf stat -I10 -x, -o $exp_dir/results/breakdown/perf_$id.csv -e cache-misses,cycles ../sorting -a $algo -E $epsl_r -F $epsl_s -U $univ -Q $bern -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -g $gap -I $id -H $gp > $exp_dir/results/breakdown/${benchmark}_${algo}_profile_${gp}_${id}.txt
  if [[ $? -eq 139 ]]; then echo "oops, sigsegv" exit 1; fi
}


function Run() {
  #####native execution
  echo "==benchmark:$benchmark -a $algo -E $epsl_r -F $epsl_s -U $univ -Q $bern -n $Threads=="
  #echo 3 >/proc/sys/vm/drop_caches
  ../sorting -a $algo -E $epsl_r -F $epsl_s -U $univ -Q $bern -r $RSIZE -s $SSIZE -n $Threads
}

function KimRun() {
  #####native execution
  echo "==benchmark:$benchmark -a $algo -E $epsl_r -F $epsl_s -U $univ -Q $bern -t $ts -w $WINDOW_SIZE -e $STEP_SIZE -q $STEP_SIZE_S -l $INTERVAL -d $distrbution -z $skew -D $TS_DISTRIBUTION -Z $ZIPF_FACTOR -n $Threads -g $gap -P $DD -W $FIXS -I $id -H $gp > $exp_dir/results/breakdown/${benchmark}_${algo}_profile_${gp}_${id}.txt =="
  #echo 3 >/proc/sys/vm/drop_caches
  ../sorting -a $algo -E $epsl_r -F $epsl_s -U $univ -Q $bern -t $ts -w $WINDOW_SIZE -e $STEP_SIZE -q $STEP_SIZE_S -l $INTERVAL -d $distrbution -z $skew -D $TS_DISTRIBUTION -Z $ZIPF_FACTOR -n $Threads -W $FIXS -g $gap -P $DD -I $id -H $gp > $exp_dir/results/breakdown/${benchmark}_${algo}_profile_${gp}_${id}.txt
}

function KimProfileRun() {
  #####native execution
  echo "==benchmark:$benchmark -a $algo -E $epsl_r -F $epsl_s -U $univ -Q $bern -t $ts -w $WINDOW_SIZE -e $STEP_SIZE -q $STEP_SIZE_S -l $INTERVAL -d $distrbution -z $skew -D $TS_DISTRIBUTION -Z $ZIPF_FACTOR -n $Threads -g $gap -P $DD -W $FIXS -I $id -H $gp =="
  #echo 3 >/proc/sys/vm/drop_caches
  if [ ! -z "$PERF_CONF" -a "$PERF_CONF"!=" " ]; then
    ../sorting -a $algo -E $epsl_r -F $epsl_s -U $univ -Q $bern -t $ts -w $WINDOW_SIZE -e $STEP_SIZE -q $STEP_SIZE_S -l $INTERVAL -d $distrbution -z $skew -D $TS_DISTRIBUTION -Z $ZIPF_FACTOR -n $Threads -W $FIXS -g $gap -P $DD -o $exp_dir/results/breakdown/${benchmark}_${algo}_profile_${gp}_${id}.txt -p $PERF_CONF -I $id -H $gp > $exp_dir/results/breakdown/${benchmark}_${algo}_profile_${gp}_${id}.txt
  else
    ../sorting -a $algo -E $epsl_r -F $epsl_s -U $univ -Q $bern -t $ts -w $WINDOW_SIZE -e $STEP_SIZE -q $STEP_SIZE_S -l $INTERVAL -d $distrbution -z $skew -D $TS_DISTRIBUTION -Z $ZIPF_FACTOR -n $Threads -W $FIXS -g $gap -P $DD -o $exp_dir/results/breakdown/${benchmark}_${algo}_profile_${gp}_${id}.txt -I $id -H $gp > $exp_dir/results/breakdown/${benchmark}_${algo}_profile_${gp}_${id}.txt
  fi
}

function PARTITION_ONLY() {
  sed -i -e "s/#define JOIN/#define NO_JOIN/g" ../joins/common_functions.h
  sed -i -e "s/#define SORT/#define NO_SORT/g" ../joins/common_functions.h
  sed -i -e "s/#define NO_PARTITION/#define PARTITION/g" ../joins/common_functions.h
  sed -i -e "s/#define OVERVIEW/#define NO_OVERVIEW/g" ../joins/common_functions.h
}
function PARTITION_BUILD_SORT() {
  sed -i -e "s/#define JOIN/#define NO_JOIN/g" ../joins/common_functions.h
  sed -i -e "s/#define NO_SORT/#define SORT/g" ../joins/common_functions.h
  sed -i -e "s/#define NO_PARTITION/#define PARTITION/g" ../joins/common_functions.h
  sed -i -e "s/#define OVERVIEW/#define NO_OVERVIEW/g" ../joins/common_functions.h
}
function PARTITION_BUILD_SORT_MERGE_JOIN() {
  sed -i -e "s/#define NO_JOIN/#define JOIN/g" ../joins/common_functions.h
  sed -i -e "s/#define SORT/#define NO_SORT/g" ../joins/common_functions.h
  sed -i -e "s/#define PARTITION/#define NO_PARTITION/g" ../joins/common_functions.h
  sed -i -e "s/#define OVERVIEW/#define NO_OVERVIEW/g" ../joins/common_functions.h
}

function OVERVIEW() {
  sed -i -e "s/#define JOIN/#define NO_JOIN/g" ../joins/common_functions.h
  sed -i -e "s/#define SORT/#define NO_SORT/g" ../joins/common_functions.h
  sed -i -e "s/#define PARTITION/#define NO_PARTITION/g" ../joins/common_functions.h
  sed -i -e "s/#define NO_OVERVIEW/#define OVERVIEW/g" ../joins/common_functions.h
}

# Different execution mode for different experiments

function PCM() {
  sed -i -e "s/#define TIMING/#define NO_TIMING/g" ../joins/common_functions.h #disable time measurement
  sed -i -e "s/#define NO_PERF_COUNTERS/#define PERF_COUNTERS/g" ../utils/perf_counters.h # enable pcm counter
  sed -i -e "s/#define PROFILE_TOPDOWN/#define NO_PROFILE_TOPDOWN/g" ../joins/common_functions.h # disable perf
  sed -i -e "s/#define PROFILE_MEMORY_CONSUMPTION/#define NO_PROFILE_MEMORY_CONSUMPTION/g" ../joins/common_functions.h # enable memory consumption
}

function PERF() {
  sed -i -e "s/#define TIMING/#define NO_TIMING/g" ../joins/common_functions.h #disable time measurement
  sed -i -e "s/#define PERF_COUNTERS/#define NO_PERF_COUNTERS/g" ../utils/perf_counters.h
  sed -i -e "s/#define NO_PROFILE_TOPDOWN/#define PROFILE_TOPDOWN/g" ../joins/common_functions.h
  sed -i -e "s/#define PROFILE_MEMORY_CONSUMPTION/#define NO_PROFILE_MEMORY_CONSUMPTION/g" ../joins/common_functions.h
}

function MEM_MEASURE() {
  sed -i -e "s/#define TIMING/#define NO_TIMING/g" ../joins/common_functions.h
  sed -i -e "s/#define PERF_COUNTERS/#define NO_PERF_COUNTERS/g" ../utils/perf_counters.h
  sed -i -e "s/#define PROFILE_TOPDOWN/#define NO_PROFILE_TOPDOWN/g" ../joins/common_functions.h
  sed -i -e "s/#define NO_PROFILE_MEMORY_CONSUMPTION/#define PROFILE_MEMORY_CONSUMPTION/g" ../joins/common_functions.h
}

function NORMAL() {
  sed -i -e "s/#define NO_TIMING/#define TIMING/g" ../joins/common_functions.h #disable time measurement
  sed -i -e "s/#define PERF_COUNTERS/#define NO_PERF_COUNTERS/g" ../utils/perf_counters.h
  sed -i -e "s/#define PROFILE_TOPDOWN/#define NO_PROFILE_TOPDOWN/g" ../joins/common_functions.h
  sed -i -e "s/#define PROFILE_MEMORY_CONSUMPTION/#define NO_PROFILE_MEMORY_CONSUMPTION/g" ../joins/common_functions.h
}

# function SetStockParameters() { #matches: 15595000. #inputs= 60527 + 77227
#   ts=1 # stream case
#   WINDOW_SIZE=1000
#   RSIZE=60527
#   SSIZE=77227
#   RPATH=$exp_dir/datasets/stock/cj_1000ms_1t.txt
#   SPATH=$exp_dir/datasets/stock/sb_1000ms_1t.txt
#   RKEY=0
#   SKEY=0
#   RTS=1
#   STS=1
#   gap=15595
# }

function SetStockParameters() { #matches: 15598112. #inputs= 60527 + 77227
  ts=1 # stream case
  WINDOW_SIZE=1000
  RSIZE=1013800
  SSIZE=1034443
  RPATH=$exp_dir/datasets/NDP/Sep_85.txt
  SPATH=$exp_dir/datasets/NDP/Sep_86.txt
  RKEY=0
  SKEY=0
  RTS=1
  STS=1
  gap=15595
}

function SetRovioParameters() { #matches: 87856849382 #inputs= 2873604 + 2873604
  ts=1 # stream case
  WINDOW_SIZE=1000
  RSIZE=2873604
  SSIZE=2873604
  RPATH=$exp_dir/datasets/rovio/1000ms_1t.txt
  SPATH=$exp_dir/datasets/rovio/1000ms_1t.txt
  RKEY=0
  SKEY=0
  RTS=3
  STS=3
  # gap=87856849
  gap=20000
}

function SetYSBParameters() { #matches: 10000000. #inputs= 1000 + 10000000
  ts=1 # stream case
  WINDOW_SIZE=1000
  RSIZE=1000
  SSIZE=10000000
  RPATH=$exp_dir/datasets/YSB/campaigns_id.txt
  SPATH=$exp_dir/datasets/YSB/ad_events.txt
  RKEY=0
  SKEY=0
  RTS=0
  STS=1
  gap=10000
}

function SetDEBSParameters() { #matches: 251033140 #inputs= 1000000 + 1000000
  ts=1 # stream case
  WINDOW_SIZE=0
  RSIZE=1000000 #1000000
  SSIZE=1000000 #1000000
  RPATH=$exp_dir/datasets/DEBS/posts_key32_partitioned.csv
  SPATH=$exp_dir/datasets/DEBS/comments_key32_partitioned.csv
  RKEY=0
  SKEY=0
  RTS=0
  STS=0
  gap=251033
}

DEFAULT_WINDOW_SIZE=1000 #(ms) -- 1 seconds
DEFAULT_STEP_SIZE=12800  # |tuples| per ms. -- 128K per seconds. ## this controls the guranalrity of input stream.
function ResetParameters() {
  TS_DISTRIBUTION=0                # uniform time distribution
  ZIPF_FACTOR=0                    # uniform time distribution
  distrbution=0                    # unique
  skew=0                           # uniform key distribution
  INTERVAL=1                       # interval of 1. always..
  STEP_SIZE=$DEFAULT_STEP_SIZE     # arrival rate = 1000 / ms
  WINDOW_SIZE=$DEFAULT_WINDOW_SIZE # MS rel size = window_size / interval * step_size.
  STEP_SIZE_S=128000               # let S has the same arrival rate of R.
  FIXS=1
  ts=1 # stream case
  if [ $thread_test == 1 ]; then
    Threads=8
  fi
  gap=12800
  DD=1
  sed -i -e "s/scalarflag [[:alnum:]]*/scalarflag 0/g" ../joins/common_functions.h
}


function SET_RAND_BUFFER_SIZE() {
  sed -i -e "s/#define RANDOM_BUFFER_SIZE [[:alnum:]]*/#define RANDOM_BUFFER_SIZE $rand_buffer_size/g" ../main.cpp
}

#recompile by default.
compile
# Configurable variables
# Generate a timestamp
timestamp=$(date +%Y%m%d-%H%M)
output=test$timestamp.txt

# sed -i -e "s/#define NO_SORT_SET_PR/#define SORT_SET_PR/g" ../main.cpp
# sed -i -e "s/#define SORT_SET_PR/#define NO_SORT_SET_PR/g" ../main.cpp
# sed -i -e "s/#define NO_SORT_SAMPLE_ON/#define SORT_SAMPLE_ON/g" ../main.cpp
# sed -i -e "s/#define NO_SORT_AVX_RAND/#define SORT_AVX_RAND/g" ../main.cpp


function IF_SORT_SET_PR()
{
  if [ $setpr == 0 ]; then
    sed -i -e "s/#define NO_SORT_SET_PR/#define SORT_SET_PR/g" ../main.cpp
  fi
  if [ $setpr == 1 ]; then
    sed -i -e "s/#define SORT_SET_PR/#define NO_SORT_SET_PR/g" ../main.cpp
  fi
}

function SAMPLE_OFF() {
	sed -i -e "s/#define SORT_SET_PR/#define NO_SORT_SET_PR/g" ../main.cpp
	sed -i -e "s/#define SORT_SAMPLE_ON/#define NO_SORT_SAMPLE_ON/g" ../main.cpp
	sed -i -e "s/#define SORT_AVX_RAND/#define NO_SORT_AVX_RAND/g" ../main.cpp
}



declare -a arr

# arr=( '1 0.1 1 0.1' '0.1 1 0.1 1' '0.3 0.333 0.3 0.333' '0.333 0.3 0.333 0.3' '0.667 0.15 0.667 0.15' '0.15 0.667 0.15 0.667' )
arr=( '0.1 0.1 1 0.1' '0.1 0.1 1 0.1' '0.1 0.1 1 0.1' '0.1 0.1 0.3 0.333' '0.1 0.1 0.3 0.333' '0.1 0.1 0.3 0.333' '0.1 0.1 0.667 0.15' '0.1 0.1 0.667 0.15' '0.1 0.1 0.667 0.15' '0.1 0.1 0.1 1' '0.1 0.1 0.1 1' '0.1 0.1 0.1 1' )


# for setpr in 0 1; do
# IF_SORT_SET_PR
# for rand_pair in "${arr[@]}"; do
# eval real_pair=(${rand_pair})
# epsl_r=${real_pair[0]};
# epsl_s=${real_pair[1]};
# univ=${real_pair[2]};
# bern=${real_pair[3]};
# let "gp++"
# APP_Bench.
# APP_BENCH=0


########## NON-SAMPLE BASELINE

thread_test=1
rand_buffer_size=1000

<<COMMENT
gp=0
epsl_s=0.5
epsl_r=0.5
univ=0.7
bern=0.7

SAMPLE_OFF
if [ $APP_BENCH == 1 ]; then
  NORMAL
  profile_breakdown=1
  compile=1
  compile
  for algo in m-way m-pass; do
    for benchmark in "Stock" "Rovio" "YSB" "DEBS"; do #
      case "$benchmark" in
      "Stock")
        id=38
        ResetParameters
        SetStockParameters
        SET_RAND_BUFFER_SIZE
        compile
        benchmarkRun
        ;;
      "Rovio") #matches:
        id=39
        ResetParameters
        SetRovioParameters
        SET_RAND_BUFFER_SIZE
        compile
        benchmarkRun
        ;;
      "YSB")
        id=40
        ResetParameters
        SetYSBParameters
        SET_RAND_BUFFER_SIZE
        compile
        benchmarkRun
        ;;
      "DEBS")
        id=41
        ResetParameters
        SetDEBSParameters
        SET_RAND_BUFFER_SIZE
        compile
        benchmarkRun
        ;;
      esac
    done
  done
fi
# exit
###### SAMPLE NON-MEM-LIMITED
gp=1
epsl_s=0.5
epsl_r=0.5
univ=0.7
bern=0.7


sed -i -e "s/#define NO_SORT_SAMPLE_ON/#define SORT_SAMPLE_ON/g" ../main.cpp
sed -i -e "s/#define NO_SORT_AVX_RAND/#define SORT_AVX_RAND/g" ../main.cpp
if [ $APP_BENCH == 1 ]; then
  NORMAL
  profile_breakdown=1
  compile=1
  compile
  for algo in m-way m-pass; do
    for benchmark in "Stock" "Rovio" "YSB" "DEBS"; do #
      case "$benchmark" in
      "Stock")
        id=38
        ResetParameters
        SetStockParameters
        SET_RAND_BUFFER_SIZE
        compile
        benchmarkRun
        ;;
      "Rovio") #matches:
        id=39
        ResetParameters
        SetRovioParameters
        SET_RAND_BUFFER_SIZE
        compile
        benchmarkRun
        ;;
      "YSB")
        id=40
        ResetParameters
        SetYSBParameters
        SET_RAND_BUFFER_SIZE
        compile
        benchmarkRun
        ;;
      "DEBS")
        id=41
        ResetParameters
        SetDEBSParameters
        SET_RAND_BUFFER_SIZE
        compile
        benchmarkRun
        ;;
      esac
    done
  done
fi

# exit
############### impact of epsilon

gp=10

epsl_s=0.5
epsl_r=0.5
univ=0.7
bern=0.7


sed -i -e "s/#define NO_SORT_SAMPLE_ON/#define SORT_SAMPLE_ON/g" ../main.cpp
sed -i -e "s/#define NO_SORT_AVX_RAND/#define SORT_AVX_RAND/g" ../main.cpp
for epsl in 0.9 0.8 0.7 0.6 0.5 0.4 0.3 0.2 0.1 0.05 0.01 0.005 0.001; do
  epsl_r=$epsl
  epsl_s=$epsl
  NORMAL
  profile_breakdown=1
  compile=1
  compile
  for algo in m-way m-pass; do
    for benchmark in "Stock" "Rovio" "YSB" "DEBS"; do #
      case "$benchmark" in
      "Stock")
        id=38
        ResetParameters
        SetStockParameters
        SET_RAND_BUFFER_SIZE
        compile
        benchmarkRun
        ;;
      "Rovio") #matches:
        id=39
        ResetParameters
        SetRovioParameters
        SET_RAND_BUFFER_SIZE
        compile
        benchmarkRun
        ;;
      "YSB")
        id=40
        ResetParameters
        SetYSBParameters
        SET_RAND_BUFFER_SIZE
        compile
        benchmarkRun
        ;;
      "DEBS")
        id=41
        ResetParameters
        SetDEBSParameters
        SET_RAND_BUFFER_SIZE
        compile
        benchmarkRun
        ;;
      esac
    done
  done
  let gp++
done


#####                        variance
gp=100

epsl_s=0.5
epsl_r=0.5
univ=0.7
bern=0.7


sed -i -e "s/#define NO_SORT_SAMPLE_ON/#define SORT_SAMPLE_ON/g" ../main.cpp
sed -i -e "s/#define NO_SORT_AVX_RAND/#define SORT_AVX_RAND/g" ../main.cpp
for iii in {1..20}; do
  NORMAL
  profile_breakdown=1
  compile=1
  compile
  for algo in m-way m-pass; do
    for benchmark in "Stock" "Rovio" "YSB" "DEBS"; do #
      case "$benchmark" in
      "Stock")
        id=38
        ResetParameters
        SetStockParameters
        SET_RAND_BUFFER_SIZE
        compile
        benchmarkRun
        ;;
      "Rovio") #matches:
        id=39
        ResetParameters
        SetRovioParameters
        SET_RAND_BUFFER_SIZE
        compile
        benchmarkRun
        ;;
      "YSB")
        id=40
        ResetParameters
        SetYSBParameters
        SET_RAND_BUFFER_SIZE
        compile
        benchmarkRun
        ;;
      "DEBS")
        id=41
        ResetParameters
        SetDEBSParameters
        SET_RAND_BUFFER_SIZE
        compile
        benchmarkRun
        ;;
      esac
    done
  done
  let gp++
done

COMMENT

#####   trade off

sed -i -e "s/#define NO_SORT_SAMPLE_ON/#define SORT_SAMPLE_ON/g" ../main.cpp
sed -i -e "s/#define NO_SORT_AVX_RAND/#define SORT_AVX_RAND/g" ../main.cpp

gp=21000

for epsl in 0.9 0.8 0.7 0.6 0.5 0.4 0.3 0.2 0.1 0.05; do
  epsl_r=$epsl
  epsl_s=$epsl
  for iii in {1..50}; do
    NORMAL
    profile_breakdown=1
    compile=1
    compile
    for algo in m-way m-pass; do
      # for benchmark in  "Rovio" ; do #
      for benchmark in "Stock" "Rovio" "YSB" "DEBS"; do #
        case "$benchmark" in
        "Stock")
          id=38
          ResetParameters
          SetStockParameters
          SET_RAND_BUFFER_SIZE
          compile
          benchmarkRun
          ;;
        "Rovio") #matches:
          id=39
          ResetParameters
          SetRovioParameters
          SET_RAND_BUFFER_SIZE
          compile
          benchmarkRun
          ;;
        "DEBS")
          id=41
          ResetParameters
          SetDEBSParameters
          SET_RAND_BUFFER_SIZE
          compile
          benchmarkRun
          ;;
        esac
      done
    done
    let gp++
  done
done

<<COMMENT

# COMMENT

thread_test=1

## AVX
rand_buffer_size=1000


sed -i -e "s/#define NO_SORT_SAMPLE_ON/#define SORT_SAMPLE_ON/g" ../main.cpp
sed -i -e "s/#define NO_SORT_AVX_RAND/#define SORT_AVX_RAND/g" ../main.cpp

gp=3100
epsl_r=0.5
epsl_s=0.5
bern=1
univ=1

for rand_buffer_size in 10 33 66 100 333 666 1000 3333 6666 10000 33333 666666 100000; do
  for iii in {1..2}; do
    NORMAL
    profile_breakdown=1
    compile=1
    compile
    for algo in m-way m-pass; do
      for benchmark in "Stock" "Rovio" "YSB" "DEBS"; do #
        case "$benchmark" in
        "Stock")
          id=38
          ResetParameters
          SetStockParameters
          SET_RAND_BUFFER_SIZE
          compile
          benchmarkRun
          ;;
        "Rovio") #matches:
          id=39
          ResetParameters
          SetRovioParameters
          SET_RAND_BUFFER_SIZE
          compile
          benchmarkRun
          ;;
        "YSB")
          id=40
          ResetParameters
          SetYSBParameters
          SET_RAND_BUFFER_SIZE
          compile
          benchmarkRun
          ;;
        "DEBS")
          id=41
          ResetParameters
          SetDEBSParameters
          SET_RAND_BUFFER_SIZE
          compile
          benchmarkRun
          ;;
        esac
      done
    done
    let gp++
  done
done

# COMMENT

################################ MULTICORE

rand_buffer_size=1000

sed -i -e "s/#define NO_SORT_SAMPLE_ON/#define SORT_SAMPLE_ON/g" ../main.cpp
sed -i -e "s/#define NO_SORT_AVX_RAND/#define SORT_AVX_RAND/g" ../main.cpp

gp=4100
epsl_r=0.5
epsl_s=0.5
bern=1
univ=1

thread_test=0

for Threads in 1 2 4 8; do
  for iii in {1..2}; do
    NORMAL
    profile_breakdown=1
    compile=1
    compile
    for algo in m-way m-pass; do
      for benchmark in "Stock" "Rovio" "YSB" "DEBS"; do #
        case "$benchmark" in
        "Stock")
          id=38
          ResetParameters
          SetStockParameters
          SET_RAND_BUFFER_SIZE
          compile
          benchmarkRun
          ;;
        "Rovio") #matches:
          id=39
          ResetParameters
          SetRovioParameters
          SET_RAND_BUFFER_SIZE
          compile
          benchmarkRun
          ;;
        "YSB")
          id=40
          ResetParameters
          SetYSBParameters
          SET_RAND_BUFFER_SIZE
          compile
          benchmarkRun
          ;;
        "DEBS")
          id=41
          ResetParameters
          SetDEBSParameters
          SET_RAND_BUFFER_SIZE
          compile
          benchmarkRun
          ;;
        esac
      done
    done
    let gp++
  done
done

# COMMENT


##################################  STREAM FEATURE


thread_test=1

rand_buffer_size=1000

sed -i -e "s/#define NO_SORT_SAMPLE_ON/#define SORT_SAMPLE_ON/g" ../main.cpp
sed -i -e "s/#define NO_SORT_AVX_RAND/#define SORT_AVX_RAND/g" ../main.cpp

gp=5100
epsl_r=0.5
epsl_s=0.5
bern=1
univ=1


NORMAL
profile_breakdown=0
compile=1
compile
for algo in m-way m-pass; do
  for benchmark in "AR" "AD"; do #
    case "$benchmark" in
    # Batch -a SHJ_JM_NP -n 8 -t 1 -w 1000 -e 1000 -l 10 -d 0 -Z 1
    "AR") #test arrival rate and assume both inputs have same arrival rate.
      id=0
      ResetParameters
      FIXS=0 #varying both.
      ts=1   # stream case
      # step size should be bigger than nthreads
      for STEP_SIZE in 1600 3200 6400 12800 25600; do
        #WINDOW_SIZE=$(expr $DEFAULT_WINDOW_SIZE \* $DEFAULT_STEP_SIZE / $STEP_SIZE) #ensure relation size is the same.
        echo relation size is $(expr $WINDOW_SIZE / $INTERVAL \* $STEP_SIZE)
        gap=$(($STEP_SIZE / 500 * $WINDOW_SIZE))
        KimRun
        let "id++"
      done
      ;;
    "AD") #test arrival distribution
      id=10
      ResetParameters
      FIXS=1
      STEP_SIZE=1600
      STEP_SIZE_S=1600
      TS_DISTRIBUTION=2
      echo test varying timestamp distribution 10 - 14
      for ZIPF_FACTOR in 0 0.4 0.8 1.2 1.6; do #
        gap=$(($STEP_SIZE / 500 * $WINDOW_SIZE))
        KimRun
        let "id++"
      done
      ;;
    esac
  done
done

COMMENT
