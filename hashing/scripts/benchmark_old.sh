#!/bin/bash

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

## Set L3 Cache according to your machine.
sed -i -e "s/#define L3_CACHE_SIZE [[:alnum:]]*/#define L3_CACHE_SIZE $L3_cache_size/g" ../utils/params.h
# set experiment dir
sed -i -e "s/#define EXP_DIR .*/#define EXP_DIR "\"${exp_dir//\//\\/}\""/g" ../joins/common_functions.h
sed -i -e "s/#define PERF_COUNTERS/#define NO_PERF_COUNTERS/g" ../utils/perf_counters.h
sed -i -e "s/#define PROFILE_TOPDOWN/#define NO_PROFILE_TOPDOWN/g" ../utils/perf_counters.h
sed -i -e "s/#define PROFILE_MEMORY_CONSUMPTION/#define NO_PROFILE_MEMORY_CONSUMPTION/g" ../utils/perf_counters.h
sed -i -e "s/#define NO_TIMING/#define TIMING/g" ../joins/common_functions.h

# change cpu-mapping path here, e.g. following changes $exp_dir/cpu-mapping.txt to $exp_dir/cpu-mapping.txt
#sed -i -e "s/\/data1\/xtra\/cpu-mapping.txt/\/data1\/xtra\/cpu-mapping.txt/g" ../affinity/cpu_mapping.h

compile=1 #enable compiling.
eager=1
profile_breakdown=1

function compile() {
  if [ $compile != 0 ]; then
    if [ $eager == 0 ] || [ $profile_breakdown == 1 ]; then #to reduce profile overhead, we postpone eager joins during profiling.
      sed -i -e "s/#define EAGER/#define NO_EAGER/g" ../joins/common_functions.h
    else
      sed -i -e "s/#define NO_EAGER/#define EAGER/g" ../joins/common_functions.h
    fi
    cd ..
    cmake . | tail -n +90
    cd scripts
    make -C .. clean -s
    make -C .. -j4 -s
  fi
  echo tangxilin | sudo -S setcap CAP_SYS_RAWIO+eip ../hashing
}

function Run() {
  #####native execution
  echo "==benchmark:$benchmark -a $algo -n $Threads=="
  #echo 3 >/proc/sys/vm/drop_caches
  echo tangxilin | sudo -S sysctl vm.drop_caches=3
  ../hashing -a $algo -r $RSIZE -s $SSIZE -n $Threads
}

function benchmarkRun() {
  #####native execution
  echo "==benchmark:$benchmark -a $algo -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -I $id -[ $progress_step -] $merge_step -G $group -g $gap -o $exp_dir/results/breakdown/profile_$id.txt =="
  #echo 3 >/proc/sys/vm/drop_caches
  echo tangxilin | sudo -S sysctl vm.drop_caches=3
  ../hashing -a $algo -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -I $id -[ $progress_step -] $merge_step -G $group -g $gap -o $exp_dir/results/breakdown/profile_$id.txt
  if [[ $? -eq 139 ]]; then echo "oops, sigsegv" exit -1; fi
}

function benchmarkProfileRun() {
  #####native execution
  echo "==benchmark:$benchmark -a $algo -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -I $id -[ $progress_step -] $merge_step -G $group -g $gap -o $exp_dir/results/breakdown/profile_$id.txt =="
  #echo 3 >/proc/sys/vm/drop_caches
  echo tangxilin | sudo -S sysctl vm.drop_caches=3
  if [ ! -z "$PERF_CONF" -a "$PERF_CONF"!=" " ]; then
    ../hashing -a $algo -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -I $id -[ $progress_step -] $merge_step -G $group -g $gap -o $exp_dir/results/breakdown/profile_$id.txt -p $PERF_CONF
  else
    ../hashing -a $algo -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -I $id -[ $progress_step -] $merge_step -G $group -g $gap -o $exp_dir/results/breakdown/profile_$id.txt
  fi
  if [[ $? -eq 139 ]]; then echo "oops, sigsegv" exit -1; fi
}

function perfUarchBenchmarkRun() {
  #####native execution
  echo "==benchmark:$benchmark -a $algo -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -I $id -[ $progress_step -] $merge_step -G $group -g $gap -o $exp_dir/results/breakdown/profile_$id.txt =="
  #echo 3 >/proc/sys/vm/drop_caches
  echo tangxilin | sudo -S sysctl vm.drop_caches=3
  if [ ! -z "$PERF_OUTPUT" -a "$PERF_OUTPUT"!=" " ]; then
    perf stat -x, -a -e CPU_CLK_UNHALTED.THREAD,IDQ_UOPS_NOT_DELIVERED.CORE,UOPS_ISSUED.ANY,UOPS_RETIRED.RETIRE_SLOTS,INT_MISC.RECOVERY_CYCLES,CYCLE_ACTIVITY.STALLS_MEM_ANY,RESOURCE_STALLS.SB -o $PERF_OUTPUT \
    ../hashing -a $algo -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -I $id -[ $progress_step -] $merge_step -G $group -g $gap
  else
    perf stat -e CPU_CLK_UNHALTED.THREAD,IDQ_UOPS_NOT_DELIVERED.CORE,UOPS_ISSUED.ANY,UOPS_RETIRED.RETIRE_SLOTS,INT_MISC.RECOVERY_CYCLES,CYCLE_ACTIVITY.STALLS_MEM_ANY,RESOURCE_STALLS.SB \
    ../hashing -a $algo -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -I $id -[ $progress_step -] $merge_step -G $group -g $gap
  fi
  if [[ $? -eq 139 ]]; then echo "oops, sigsegv" exit -1; fi
}

function perfUtilBenchmarkRun() {
  #####native execution
  echo "==benchmark:$benchmark -a $algo -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -I $id -[ $progress_step -] $merge_step -G $group -g $gap -o $exp_dir/results/breakdown/perf_$id.txt =="
  #echo 3 >/proc/sys/vm/drop_caches
  echo tangxilin | sudo -S sysctl vm.drop_caches=3
  perf stat -I10 -x, -o $exp_dir/results/breakdown/perf_$id.csv -e cache-misses,cycles ../hashing -a $algo -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -I $id -[ $progress_step -] $merge_step -G $group -g $gap
  if [[ $? -eq 139 ]]; then echo "oops, sigsegv" exit -1; fi
}

function KimRun() {
  #####native execution
  echo "==KIM benchmark:$benchmark -a $algo -t $ts -w $WINDOW_SIZE -e $STEP_SIZE -q $STEP_SIZE_S -l $INTERVAL -d $distrbution -z $skew -D $TS_DISTRIBUTION -Z $ZIPF_FACTOR -n $Threads -I $id -W $FIXS -[ $progress_step -] $merge_step -G $group -P $DD -g $gap =="
  #echo 3 >/proc/sys/vm/drop_caches
  echo tangxilin | sudo -S sysctl vm.drop_caches=3
  ../hashing -a $algo -t $ts -w $WINDOW_SIZE -e $STEP_SIZE -q $STEP_SIZE_S -l $INTERVAL -d $distrbution -z $skew -D $TS_DISTRIBUTION -Z $ZIPF_FACTOR -n $Threads -I $id -W $FIXS -[ $progress_step -] $merge_step -G $group -P $DD -g $gap
  if [[ $? -eq 139 ]]; then echo "oops, sigsegv" exit -1; fi
}

function KimProfileRun() {
  #####native execution
  echo "==KIM benchmark:$benchmark -a $algo -t $ts -w $WINDOW_SIZE -e $STEP_SIZE -q $STEP_SIZE_S -l $INTERVAL -d $distrbution -z $skew -D $TS_DISTRIBUTION -Z $ZIPF_FACTOR -n $Threads -I $id -W $FIXS -[ $progress_step -] $merge_step -G $group -P $DD -g $gap =="
  #echo 3 >/proc/sys/vm/drop_caches
  echo tangxilin | sudo -S sysctl vm.drop_caches=3
  if [ ! -z "$PERF_CONF" -a "$PERF_CONF"!=" " ]; then
    ../hashing -a $algo -t $ts -w $WINDOW_SIZE -e $STEP_SIZE -q $STEP_SIZE_S -l $INTERVAL -d $distrbution -z $skew -D $TS_DISTRIBUTION -Z $ZIPF_FACTOR -n $Threads -I $id -W $FIXS -[ $progress_step -] $merge_step -G $group -P $DD -g $gap -o $exp_dir/results/breakdown/profile_$id.txt -p $PERF_CONF
  else
    ../hashing -a $algo -t $ts -w $WINDOW_SIZE -e $STEP_SIZE -q $STEP_SIZE_S -l $INTERVAL -d $distrbution -z $skew -D $TS_DISTRIBUTION -Z $ZIPF_FACTOR -n $Threads -I $id -W $FIXS -[ $progress_step -] $merge_step -G $group -P $DD -g $gap -o $exp_dir/results/breakdown/profile_$id.txt
  fi
  if [[ $? -eq 139 ]]; then echo "oops, sigsegv" exit -1; fi
}

function PARTITION_ONLY() {
  sed -i -e "s/#define JOIN/#define NO_JOIN/g" ../joins/common_functions.h
  sed -i -e "s/#define MERGE/#define NO_MERGE/g" ../joins/common_functions.h
  sed -i -e "s/#define MATCH/#define NO_MATCH/g" ../joins/common_functions.h
  sed -i -e "s/#define WAIT/#define NO_WAIT/g" ../joins/common_functions.h
  sed -i -e "s/#define OVERVIEW/#define NO_OVERVIEW/g" ../joins/common_functions.h
}

function PARTITION_BUILD_SORT() {
  sed -i -e "s/#define NO_JOIN/#define JOIN/g" ../joins/common_functions.h
  sed -i -e "s/#define MERGE/#define NO_MERGE/g" ../joins/common_functions.h
  sed -i -e "s/#define MATCH/#define NO_MATCH/g" ../joins/common_functions.h
  sed -i -e "s/#define WAIT/#define NO_WAIT/g" ../joins/common_functions.h
  sed -i -e "s/#define OVERVIEW/#define NO_OVERVIEW/g" ../joins/common_functions.h
}

function PARTITION_BUILD_SORT_MERGE() {
  sed -i -e "s/#define NO_JOIN/#define JOIN/g" ../joins/common_functions.h
  sed -i -e "s/#define NO_MERGE/#define MERGE/g" ../joins/common_functions.h
  sed -i -e "s/#define MATCH/#define NO_MATCH/g" ../joins/common_functions.h
  sed -i -e "s/#define WAIT/#define NO_WAIT/g" ../joins/common_functions.h
  sed -i -e "s/#define OVERVIEW/#define NO_OVERVIEW/g" ../joins/common_functions.h
}

function PARTITION_BUILD_SORT_MERGE_JOIN() {
  sed -i -e "s/#define NO_JOIN/#define JOIN/g" ../joins/common_functions.h
  sed -i -e "s/#define NO_MERGE/#define MERGE/g" ../joins/common_functions.h
  sed -i -e "s/#define NO_MATCH/#define MATCH/g" ../joins/common_functions.h
  sed -i -e "s/#define WAIT/#define NO_WAIT/g" ../joins/common_functions.h
  sed -i -e "s/#define OVERVIEW/#define NO_OVERVIEW/g" ../joins/common_functions.h
}

function ALL_ON() {
  sed -i -e "s/#define NO_JOIN/#define JOIN/g" ../joins/common_functions.h
  sed -i -e "s/#define NO_MERGE/#define MERGE/g" ../joins/common_functions.h
  sed -i -e "s/#define NO_MATCH/#define MATCH/g" ../joins/common_functions.h
  sed -i -e "s/#define NO_WAIT/#define WAIT/g" ../joins/common_functions.h
  sed -i -e "s/#define NO_OVERVIEW/#define OVERVIEW/g" ../joins/common_functions.h
}

# Different execution mode for different experiments

function PCM() {
  sed -i -e "s/#define TIMING/#define NO_TIMING/g" ../joins/common_functions.h #disable time measurement
  sed -i -e "s/#define NO_PERF_COUNTERS/#define PERF_COUNTERS/g" ../utils/perf_counters.h
  sed -i -e "s/#define PROFILE_TOPDOWN/#define NO_PROFILE_TOPDOWN/g" ../joins/common_functions.h
  sed -i -e "s/#define PROFILE_MEMORY_CONSUMPTION/#define NO_PROFILE_MEMORY_CONSUMPTION/g" ../joins/common_functions.h
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

function FULLBENCHRUN() {
  PARTITION_ONLY
  compile
  echo "PARTITION_ONLY"
  benchmarkRun

  PARTITION_BUILD_SORT
  compile
  echo "PARTITION_BUILD_SORT"
  benchmarkRun

  PARTITION_BUILD_SORT_MERGE
  compile
  echo "PARTITION_BUILD_SORT_MERGE"
  benchmarkRun

  PARTITION_BUILD_SORT_MERGE_JOIN
  compile
  echo "PARTITION_BUILD_SORT_MERGE_JOIN"
  benchmarkRun

  ALL_ON
  compile
  echo "ALL_ON"
  benchmarkRun
}

function SHJBENCHRUN() {
  PARTITION_ONLY
  compile
  echo "PARTITION_ONLY"
  benchmarkRun

  PARTITION_BUILD_SORT
  compile
  echo "PARTITION_BUILD_SORT"
  benchmarkRun

  PARTITION_BUILD_SORT_MERGE_JOIN
  compile
  echo "PARTITION_BUILD_SORT_MERGE_JOIN"
  benchmarkRun

  ALL_ON
  compile
  echo "ALL_ON"
  benchmarkRun
}

function FULLKIMRUN() {
  PARTITION_ONLY
  compile
  echo "PARTITION_ONLY"
  KimRun

  PARTITION_BUILD_SORT
  compile
  echo "PARTITION_BUILD_SORT"
  KimRun

  PARTITION_BUILD_SORT_MERGE
  compile
  echo "PARTITION_BUILD_SORT_MERGE"
  KimRun

  PARTITION_BUILD_SORT_MERGE_JOIN
  compile
  echo "PARTITION_BUILD_SORT_MERGE_JOIN"
  KimRun

  ALL_ON
  compile
  echo "ALL_ON"
  KimRun
}

function SHJKIMRUN() {
  PARTITION_ONLY
  compile
  KimRun

  PARTITION_BUILD_SORT
  compile
  KimRun

  PARTITION_BUILD_SORT_MERGE_JOIN
  compile
  KimRun

  ALL_ON
  compile
  echo "ALL_ON"
  KimRun
}

function RUNALL() {
  if [ $profile_breakdown == 1 ]; then
    if [ $algo == SHJ_JM_P ] || [ $algo == SHJ_JM_NP ] || [ $algo == SHJ_JBCR_P ]|| [ $algo == SHJ_JBCR_NP ] ; then
      SHJBENCHRUN
    else
      if [ $algo == PMJ_JM_P ] || [ $algo == PMJ_JM_NP ] || [ $algo == PMJ_JBCR_P ] || [ $algo == PMJ_JBCR_NP ];  then
        FULLBENCHRUN
      else
        benchmarkRun
      fi
    fi
  else
    ALL_ON
    compile
    benchmarkRun
  fi
}

function RUNALLMic() {
  if [ $profile_breakdown == 1 ]; then
    if [ $algo == SHJ_JM_P ] || [ $algo == SHJ_JM_NP ] || [ $algo == SHJ_JBCR_P ] || [ $algo == SHJ_JBCR_NP ] || [ $algo == SHJ_HS_P ]|| [ $algo == SHJ_HS_NP ]; then
      SHJKIMRUN
    else
      if [ $algo == PMJ_JM_P ] || [ $algo == PMJ_JM_NP ] || [ $algo == PMJ_JBCR_P ] || [ $algo == PMJ_JBCR_NP ]; then
        FULLKIMRUN
      else
        KimRun
      fi
    fi
  else
    ALL_ON
    compile
    KimRun
  fi
}

function SetStockParameters() { #matches: 15598112. #inputs= 60527 + 77227
  ts=1 # stream case
  WINDOW_SIZE=1000
  RSIZE=60527
  SSIZE=77227
  RPATH=$exp_dir/datasets/stock/cj_1000ms_1t.txt
  SPATH=$exp_dir/datasets/stock/sb_1000ms_1t.txt
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
  gap=87856849
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
  Threads=8
  progress_step=20
  merge_step=16 #not in use.
  group=2
  gap=12800
  DD=1
  sed -i -e "s/scalarflag [[:alnum:]]*/scalarflag 0/g" ../helper/sort_common.h
  sed -i -e "s/NUM_RADIX_BITS [[:alnum:]]*/NUM_RADIX_BITS 8/g" ../joins/prj_params.h
}

#compile once by default.
compile
# Configurable variables
# Generate a timestamp
timestamp=$(date +%Y%m%d-%H%M)
output=test$timestamp.txt

## APP benchmark.
#APP_BENCH=0
if [ $APP_BENCH == 1 ]; then
  NORMAL
  #compile depends on whether we want to profile.
  for profile_breakdown in 1; do
    compile=1
    for benchmark in "Stock" "Rovio" "YSB" "DEBS"; do # "Stock" "Rovio" "YSB" "DEBS"
      for algo in NPO PRO SHJ_JM_NP SHJ_JBCR_NP PMJ_JM_NP PMJ_JBCR_NP; do # NPO PRO SHJ_JM_NP SHJ_JBCR_NP PMJ_JM_NP PMJ_JBCR_NP
      # for algo in SHJ_JM_NP SHJ_JBCR_NP PMJ_JM_NP PMJ_JBCR_NP; do # NPO PRO SHJ_JM_NP SHJ_JBCR_NP PMJ_JM_NP PMJ_JBCR_NP
        case "$benchmark" in
        "Stock")
          id=38
          ResetParameters
          SetStockParameters
          RUNALL
          ;;
        "Rovio") #matches:
          id=39
          ResetParameters
          SetRovioParameters
          RUNALL
          ;;
        "YSB")
          id=40
          ResetParameters
          SetYSBParameters
          RUNALL
          ;;
        "DEBS")
          id=41
          ResetParameters
          SetDEBSParameters
          RUNALL
          ;;
        esac
      done
    done
  done
fi

## MICRO benchmark.
#MICRO_BENCH=0
if [ $MICRO_BENCH == 1 ]; then
  NORMAL
  profile_breakdown=0        # set to 1 if we want to measure time breakdown!
  compile=$profile_breakdown # compile depends on whether we want to profile.
  for benchmark in "AR" "RAR" "AD" "KD" "WS" "DD"; do #
    for algo in NPO NPO PRO SHJ_JM_NP SHJ_JBCR_NP PMJ_JM_NP PMJ_JBCR_NP; do
      case "$benchmark" in
      # Batch -a SHJ_JM_P -n 8 -t 1 -w 1000 -e 1000 -l 10 -d 0 -Z 1
      "AR") #test arrival rate and assume both inputs have same arrival rate.
        id=0
        ## Figure 1
        ResetParameters
        FIXS=0 #varying both.
        ts=1   # stream case
        # step size should be bigger than nthreads
        for STEP_SIZE in 1600 3200 6400 12800 25600; do #128000
          #WINDOW_SIZE=$(expr $DEFAULT_WINDOW_SIZE \* $DEFAULT_STEP_SIZE / $STEP_SIZE) #ensure relation size is the same.
          echo relation size is $(expr $WINDOW_SIZE / $INTERVAL \* $STEP_SIZE)
          gap=$(($STEP_SIZE / 500 * $WINDOW_SIZE))
          RUNALLMic
          let "id++"
        done
        ;;
      "RAR") #test relative arrival rate when R is small
        id=5
        ## Figure 2
        ResetParameters
        FIXS=1
        echo test relative arrival rate 5 - 9
        ts=1 # stream case
        # step size should be bigger than nthreads
        # remember to fix the relation size of S.
        STEP_SIZE=1600
        for STEP_SIZE_S in 1600 3200 6400 12800 25600; do
          #        WINDOW_SIZE=$(expr $DEFAULT_WINDOW_SIZE \* $DEFAULT_STEP_SIZE / $STEP_SIZE) #ensure relation size is the same.
          echo relation size is $(expr $WINDOW_SIZE / $INTERVAL \* $STEP_SIZE)
          gap=$(($STEP_SIZE / 500 * $WINDOW_SIZE))
          RUNALLMic
          let "id++"
        done
        ;;
      "AD") #test arrival distribution
        id=10
        ## Figure 3
        ResetParameters
        FIXS=1
        STEP_SIZE=1600
        STEP_SIZE_S=1600
        TS_DISTRIBUTION=2
        echo test varying timestamp distribution 10 - 14
        for ZIPF_FACTOR in 0 0.4 0.8 1.2 1.6; do #
          gap=$(($STEP_SIZE / 500 * $WINDOW_SIZE))
          RUNALLMic
          let "id++"
        done
        ;;
      "KD") #test key distribution
        id=15
        ## Figure 4
        ResetParameters
        FIXS=1
        STEP_SIZE=12800
        STEP_SIZE_S=12800
        gap=$(($STEP_SIZE / 500 * $WINDOW_SIZE))
        echo test varying key distribution 15 - 19
        distrbution=2 #varying zipf factor
        for skew in 0 0.4 0.8 1.2 1.6; do
          if [ $skew == 1.2 ]; then
            gap=100
          fi
          if [ $skew == 1.6 ]; then
            gap=1
          fi
          RUNALLMic
          let "id++"
        done
        ;;
      "WS") #test window size
        id=20
        ## Figure 5
        ResetParameters
        FIXS=1
        STEP_SIZE=6400
        STEP_SIZE_S=6400
        echo test varying window size 20 - 24
        for WINDOW_SIZE in 500 1000 1500 2000 2500; do
          gap=$(($STEP_SIZE / 500 * $WINDOW_SIZE))
          RUNALLMic
          let "id++"
        done
        ;;
      "DD") #test data duplication
        id=25
        ## Figure 6
        ResetParameters
        ts=1
        FIXS=1
        STEP_SIZE=6400
        STEP_SIZE_S=6400
        echo test DD 25 - 28
        for DD in 1 100 1000 10000; do
          gap=$(($STEP_SIZE * $WINDOW_SIZE * $DD / 500))
          RUNALLMic
          let "id++"
        done
        ;;
      esac
    done
  done
fi

## SCLAE STUDY
#SCALE_STUDY=0
if [ $SCALE_STUDY == 1 ]; then
  NORMAL
  profile_breakdown=0                                                                     #compile depends on whether we want to profile.
  compile=0
  # general benchmark.
  for algo in SHJ_JM_NP; do
    for benchmark in "ScaleStock" "ScaleRovio" "ScaleYSB" "ScaleDEBS"; do #
      case "$benchmark" in
      "ScaleStock")
        id=42
        ResetParameters
        SetStockParameters
        echo test scalability of Stock 42 - 45
        for Threads in 1 2 4 8; do
          RUNALL
          let "id++"
        done
        ;;
      "ScaleRovio")
        id=46
        ResetParameters
        SetRovioParameters
        echo test scalability 46 - 49
        for Threads in 1 2 4 8; do
          RUNALL
          let "id++"
        done
        ;;
      "ScaleYSB")
        id=50
        ResetParameters
        SetYSBParameters
        echo test scalability 50 - 53
        for Threads in 1 2 4 8; do
          RUNALL
          let "id++"
        done
        ;;
      "ScaleDEBS")
        id=54
        ResetParameters
        SetDEBSParameters
        echo test scalability 54 - 57
        for Threads in 1 2 4 8; do
          RUNALL
          let "id++"
        done
        ;;
      esac
    done
  done
fi
## back up.
#  "PMJ_MERGE_STEP_STUDY")
#    id=66
#    algo="PMJ_JBCR_P"
#    ResetParameters
#    echo PMJ_MERGE_STEP_STUDY 66-70
#    for merge_step in 8 10 12 14 16; do
#      ts=0   # batch data.
#      KimRun #
#      let "id++"
#    done
#    python3 breakdown_merge.py
#    python3 latency_merge.py
#    python3 progressive_merge.py
#    ;;

## MICRO STUDY
#PROFILE_MICRO=0
if [ $PROFILE_MICRO == 1 ]; then
  NORMAL
  profile_breakdown=1                                                                     #compile depends on whether we want to profile.
  compile=1                                                                               #enable compiling.
  #benchmark experiment only apply for hashing directory.
  for benchmark in  "NP_P_STUDY" "SIMD_STUDY" "PMJ_SORT_STEP_STUDY" "GROUP_SIZE_STUDY" "BUCKET_SIZE_STUDY"  "PRJ_RADIX_BITS_STUDY"; do # "SIMD_STUDY" "PMJ_SORT_STEP_STUDY" "GROUP_SIZE_STUDY" "BUCKET_SIZE_STUDY"  "PRJ_RADIX_BITS_STUDY"
    case "$benchmark" in
    "SIMD_STUDY")
      id=104
      ResetParameters
      ts=0 # batch data.
      echo SIMD PMJ 104 - 107
      PARTITION_ONLY
      compile
      for algo in "PMJ_JM_NP" "PMJ_JBCR_NP"; do
        for scalar in 0 1; do
          sed -i -e "s/scalarflag [[:alnum:]]*/scalarflag $scalar/g" ../helper/sort_common.h
          RUNALLMic
          let "id++"
        done
      done
      PARTITION_BUILD_SORT
      compile
      for algo in "PMJ_JM_NP" "PMJ_JBCR_NP"; do
        for scalar in 0 1; do
          sed -i -e "s/scalarflag [[:alnum:]]*/scalarflag $scalar/g" ../helper/sort_common.h
          RUNALLMic
          let "id++"
        done
      done
      python3 breakdown_simd.py
      python3 profile_simd.py
      ;;
    "BUCKET_SIZE_STUDY")
      id=108
      ResetParameters
      ts=0 # batch data.
      for algo in "NPO"; do
        for size in 1 2 4 8 16; do
          echo BUCKET_SIZE_STUDY $id
          sed -i -e "s/#define BUCKET_SIZE [[:alnum:]]*/#define BUCKET_SIZE $size/g" ../joins/npj_params.h
          compile
          RUNALLMic
          let "id++"
        done
      done
      python3 breakdown_bucket.py
      ;;
    "PRJ_RADIX_BITS_STUDY")
      algo="PRO"
      id=113
      ResetParameters
      ts=0 # batch data.
      for b in 8 10 12 14 16 18; do
        echo RADIX BITS STUDY $id
        sed -i -e "s/NUM_RADIX_BITS [[:alnum:]]*/NUM_RADIX_BITS $b/g" ../joins/prj_params.h
        compile
        RUNALLMic
        let "id++"
      done
      python3 breakdown_radix.py
      python3 latency_radix.py
      python3 progressive_radix.py
      ;;
    "PMJ_SORT_STEP_STUDY")
      id=119
      algo="PMJ_JBCR_NP"
      ResetParameters
      ts=0 # batch data.
      for progress_step in 10 20 30 40 50; do #%
        echo PMJ_SORT_STEP_STUDY $id
        RUNALLMic
        let "id++"
      done
      python3 breakdown_sort.py
      python3 latency_sort.py
      python3 progressive_sort.py
      ;;
    "GROUP_SIZE_STUDY")
      id=124
      ResetParameters
      ts=0 # batch data.
      algo="PMJ_JM_NP"
      RUNALLMicz
      algo="SHJ_JM_NP"
      RUNALLMic

      algo="PMJ_JBCR_NP"
      echo GROUP_SIZE_STUDY PMJ 124 - 127
      for group in 1 2 4 8; do
        RUNALLMic
        let "id++"
      done

      algo="SHJ_JBCR_NP"
      echo GROUP_SIZE_STUDY SHJ 128 - 131
      for group in 1 2 4 8; do
        RUNALLMic
        let "id++"
      done
      python3 breakdown_group_pmj.py
      python3 breakdown_group_shj.py
      ;;
    "HS_STUDY")
      id=132
      ResetParameters
      ts=0 # batch data.
      algo="SHJ_HS_NP"
      RUNALLMic
      algo="SHJ_JM_NP"
      RUNALLMic
      python3 breakdown_hsstudy.py
      ;;
    "NP_P_STUDY")
      id=133
      ResetParameters
      ts=0 # batch data.
      algo="SHJ_JM_NP"
      RUNALLMic
      algo="SHJ_JM_P"
      RUNALLMic
      python3 breakdown_p_np_study.py
      ;;
    esac
  done
fi

#PROFILE=0 ## Cache misses profiling, please run the program with sudo
if [ $PROFILE == 1 ]; then
  PCM
  profile_breakdown=0 # disable measure time breakdown!
  eager=1             #with eager
  compile=1

  PARTITION_ONLY
  compile
  for benchmark in "YSB"; do #"
    id=205
    for algo in NPO PRO SHJ_JM_NP SHJ_JBCR_NP PMJ_JM_NP PMJ_JBCR_NP; do #PRO SHJ_JM_P SHJ_JBCR_P PMJ_JM_P PMJ_JBCR_P
      case "$benchmark" in
      "YSB")
        ResetParameters
        SetYSBParameters
        rm $exp_dir/results/breakdown/profile_$id.txt
        benchmarkRun
        ;;
      esac
      let "id++"
    done
  done

  PARTITION_BUILD_SORT_MERGE_JOIN
  compile
  for benchmark in "YSB"; do #"
    id=211
    for algo in NPO PRO SHJ_JM_NP SHJ_JBCR_NP PMJ_JM_NP PMJ_JBCR_NP; do # ~215 NPO PRO SHJ_JM_P SHJ_JBCR_P PMJ_JM_P PMJ_JBCR_P
      case "$benchmark" in
      "YSB")
        ResetParameters
        SetYSBParameters
        rm $exp_dir/results/breakdown/profile_$id.txt
        benchmarkRun
        ;;
      esac
      let "id++"
    done
  done
  NORMAL
fi

#PROFILE_MEMORY_CONSUMPTION=1 ## profile memory consumption
if [ $PROFILE_MEMORY_CONSUMPTION == 1 ]; then
  MEM_MEASURE
  profile_breakdown=0
  compile=1
  compile
  for benchmark in "Rovio"; do #"YSB
    id=302
    for algo in NPO PRO SHJ_JM_NP SHJ_JBCR_NP PMJ_JM_NP PMJ_JBCR_NP; do # NPO PRO SHJ_JM_NP SHJ_JBCR_NP PMJ_JM_NP PMJ_JBCR_NP
      case "$benchmark" in
      "Kim")
        ResetParameters
        STEP_SIZE=1280
        STEP_SIZE_S=12800
        WINDOW_SIZE=10000
        rm $exp_dir/results/breakdown/perf_$id.csv
        KimRun
        ;;
      "YSB")
        ResetParameters
        SetYSBParameters
        rm $exp_dir/results/breakdown/perf_$id.txt
        benchmarkRun
        ;;
      "Rovio")
        ResetParameters
        SetRovioParameters
        rm $exp_dir/results/breakdown/perf_$id.txt
        benchmarkRun
        ;;
      esac
      let "id++"
    done
  done
  NORMAL
fi

#PROFILE_PMU_COUNTERS=1 # profile PMU counters using pcm
if [ $PROFILE_PMU_COUNTERS == 1 ]; then
  PCM
  profile_breakdown=0 # disable measure time breakdown!
  ALL_ON # eliminate wait phase
  compile=1
  compile
  for benchmark in "Rovio"; do #"YSB
    id=402
    for algo in NPO PRO SHJ_JM_NP SHJ_JBCR_NP PMJ_JM_NP PMJ_JBCR_NP; do # NPO PRO SHJ_JM_NP SHJ_JBCR_NP PMJ_JM_NP PMJ_JBCR_NP
      case "$benchmark" in
      "Rovio")
        ResetParameters
        SetRovioParameters
        rm $exp_dir/results/breakdown/profile_$id.txt
        PERF_CONF=$exp_dir/pcm.cfg
        benchmarkProfileRun
        PERF_CONF=$exp_dir/pcm2.cfg
        benchmarkProfileRun
        PERF_CONF=""
        benchmarkProfileRun
        ;;
      "YSB")
        ResetParameters
        SetYSBParameters
        rm $exp_dir/results/breakdown/profile_$id.txt
        PERF_CONF=$exp_dir/pcm.cfg
        benchmarkProfileRun
        PERF_CONF=$exp_dir/pcm2.cfg
        benchmarkProfileRun
        PERF_CONF=""
        benchmarkProfileRun
        ;;
      esac
      let "id++"
    done
  done
  NORMAL
fi

#PROFILE_TOPDOWN=1 ## profile intel topdown performance metrics using perf/pcm
if [ $PROFILE_TOPDOWN == 1 ]; then
  PERF

  for benchmark in "Rovio"; do
    id=402
    for algo in NPO PRO SHJ_JM_NP SHJ_JBCR_NP PMJ_JM_NP PMJ_JBCR_NP ; do # NPO PRO SHJ_JM_NP SHJ_JBCR_NP PMJ_JM_NP PMJ_JBCR_NP
      case "$benchmark" in
      "YSB")
        ResetParameters
        SetYSBParameters
        # use perf
        # with JOIN
        sed -i -e "s/#define NO_JOIN_THREAD/#define JOIN_THREAD/g" ../joins/common_functions.h
        profile_breakdown=0
        compile=1
        compile
        PERF_OUTPUT=$exp_dir/results/breakdown/profile_w_join_$id.txt
        perfUarchBenchmarkRun
        # without JOIN
        sed -i -e "s/#define JOIN_THREAD/#define NO_JOIN_THREAD/g" ../joins/common_functions.h
        profile_breakdown=0
        compile=1
        compile
        PERF_OUTPUT=$exp_dir/results/breakdown/profile_wo_join_$id.txt
        perfUarchBenchmarkRun
        ;;
      "Rovio")
        ResetParameters
        SetRovioParameters
        # use perf
        # with JOIN
        sed -i -e "s/#define NO_JOIN_THREAD/#define JOIN_THREAD/g" ../joins/common_functions.h
        profile_breakdown=0
        compile=1
        compile
        PERF_OUTPUT=$exp_dir/results/breakdown/profile_w_join_$id.txt
        # without JOIN
        perfUarchBenchmarkRun
        sed -i -e "s/#define JOIN_THREAD/#define NO_JOIN_THREAD/g" ../joins/common_functions.h
        compile=1
        compile
        PERF_OUTPUT=$exp_dir/results/breakdown/profile_wo_join_$id.txt
        perfUarchBenchmarkRun
        ;;
      esac
      let "id++"
    done
  done
  NORMAL
fi

#./draw.sh
#python3 jobdone.py
