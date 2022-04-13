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
  echo "==benchmark:$benchmark -a $algo -E $epsl_r -F $epsl_s -U $univ -Q $bern -n $Threads=="
  #echo 3 >/proc/sys/vm/drop_caches
  echo tangxilin | sudo -S sysctl vm.drop_caches=3
  ../hashing -a $algo -E $epsl_r -F $epsl_s -U $univ -Q $bern -r $RSIZE -s $SSIZE -n $Threads
}

function benchmarkRun() {
  #####native execution
  echo "==benchmark:$benchmark -a $algo -E $epsl_r -F $epsl_s -U $univ -Q $bern -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -I $id -H $gp -[ $progress_step -] $merge_step -G $group -g $gap -o $exp_dir/results/breakdown/profile_${gp}_${id}.txt > ${exp_dir}/results/breakdown/${phase}_${benchmark}_${algo}_profile_${gp}_${id}.txt =="
  #echo 3 >/proc/sys/vm/drop_caches
  echo tangxilin | sudo -S sysctl vm.drop_caches=3
  ../hashing -a $algo -E $epsl_r -F $epsl_s -U $univ -Q $bern -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -I $id -H $gp -[ $progress_step -] $merge_step -G $group -g $gap -o $exp_dir/results/breakdown/profile_${gp}_${id}.txt > ${exp_dir}/results/breakdown/${phase}_${benchmark}_${algo}_profile_${gp}_${id}.txt  
  if [[ $? -eq 139 ]]; then echo "oops, sigsegv" exit -1; fi
}

function benchmarkProfileRun() {
  #####native execution
  echo "==benchmark:$benchmark -a $algo -E $epsl_r -F $epsl_s -U $univ -Q $bern -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -I $id -H $gp -[ $progress_step -] $merge_step -G $group -g $gap -o $exp_dir/results/breakdown/profile_${gp}_${id}.txt > ${exp_dir}/results/breakdown/${phase}_${benchmark}_${algo}_profile_${gp}_${id}.txt =="
  #echo 3 >/proc/sys/vm/drop_caches
  echo tangxilin | sudo -S sysctl vm.drop_caches=3
  if [ ! -z "$PERF_CONF" -a "$PERF_CONF"!=" " ]; then
    ../hashing -a $algo -E $epsl_r -F $epsl_s -U $univ -Q $bern -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -I $id -H $gp -[ $progress_step -] $merge_step -G $group -g $gap -o $exp_dir/results/breakdown/profile_${gp}_${id}.txt -p $PERF_CONF > ${exp_dir}/results/breakdown/${phase}_${benchmark}_${algo}_profile_${gp}_${id}.txt 
  else
    ../hashing -a $algo -E $epsl_r -F $epsl_s -U $univ -Q $bern -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -I $id -H $gp -[ $progress_step -] $merge_step -G $group -g $gap -o $exp_dir/results/breakdown/profile_${gp}_${id}.txt
  fi
  if [[ $? -eq 139 ]]; then echo "oops, sigsegv" exit -1; fi
}

function perfUarchBenchmarkRun() {
  #####native execution
  echo "==benchmark:$benchmark -a $algo -E $epsl_r -F $epsl_s -U $univ -Q $bern -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -I $id -H $gp -[ $progress_step -] $merge_step -G $group -g $gap -o $exp_dir/results/breakdown/profile_${gp}_${id}.txt > ${exp_dir}/results/breakdown/${phase}_${benchmark}_${algo}_profile_${gp}_${id}.txt =="
  #echo 3 >/proc/sys/vm/drop_caches
  echo tangxilin | sudo -S sysctl vm.drop_caches=3
  if [ ! -z "$PERF_OUTPUT" -a "$PERF_OUTPUT"!=" " ]; then
    perf stat -x, -a -e CPU_CLK_UNHALTED.THREAD,IDQ_UOPS_NOT_DELIVERED.CORE,UOPS_ISSUED.ANY,UOPS_RETIRED.RETIRE_SLOTS,INT_MISC.RECOVERY_CYCLES,CYCLE_ACTIVITY.STALLS_MEM_ANY,RESOURCE_STALLS.SB -o $PERF_OUTPUT \
    ../hashing -a $algo -E $epsl_r -F $epsl_s -U $univ -Q $bern -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -I $id -H $gp -[ $progress_step -] $merge_step -G $group -g $gap
  else
    perf stat -e CPU_CLK_UNHALTED.THREAD,IDQ_UOPS_NOT_DELIVERED.CORE,UOPS_ISSUED.ANY,UOPS_RETIRED.RETIRE_SLOTS,INT_MISC.RECOVERY_CYCLES,CYCLE_ACTIVITY.STALLS_MEM_ANY,RESOURCE_STALLS.SB \
    ../hashing -a $algo -E $epsl_r -F $epsl_s -U $univ -Q $bern -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -I $id -H $gp -[ $progress_step -] $merge_step -G $group -g $gap
  fi
  if [[ $? -eq 139 ]]; then echo "oops, sigsegv" exit -1; fi
}

function perfUtilBenchmarkRun() {
  #####native execution
  echo "==benchmark:$benchmark -a $algo -E $epsl_r -F $epsl_s -U $univ -Q $bern -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -I $id -H $gp -[ $progress_step -] $merge_step -G $group -g $gap -o $exp_dir/results/breakdown/perf_${gp}_${id}.txt > ${exp_dir}/results/breakdown/${phase}_${benchmark}_${algo}_profile_${gp}_${id}.txt =="
  #echo 3 >/proc/sys/vm/drop_caches
  echo tangxilin | sudo -S sysctl vm.drop_caches=3
  perf stat -I10 -x, -o $exp_dir/results/breakdown/perf_${gp}_${id}.csv -e cache-misses,cycles ../hashing -a $algo -E $epsl_r -F $epsl_s -U $univ -Q $bern -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -I $id -H $gp -[ $progress_step -] $merge_step -G $group -g $gap
  if [[ $? -eq 139 ]]; then echo "oops, sigsegv" exit -1; fi
}

function KimRun() {
  #####native execution
  echo "==KIM benchmark:$benchmark -a $algo -E $epsl_r -F $epsl_s -U $univ -Q $bern -t $ts -w $WINDOW_SIZE -e $STEP_SIZE -q $STEP_SIZE_S -l $INTERVAL -d $distrbution -z $skew -D $TS_DISTRIBUTION -Z $ZIPF_FACTOR -n $Threads -I $id -H $gp -W $FIXS -[ $progress_step -] $merge_step -G $group -P $DD -g $gap -o $exp_dir/results/breakdown/profile_${gp}_${id}.txt > ${exp_dir}/results/breakdown/${phase}_${benchmark}_${algo}_profile_${gp}_${id}.txt  =="
  #echo 3 >/proc/sys/vm/drop_caches
  echo tangxilin | sudo -S sysctl vm.drop_caches=3
  ../hashing -a $algo -E $epsl_r -F $epsl_s -U $univ -Q $bern -t $ts -w $WINDOW_SIZE -e $STEP_SIZE -q $STEP_SIZE_S -l $INTERVAL -d $distrbution -z $skew -D $TS_DISTRIBUTION -Z $ZIPF_FACTOR -n $Threads -I $id -H $gp -W $FIXS -[ $progress_step -] $merge_step -G $group -P $DD -g $gap -o $exp_dir/results/breakdown/profile_${gp}_${id}.txt > ${exp_dir}/results/breakdown/${phase}_${benchmark}_${algo}_profile_${gp}_${id}.txt  
  if [[ $? -eq 139 ]]; then echo "oops, sigsegv" exit -1; fi
}

function KimProfileRun() {
  #####native execution
  echo "==KIM benchmark:$benchmark -a $algo -E $epsl_r -F $epsl_s -U $univ -Q $bern -t $ts -w $WINDOW_SIZE -e $STEP_SIZE -q $STEP_SIZE_S -l $INTERVAL -d $distrbution -z $skew -D $TS_DISTRIBUTION -Z $ZIPF_FACTOR -n $Threads -I $id -H $gp -W $FIXS -[ $progress_step -] $merge_step -G $group -P $DD -g $gap =="
  #echo 3 >/proc/sys/vm/drop_caches
  echo tangxilin | sudo -S sysctl vm.drop_caches=3
  if [ ! -z "$PERF_CONF" -a "$PERF_CONF"!=" " ]; then
    ../hashing -a $algo -E $epsl_r -F $epsl_s -U $univ -Q $bern -t $ts -w $WINDOW_SIZE -e $STEP_SIZE -q $STEP_SIZE_S -l $INTERVAL -d $distrbution -z $skew -D $TS_DISTRIBUTION -Z $ZIPF_FACTOR -n $Threads -I $id -H $gp -W $FIXS -[ $progress_step -] $merge_step -G $group -P $DD -g $gap -o $exp_dir/results/breakdown/profile_${gp}_${id}.txt -p $PERF_CONF > ${exp_dir}/results/breakdown/${phase}_${benchmark}_${algo}_profile_${gp}_${id}.txt
  else
    ../hashing -a $algo -E $epsl_r -F $epsl_s -U $univ -Q $bern -t $ts -w $WINDOW_SIZE -e $STEP_SIZE -q $STEP_SIZE_S -l $INTERVAL -d $distrbution -z $skew -D $TS_DISTRIBUTION -Z $ZIPF_FACTOR -n $Threads -I $id -H $gp -W $FIXS -[ $progress_step -] $merge_step -G $group -P $DD -g $gap -o $exp_dir/results/breakdown/profile_${gp}_${id}.txt
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
  # PARTITION_ONLY
  # compile
  # echo "PARTITION_ONLY"
  # phase='PARTITION_ONLY'
  # benchmarkRun

  # PARTITION_BUILD_SORT
  # compile
  # echo "PARTITION_BUILD_SORT"
  # phase='PARTITION_BUILD_SORT'
  # benchmarkRun

  # PARTITION_BUILD_SORT_MERGE
  # compile
  # echo "PARTITION_BUILD_SORT_MERGE"
  # phase='PARTITION_BUILD_SORT_MERGE'
  # benchmarkRun

  # PARTITION_BUILD_SORT_MERGE_JOIN
  # compile
  # echo "PARTITION_BUILD_SORT_MERGE_JOIN"
  # phase='PARTITION_BUILD_SORT_MERGE_JOIN'
  # benchmarkRun

  ALL_ON
  compile
  echo "ALL_ON"
  phase='ALL_ON'
  benchmarkRun
}

function SHJBENCHRUN() {
  # PARTITION_ONLY
  # compile
  # echo "PARTITION_ONLY"
  # phase='PARTITION_ONLY'
  # benchmarkRun

  # PARTITION_BUILD_SORT
  # compile
  # echo "PARTITION_BUILD_SORT"
  # phase='PARTITION_BUILD_SORT'
  # benchmarkRun

  # PARTITION_BUILD_SORT_MERGE_JOIN
  # compile
  # echo "PARTITION_BUILD_SORT_MERGE_JOIN"
  # phase='PARTITION_BUILD_SORT_MERGE_JOIN'
  # benchmarkRun

  ALL_ON
  compile
  echo "ALL_ON"
  phase='ALL_ON'
  benchmarkRun
}

function FULLKIMRUN() {
  # PARTITION_ONLY
  # compile
  # echo "PARTITION_ONLY"
  # phase='PARTITION_ONLY'
  # KimRun

  # PARTITION_BUILD_SORT
  # compile
  # echo "PARTITION_BUILD_SORT"
  # phase='PARTITION_BUILD_SORT'
  # KimRun

  # PARTITION_BUILD_SORT_MERGE
  # compile
  # echo "PARTITION_BUILD_SORT_MERGE"
  # phase='PARTITION_BUILD_SORT_MERGE'
  # KimRun

  # PARTITION_BUILD_SORT_MERGE_JOIN
  # compile
  # echo "PARTITION_BUILD_SORT_MERGE_JOIN"
  # phase='PARTITION_BUILD_SORT_MERGE_JOIN'
  # KimRun

  ALL_ON
  compile
  echo "ALL_ON"
  phase='ALL_ON'
  KimRun
}

function SHJKIMRUN() {
  # PARTITION_ONLY
  # phase='PARTITION_ONLY'
  # compile
  # KimRun

  # PARTITION_BUILD_SORT
  # phase='PARTITION_BUILD_SORT'
  # compile
  # KimRun

  # PARTITION_BUILD_SORT_MERGE_JOIN
  # phase='PARTITION_BUILD_SORT_MERGE_JOIN'
  # compile
  # KimRun

  ALL_ON
  compile
  echo "ALL_ON"
  phase='ALL_ON'
  KimRun
}

function RUNALL() {
  if [ $profile_breakdown == 1 ]; then
    if [ "$algo" == SHJ_JM_P ] || [ "$algo" == SHJ_JM_NP ] || [ "$algo" == SHJ_JBCR_P ]|| [ "$algo" == SHJ_JBCR_NP ] ; then
      SHJBENCHRUN
    else
      if [ "$algo" == PMJ_JM_P ] || [ "$algo" == PMJ_JM_NP ] || [ "$algo" == PMJ_JBCR_P ] || [ "$algo" == PMJ_JBCR_NP ];  then
        FULLBENCHRUN
      else
        compile
        benchmarkRun
      fi
    fi
  else
    if [ "$algo" == SHJ_JM_P ] || [ "$algo" == SHJ_JM_NP ] || [ "$algo" == SHJ_JBCR_P ]|| [ "$algo" == SHJ_JBCR_NP ] ; then
      ALL_ON
      compile
      benchmarkRun
    else
      if [ "$algo" == PMJ_JM_P ] || [ "$algo" == PMJ_JM_NP ] || [ "$algo" == PMJ_JBCR_P ] || [ "$algo" == PMJ_JBCR_NP ];  then
        ALL_ON
        compile
        benchmarkRun
      else
        compile
        benchmarkRun
      fi
    fi
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
        compile
        KimRun
      fi
    fi
  else
    ALL_ON
    compile
    KimRun
    if [ $algo == SHJ_JM_P ] || [ $algo == SHJ_JM_NP ] || [ $algo == SHJ_JBCR_P ] || [ $algo == SHJ_JBCR_NP ] || [ $algo == SHJ_HS_P ]|| [ $algo == SHJ_HS_NP ]; then
      ALL_ON
      compile
      KimRun
    else
      if [ $algo == PMJ_JM_P ] || [ $algo == PMJ_JM_NP ] || [ $algo == PMJ_JBCR_P ] || [ $algo == PMJ_JBCR_NP ]; then
        ALL_ON
        compile
        KimRun
      else
        compile
        KimRun
      fi
    fi
  fi
}

# function SetStockParameters() { #matches: 15598112. #inputs= 60527 + 77227
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

function SetStockParameters() { #matches: 1635926294. #inputs= 1013800 + 1034443
  ts=1 # stream case
  TOTAL_JOIN=1635926294
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
  TOTAL_JOIN=87856849382
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
  TOTAL_JOIN=10000000
  RSIZE=1000
  SSIZE=10000000
  RPATH=$exp_dir/datasets/YSB/campaigns_id.txt
  SPATH=$exp_dir/datasets/YSB/ad_events.txt
  # epsl_r=1
  RKEY=0
  SKEY=0
  RTS=0
  STS=1
  gap=10000
}

function SetDEBSParameters() { #matches: 251033140 #inputs= 1000000 + 1000000
  ts=1 # stream case
  WINDOW_SIZE=0
  TOTAL_JOIN=251033140
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


# sed -i -e "s/#define NO_SET_PR/#define SET_PR/g" ../helper/localjoiner.h
# sed -i -e "s/#define NO_SAMPLE_ON/#define SAMPLE_ON/g" ../helper/localjoiner.h
# sed -i -e "s/#define NO_ALWAYS_PROBE/#define ALWAYS_PROBE/g" ../helper/localjoiner.h
# sed -i -e "s/#define NO_PRESAMPLE/#define PRESAMPLE/g" ../helper/localjoiner.h
# sed -i -e "s/#define NO_AVX_RAND/#define AVX_RAND/g" ../helper/localjoiner.h
# sed -i -e "s/#define NO_MEM_LIM/#define MEM_LIM/g" ../helper/localjoiner.h



sed -i -e "s/#define SET_PR/#define NO_SET_PR/g" ../helper/localjoiner.h
sed -i -e "s/#define SAMPLE_ON/#define NO_SAMPLE_ON/g" ../helper/localjoiner.h
sed -i -e "s/#define ALWAYS_PROBE/#define NO_ALWAYS_PROBE/g" ../helper/localjoiner.h
sed -i -e "s/#define PRESAMPLE/#define NO_PRESAMPLE/g" ../helper/localjoiner.h
sed -i -e "s/#define AVX_RAND/#define NO_AVX_RAND/g" ../helper/localjoiner.h
sed -i -e "s/#define MEM_LIM/#define NO_MEM_LIM/g" ../helper/localjoiner.h
sed -i -e "s/#define MARTERIAL_SAMPLE/#define NO_MARTERIAL_SAMPLE/g" ../benchmark.cpp


function IF_ALWAYS_PROBE()
{
  if [ $set_always_probe == 0 ]; then
    sed -i -e "s/#define NO_ALWAYS_PROBE/#define ALWAYS_PROBE/g" ../helper/localjoiner.h
  fi
  if [ $set_always_probe == 1 ]; then
    sed -i -e "s/#define ALWAYS_PROBE/#define NO_ALWAYS_PROBE/g" ../helper/localjoiner.h
  fi
}
set_always_probe=1
mem_lim=1
set_pr=1
set_avx_rand=1


function IF_SET_PR()
{
  if [ $set_pr == 0 ]; then
    sed -i -e "s/#define PRESAMPLE/#define NO_PRESAMPLE/g" ../helper/localjoiner.h
  fi
  if [ $set_pr == 1 ]; then
    sed -i -e "s/#define NO_PRESAMPLE/#define PRESAMPLE/g" ../helper/localjoiner.h
  fi
}

function IF_AVX_RAND() {
  if [ $set_avx_rand == 0 ]; then
    sed -i -e "s/#define NO_AVX_RAND/#define AVX_RAND/g" ../helper/localjoiner.h
  fi
  if [ $set_avx_rand == 1 ]; then
    sed -i -e "s/#define AVX_RAND/#define NO_AVX_RAND/g" ../helper/localjoiner.h
  fi
}

function APPROXIMATE_OFF() {
  sed -i -e "s/#define SET_PR/#define NO_SET_PR/g" ../helper/localjoiner.h
  sed -i -e "s/#define SAMPLE_ON/#define NO_SAMPLE_ON/g" ../helper/localjoiner.h
  sed -i -e "s/#define ALWAYS_PROBE/#define NO_ALWAYS_PROBE/g" ../helper/localjoiner.h
  sed -i -e "s/#define PRESAMPLE/#define NO_PRESAMPLE/g" ../helper/localjoiner.h
  sed -i -e "s/#define AVX_RAND/#define NO_AVX_RAND/g" ../helper/localjoiner.h
  sed -i -e "s/#define MEM_LIM/#define NO_MEM_LIM/g" ../helper/localjoiner.h
  sed -i -e "s/#define MARTERIAL_SAMPLE/#define NO_MARTERIAL_SAMPLE/g" ../benchmark.cpp
  sed -i -e "s/#define PROBEHASH/#define NO_PROBEHASH/g" ../helper/localjoiner.h

}

function SAMPLE_OFF() {
  sed -i -e "s/#define SET_PR/#define NO_SET_PR/g" ../helper/localjoiner.h
  sed -i -e "s/#define SAMPLE_ON/#define NO_SAMPLE_ON/g" ../helper/localjoiner.h
  sed -i -e "s/#define ALWAYS_PROBE/#define NO_ALWAYS_PROBE/g" ../helper/localjoiner.h
  sed -i -e "s/#define PRESAMPLE/#define NO_PRESAMPLE/g" ../helper/localjoiner.h
  sed -i -e "s/#define AVX_RAND/#define NO_AVX_RAND/g" ../helper/localjoiner.h
  sed -i -e "s/#define MEM_LIM/#define NO_MEM_LIM/g" ../helper/localjoiner.h
  sed -i -e "s/#define MARTERIAL_SAMPLE/#define NO_MARTERIAL_SAMPLE/g" ../benchmark.cpp
  sed -i -e "s/#define PROBEHASH/#define NO_PROBEHASH/g" ../helper/localjoiner.h
}

function EAGER_SAMPLE() {
  if [ $eg_smp == 0 ]; then
    sed -i -e "s/#define NO_SAMPLE_ON/#define SAMPLE_ON/g" ../helper/localjoiner.h
    sed -i -e "s/#define NO_PRESAMPLE/#define PRESAMPLE/g" ../helper/localjoiner.h
  fi
  if [ $eg_smp == 1 ]; then
    sed -i -e "s/#define SAMPLE_ON/#define NO_SAMPLE_ON/g" ../helper/localjoiner.h
    sed -i -e "s/#define PRESAMPLE/#define NO_PRESAMPLE/g" ../helper/localjoiner.h
  fi
}

function LAZY_SAMPLE() {
  if [ $lz_smp == 0 ]; then
    SAMPLE_OFF
    sed -i -e "s/#define NO_MARTERIAL_SAMPLE/#define MARTERIAL_SAMPLE/g" ../benchmark.cpp
  fi
  if [ $lz_smp == 1 ]; then
    sed -i -e "s/#define MARTERIAL_SAMPLE/#define NO_MARTERIAL_SAMPLE/g" ../benchmark.cpp
  fi
}

function MEM_LIM()
{
  if [ $mem_lim == 0 ]; then
    sed -i -e "s/#define NO_MEM_LIM/#define MEM_LIM/g" ../helper/localjoiner.h
  fi
  if [ $mem_lim == 1 ]; then
    sed -i -e "s/#define MEM_LIM/#define NO_MEM_LIM/g" ../helper/localjoiner.h
  fi
}

function PROBEHASH() {
  if [ $prb_hsh == 0 ]; then
    SAMPLE_OFF
    sed -i -e "s/#define NO_PROBEHASH/#define PROBEHASH/g" ../helper/localjoiner.h
  fi
  if [ $prb_hsh == 1 ]; then
    sed -i -e "s/#define PROBEHASH/#define NO_PROBEHASH/g" ../helper/localjoiner.h
  fi
}

function RESERVOIR_STRATA() {
  if [ $res_strata == 0 ]; then
    SAMPLE_OFF
    sed -i -e "s/#define NO_RESERVOIR_STRATA/#define RESERVOIR_STRATA/g" ../helper/localjoiner.h
  fi
  if [ $res_strata == 1 ]; then
    sed -i -e "s/#define RESERVOIR_STRATA/#define NO_RESERVOIR_STRATA/g" ../helper/localjoiner.h
  fi
}

# function SET_RESERVOIR_SIZE() {
#   sed -i -e "s/#define RESERVOIR_SIZE [[:alnum:]]*/#define RESERVOIR_SIZE $reservoir_size/g" ../joins/npj_types.h
# }

function SET_GAP_RESERV() {
  reservoir_size=`python3 reservoir_size.py -e $epsl_r -r $RSIZE -s $SSIZE -n $Threads`
  gap=`python3 gap_size.py -e $epsl_r -m $TOTAL_JOIN`
  echo "\n\n\n\n\n\n"
  echo $reservoir_size
  echo $gap
  sed -i -e "s/#define RESERVOIR_SIZE [[:alnum:]]*/#define RESERVOIR_SIZE $reservoir_size/g" ../joins/npj_types.h
}

function SET_RAND_BUFFER_SIZE() {
  sed -i -e "s/#define RANDOM_BUFFER_SIZE [[:alnum:]]*/#define RANDOM_BUFFER_SIZE $rand_buffer_size/g" ../joins/npj_types.h
}

function SAMPLE_WITH_PARA() {
  EAGER_SAMPLE
  LAZY_SAMPLE
  MEM_LIM
  PROBEHASH
  RESERVOIR_STRATA

  IF_SET_PR
  IF_AVX_RAND
  IF_ALWAYS_PROBE
}

function RUN_SHJ (){
  for profile_breakdown in 1; do
    compile=1
    for benchmark in "Stock" "Rovio" "YSB" "DEBS"; do # "Stock" "Rovio" "YSB" "DEBS"
      # for algo in NPO PRO SHJ_JM_NP SHJ_JBCR_NP PMJ_JM_NP PMJ_JBCR_NP; do # NPO PRO SHJ_JM_NP SHJ_JBCR_NP PMJ_JM_NP PMJ_JBCR_NP
      for algo in SHJ_JM_NP SHJ_JBCR_NP; do # NPO PRO SHJ_JM_NP SHJ_JBCR_NP PMJ_JM_NP PMJ_JBCR_NP
        case "$benchmark" in
        "Stock")
          id=38
          ResetParameters
          SetStockParameters
          SET_GAP_RESERV
          SET_RAND_BUFFER_SIZE
          RUNALL
          ;;
        "Rovio") #matches:
          id=39
          ResetParameters
          SetRovioParameters
          SET_GAP_RESERV
          SET_RAND_BUFFER_SIZE
          RUNALL
          ;;
        "DEBS")
          id=41
          ResetParameters
          SetDEBSParameters
          SET_GAP_RESERV
          SET_RAND_BUFFER_SIZE
          RUNALL
          ;;
        esac
      done
    done
  done
}

function RUN_PMJ (){
  for profile_breakdown in 1; do
    compile=1
    for benchmark in "Stock" "Rovio" "YSB" "DEBS"; do # "Stock" "Rovio" "YSB" "DEBS"
      # for algo in NPO PRO SHJ_JM_NP SHJ_JBCR_NP PMJ_JM_NP PMJ_JBCR_NP; do # NPO PRO SHJ_JM_NP SHJ_JBCR_NP PMJ_JM_NP PMJ_JBCR_NP
      for algo in PMJ_JM_NP PMJ_JBCR_NP; do # NPO PRO SHJ_JM_NP SHJ_JBCR_NP SHJ_JM_NP SHJ_JBCR_NP
        case "$benchmark" in
        "Stock")
          id=38
          ResetParameters
          SetStockParameters
          SET_GAP_RESERV
          SET_RAND_BUFFER_SIZE
          RUNALL
          ;;
        "Rovio") #matches:
          id=39
          ResetParameters
          SetRovioParameters
          SET_GAP_RESERV
          SET_RAND_BUFFER_SIZE
          RUNALL
          ;;
        "DEBS")
          id=41
          ResetParameters
          SetDEBSParameters
          SET_GAP_RESERV
          SET_RAND_BUFFER_SIZE
          RUNALL
          ;;
        esac
      done
    done
  done
}

function RUN_LAZY_HASH() {
  for profile_breakdown in 1; do
    compile=1
    for benchmark in "Stock" "Rovio" "YSB" "DEBS"; do # "Stock" "Rovio" "YSB" "DEBS"
      # for algo in NPO PRO SHJ_JM_NP SHJ_JBCR_NP PMJ_JM_NP PMJ_JBCR_NP; do # NPO PRO SHJ_JM_NP SHJ_JBCR_NP PMJ_JM_NP PMJ_JBCR_NP
      for algo in NPO PRO; do # NPO PRO SHJ_JM_NP SHJ_JBCR_NP PMJ_JM_NP PMJ_JBCR_NP
        case "$benchmark" in
        "Stock")
          id=38
          ResetParameters
          SetStockParameters
          SET_GAP_RESERV
          SET_RAND_BUFFER_SIZE
          RUNALL
          ;;
        "Rovio") #matches:
          id=39
          ResetParameters
          SetRovioParameters
          SET_GAP_RESERV
          SET_RAND_BUFFER_SIZE
          RUNALL
          ;;
        "DEBS")
          id=41
          ResetParameters
          SetDEBSParameters
          SET_GAP_RESERV
          SET_RAND_BUFFER_SIZE
          RUNALL
          ;;
        esac
      done
    done
  done
}


declare -a arr

# arr=( '1 0.1 1 0.1' '0.1 1 0.1 1' '0.3 0.333 0.3 0.333' '0.333 0.3 0.333 0.3' '0.667 0.15 0.667 0.15' '0.15 0.667 0.15 0.667' )
arr=( '0.1 0.1 1 0.1' '0.1 0.1 1 0.1' '0.1 0.1 1 0.1' '0.1 0.1 0.3 0.333' '0.1 0.1 0.3 0.333' '0.1 0.1 0.3 0.333' '0.1 0.1 0.667 0.15' '0.1 0.1 0.667 0.15' '0.1 0.1 0.667 0.15' )


# for set_always_probe in 0 1; do
# IF_ALWAYS_PROBE
# for mem_lim in 0 1; do
# MEM_LIM
# for rand_pair in "${arr[@]}"; do
# eval real_pair=(${rand_pair})
# epsl_r=${real_pair[0]};
# epsl_s=${real_pair[1]};
# univ=${real_pair[2]};
# bern=${real_pair[3]};
# let "gp++"


# prb_hsh=0
# PROBEHASH


phase='ALL_ON'


###################  

epsl_r=1
epsl_s=1
univ=1
bern=1
set_pr=1
rand_buffer_size=1000

APPROXIMATE_OFF

thread_test=1

if [ $APP_BENCH == 1 ]; then
  NORMAL
  benchmark=Rovio
  algo=SHJ_JBCR_NP
  # algo=SHJ_JM_NP
  compile=1
  id=39
  SAMPLE_OFF
  # NO HARDWARE NO SAMPLE
  gp='0-0'
  ResetParameters
  Threads=1
  SetRovioParameters
  RUNALL

  # MULTICORE NO SAMPLE
  gp='0-1'
  SAMPLE_OFF
  ResetParameters
  Threads=8
  SetRovioParameters
  RUNALL
  # NO MULTICORE SAMPLE 0.9

  gp='0-2'
  epsl_r=0.9
  epsl_s=0.9

  eg_smp=0
  lz_smp=1
  mem_lim=1
  prb_hsh=1
  res_strata=1

  set_avx_rand=0
  set_always_probe=1

  SAMPLE_WITH_PARA
  ResetParameters
  Threads=1
  SetRovioParameters
  RUNALL
  # NO MULTICORE SAMPLE 0.5
  gp='0-3'
  
  epsl_r=0.5
  epsl_s=0.5

  eg_smp=0
  lz_smp=1
  mem_lim=1
  prb_hsh=1
  res_strata=1

  set_avx_rand=0
  set_always_probe=1
  SAMPLE_WITH_PARA
  ResetParameters
  Threads=1
  SetRovioParameters
  RUNALL
  # NO MULTICORE SAMPLE 0.1
  gp='0-4'
  
  epsl_r=0.1
  epsl_s=0.1

  eg_smp=0
  lz_smp=1
  mem_lim=1
  prb_hsh=1
  res_strata=1

  set_avx_rand=0
  set_always_probe=1
  SAMPLE_WITH_PARA
  ResetParameters
  Threads=1
  SetRovioParameters
  RUNALL
  # MULTICORE SAMPLE 0.9
  gp='0-5'
  
  epsl_r=0.9
  epsl_s=0.9

  eg_smp=0
  lz_smp=1
  mem_lim=1
  prb_hsh=1
  res_strata=1

  set_avx_rand=0
  set_always_probe=1
  SAMPLE_WITH_PARA
  ResetParameters
  Threads=8
  SetRovioParameters
  RUNALL
  # MULTICORE SAMPLE 0.5
  gp='0-6'
  
  epsl_r=0.5
  epsl_s=0.5

  eg_smp=0
  lz_smp=1
  mem_lim=1
  prb_hsh=1
  res_strata=1

  set_avx_rand=0
  set_always_probe=1
  SAMPLE_WITH_PARA
  Threads=8
  SetRovioParameters
  RUNALL
  # MULTICORE SAMPLE 0.1
  gp='0-7'
  
  epsl_r=0.1
  epsl_s=0.1

  eg_smp=0
  lz_smp=1
  mem_lim=1
  prb_hsh=1
  res_strata=1

  set_avx_rand=0
  set_always_probe=1
  SAMPLE_WITH_PARA
  ResetParameters
  Threads=8
  SetRovioParameters
  RUNALL
fi

exit


<<COMMENT
########### NON-SAMPLE BASELINE

gp=0

APPROXIMATE_OFF

if [ $APP_BENCH == 1 ]; then
  NORMAL
  RUN_LAZY_HASH
  RUN_SHJ
  RUN_PMJ
fi

# COMMENT


######### NON-MEM-LIMITED SAMPLE

gp=1

epsl_r=0.6
epsl_s=0.6

eg_smp=1
lz_smp=0
mem_lim=1
prb_hsh=1
res_strata=1

set_avx_rand=0
set_always_probe=1

SAMPLE_WITH_PARA

if [ $APP_BENCH == 1 ]; then
  NORMAL
  #compile depends on whether we want to profile.
  RUN_LAZY_HASH
fi

# exit
######### NON-MEM-LIMITED SAMPLE

gp=1

epsl_r=0.5
epsl_s=0.5

eg_smp=0
lz_smp=1
mem_lim=1
prb_hsh=1
res_strata=1

set_avx_rand=0
set_always_probe=1

SAMPLE_WITH_PARA

if [ $APP_BENCH == 1 ]; then
  NORMAL
  #compile depends on whether we want to profile.
  RUN_SHJ
  RUN_PMJ
fi

########## PROBHASH

gp=2

epsl_r=0.5
epsl_s=0.5

eg_smp=1
lz_smp=1
mem_lim=1
prb_hsh=0
res_strata=1

set_avx_rand=0
set_always_probe=1

SAMPLE_WITH_PARA

if [ $APP_BENCH == 1 ]; then
  NORMAL
  #compile depends on whether we want to profile.
  RUN_SHJ
fi

######### SRAJ

gp=3

epsl_r=0.5
epsl_s=0.5

eg_smp=1
lz_smp=1
mem_lim=0
prb_hsh=1
res_strata=1

set_avx_rand=0
set_always_probe=1

SAMPLE_WITH_PARA

if [ $APP_BENCH == 1 ]; then
  NORMAL
  #compile depends on whether we want to profile.
  RUN_SHJ
fi

# COMMENT
######### PROBE_ALL

gp=4

epsl_r=0.5
epsl_s=0.5

eg_smp=0
lz_smp=1
mem_lim=1
prb_hsh=1
res_strata=1

set_avx_rand=0
set_always_probe=0

SAMPLE_WITH_PARA

if [ $APP_BENCH == 1 ]; then
  NORMAL
  #compile depends on whether we want to profile.
  RUN_SHJ
fi

######### SRAJ PROBEALL

gp=5

epsl_r=0.5
epsl_s=0.5

eg_smp=1
lz_smp=1
mem_lim=0
prb_hsh=1
res_strata=1

set_avx_rand=0
set_always_probe=0

SAMPLE_WITH_PARA

if [ $APP_BENCH == 1 ]; then
  NORMAL
  #compile depends on whether we want to profile.
  RUN_SHJ
fi

# exit

#########################     impact of epsilon on throughput 

# lazy epsilon

gp=10

eg_smp=1
lz_smp=0
mem_lim=1
prb_hsh=1
res_strata=1

set_avx_rand=0
set_always_probe=1

SAMPLE_WITH_PARA

for epsl in 0.9 0.8 0.7 0.6 0.5 0.4 0.3 0.2 0.1 0.05 0.01 0.005 0.001; do
  epsl_r=$epsl
  epsl_s=$epsl
  NORMAL
  #compile depends on whether we want to profile.
  RUN_LAZY_HASH
  let "gp++"
done

# eager epsilon

gp=10

eg_smp=0
lz_smp=1
mem_lim=1
prb_hsh=1
res_strata=1

set_avx_rand=0
set_always_probe=1

SAMPLE_WITH_PARA

for epsl in 0.9 0.8 0.7 0.6 0.5 0.4 0.3 0.2 0.1 0.05 0.01 0.005 0.001; do
  epsl_r=$epsl
  epsl_s=$epsl
  NORMAL
  #compile depends on whether we want to profile.
  RUN_SHJ
  RUN_PMJ
  let "gp++"
done

# reservoir epsilon

gp=30

eg_smp=1
lz_smp=1
mem_lim=0
prb_hsh=1
res_strata=1

set_avx_rand=0
set_always_probe=1

SAMPLE_WITH_PARA

for epsl in 0.9 0.8 0.7 0.6 0.5 0.4 0.3 0.2 0.1 0.05 0.01 0.005 0.001; do
  epsl_r=$epsl
  epsl_s=$epsl
  NORMAL
  #compile depends on whether we want to profile.
  RUN_SHJ
  RUN_PMJ
  let "gp++"
done


# probehash

gp=1090

eg_smp=1
lz_smp=1
mem_lim=1
prb_hsh=0
res_strata=1

set_avx_rand=0
set_always_probe=1

SAMPLE_WITH_PARA

for epsl in 0.9 0.8 0.7 0.6 0.5 0.4 0.3 0.2 0.1 0.05 0.01 0.005 0.001; do
  epsl_r=$epsl
  epsl_s=$epsl
  NORMAL
  #compile depends on whether we want to profile.
  RUN_SHJ
  RUN_PMJ
  let "gp++"
done

# eager epsilon probeall

gp=50

eg_smp=0
lz_smp=1
mem_lim=1
prb_hsh=1
res_strata=1

set_avx_rand=0
set_always_probe=0

SAMPLE_WITH_PARA

for epsl in 0.9 0.8 0.7 0.6 0.5 0.4 0.3 0.2 0.1 0.05 0.01 0.005 0.001; do
  epsl_r=$epsl
  epsl_s=$epsl
  NORMAL
  #compile depends on whether we want to profile.
  RUN_SHJ
  RUN_PMJ
  let "gp++"
done


# reservoir probeall

gp=70

eg_smp=1
lz_smp=1
mem_lim=0
prb_hsh=1
res_strata=1

set_avx_rand=0
set_always_probe=0

SAMPLE_WITH_PARA

for epsl in 0.9 0.8 0.7 0.6 0.5 0.4 0.3 0.2 0.1 0.05 0.01 0.005 0.001; do
  epsl_r=$epsl
  epsl_s=$epsl
  NORMAL
  #compile depends on whether we want to profile.
  RUN_SHJ
  RUN_PMJ
  let "gp++"
done

# COMMENT
##### variance


gp=100

epsl_r=0.5
epsl_s=0.5

eg_smp=1
lz_smp=0
mem_lim=1
prb_hsh=1
res_strata=1

set_avx_rand=0
set_always_probe=1

SAMPLE_WITH_PARA

for iii in {1..20}; do
  NORMAL
  #compile depends on whether we want to profile.
  RUN_LAZY_HASH
  let "gp++"
done
# COMMENT
# exit

######          eager

gp=100

epsl_r=0.5
epsl_s=0.5

eg_smp=0
lz_smp=1
mem_lim=1
prb_hsh=1
res_strata=1

set_avx_rand=0
set_always_probe=1

SAMPLE_WITH_PARA

for iii in {1..20}; do
  NORMAL
  #compile depends on whether we want to profile.
  RUN_SHJ
  RUN_PMJ
  let "gp++"
done

########## PROBHASH

gp=200

epsl_r=0.5
epsl_s=0.5

eg_smp=1
lz_smp=1
mem_lim=1
prb_hsh=0
res_strata=1

set_avx_rand=0
set_always_probe=1

SAMPLE_WITH_PARA

for iii in {1..20}; do
  NORMAL
  #compile depends on whether we want to profile.
  RUN_SHJ
  let "gp++"
done


######### SRAJ

gp=300

epsl_r=0.5
epsl_s=0.5

eg_smp=1
lz_smp=1
mem_lim=0
prb_hsh=1
res_strata=1

set_avx_rand=0
set_always_probe=1

SAMPLE_WITH_PARA

for iii in {1..20}; do
  NORMAL
  #compile depends on whether we want to profile.
  RUN_SHJ
  let "gp++"
done


######### PROBE_ALL

gp=400


epsl_r=0.5
epsl_s=0.5

eg_smp=0
lz_smp=1
mem_lim=1
prb_hsh=1
res_strata=1

set_avx_rand=0
set_always_probe=0

SAMPLE_WITH_PARA

for iii in {1..20}; do
  NORMAL
  #compile depends on whether we want to profile.
  RUN_SHJ
  let "gp++"
done

######### SRAJ PROBEALL

gp=500


epsl_r=0.5
epsl_s=0.5

eg_smp=1
lz_smp=1
mem_lim=0
prb_hsh=1
res_strata=1

set_avx_rand=0
set_always_probe=0

SAMPLE_WITH_PARA

for iii in {1..20}; do
  NORMAL
  #compile depends on whether we want to profile.
  RUN_SHJ
  let "gp++"
done

# COMMENT


# lazy trade off

benchmark="Rovio"

gp=21000

eg_smp=1
lz_smp=0
mem_lim=1
prb_hsh=1
res_strata=1

set_avx_rand=0
set_always_probe=1

SAMPLE_WITH_PARA

for epsl in 0.9 0.8 0.7 0.6 0.5 0.4 0.3 0.2 0.1 0.05; do
  epsl_r=$epsl
  epsl_s=$epsl
  for iii in {1..50}; do
    NORMAL
    RUN_LAZY_HASH
    let "gp++"
  done
done

# eager trade off

gp=21000

eg_smp=0
lz_smp=1
mem_lim=1
prb_hsh=1
res_strata=1

set_avx_rand=0
set_always_probe=1

SAMPLE_WITH_PARA

for epsl in 0.9 0.8 0.7 0.6 0.5 0.4 0.3 0.2 0.1 0.05; do
  epsl_r=$epsl
  epsl_s=$epsl
  for iii in {1..50}; do
    NORMAL
    RUN_SHJ
    RUN_PMJ
    let "gp++"
  done
done

# reservoir trade off

# COMMENT

gp=24000

eg_smp=1
lz_smp=1
mem_lim=0
prb_hsh=1
res_strata=1

set_avx_rand=0
set_always_probe=1

SAMPLE_WITH_PARA

for epsl in 0.9 0.8 0.7 0.6 0.5 0.4 0.3 0.2 0.1 0.05; do
  epsl_r=$epsl
  epsl_s=$epsl
  for iii in {1..50}; do
    NORMAL
    RUN_SHJ
    let "gp++"
  done
done

<<COMMENT
# eager trade off probeall

gp=26000

eg_smp=0
lz_smp=1
mem_lim=1
prb_hsh=1
res_strata=1

set_avx_rand=0
set_always_probe=0

SAMPLE_WITH_PARA

for epsl in 0.9 0.8 0.7 0.6 0.5 0.4 0.3 0.2 0.1 0.05; do
  epsl_r=$epsl
  epsl_s=$epsl
  for iii in {1..50}; do
    NORMAL
    RUN_SHJ
    let "gp++"
  done
done

# COMMENT
# reservoir probeall

gp=28000

eg_smp=1
lz_smp=1
mem_lim=0
prb_hsh=1
res_strata=1

set_avx_rand=0
set_always_probe=0

SAMPLE_WITH_PARA

for epsl in 0.9 0.8 0.7 0.6 0.5 0.4 0.3 0.2 0.1 0.05; do
  epsl_r=$epsl
  epsl_s=$epsl
  for iii in {1..50}; do
    NORMAL
    RUN_SHJ
    let "gp++"
  done
done

<<COMMENT
# exit

COMMENT


thread_test=1


# PRIORI PROBHASH
# <<COMMENT
gp=30000

eg_smp=1
lz_smp=1
mem_lim=1
prb_hsh=0
res_strata=1

set_avx_rand=0
set_always_probe=1

SAMPLE_WITH_PARA

for epsl in 0.9 0.8 0.7 0.6 0.5 0.4 0.3 0.2 0.1 0.05; do
  epsl_r=$epsl
  epsl_s=$epsl
  for iii in {1..50}; do
    NORMAL
    RUN_SHJ
    let "gp++"
  done
done

# COMMENT
# StreamApprox

gp=32000

eg_smp=1
lz_smp=1
mem_lim=1
prb_hsh=1
res_strata=0

set_avx_rand=0
set_always_probe=1

SAMPLE_WITH_PARA

for epsl in 0.9 0.8 0.7 0.6 0.5 0.4 0.3 0.2 0.1 0.05; do
  epsl_r=$epsl
  epsl_s=$epsl
  for iii in {1..50}; do
    NORMAL
    RUN_SHJ
    let "gp++"
  done
done


exit

# lazy AVX

thread_test=1


epsl_r=0.5
epsl_s=0.5

gp=3100

eg_smp=1
lz_smp=0
mem_lim=1
prb_hsh=1
res_strata=1

set_avx_rand=0
set_always_probe=1

SAMPLE_WITH_PARA

for rand_buffer_size in 10 33 66 100 333 666 1000 3333 6666 10000 33333 66666 100000; do
  for iii in {1..2}; do
    NORMAL
    #compile depends on whether we want to profile.
    RUN_LAZY_HASH
    let "gp++"
  done
done

# eager AVX

gp=3100

eg_smp=0
lz_smp=1
mem_lim=1
prb_hsh=1
res_strata=1

set_avx_rand=0
set_always_probe=1

SAMPLE_WITH_PARA

for rand_buffer_size in 10 33 66 100 333 666 1000 3333 6666 10000 33333 66666 100000; do
  for iii in {1..2}; do
    NORMAL
    #compile depends on whether we want to profile.
    RUN_SHJ
    RUN_PMJ
    let "gp++"
  done
done

# reservoir AVX

gp=3200

eg_smp=1
lz_smp=1
mem_lim=0
prb_hsh=1
res_strata=1

set_avx_rand=0
set_always_probe=1

SAMPLE_WITH_PARA

for rand_buffer_size in 10 33 66 100 333 666 1000 3333 6666 10000 33333 66666 100000; do
  for iii in {1..2}; do
    NORMAL
    #compile depends on whether we want to profile.
    RUN_SHJ
    let "gp++"
  done
done


# probehash

gp=3900

eg_smp=1
lz_smp=1
mem_lim=1
prb_hsh=0
res_strata=1

set_avx_rand=0
set_always_probe=1

SAMPLE_WITH_PARA

for rand_buffer_size in 10 33 66 100 333 666 1000 3333 6666 10000 33333 66666 100000; do
  for iii in {1..2}; do
  NORMAL
  #compile depends on whether we want to profile.
  RUN_SHJ
  let "gp++"
  done
done

# COMMENT

# eager probeall AVX

gp=3300

eg_smp=0
lz_smp=1
mem_lim=1
prb_hsh=1
res_strata=1

set_avx_rand=0
set_always_probe=0

SAMPLE_WITH_PARA

for rand_buffer_size in 10 33 66 100 333 666 1000 3333 6666 10000 33333 66666 100000; do
  for iii in {1..2}; do
    NORMAL
    #compile depends on whether we want to profile.
    RUN_SHJ
    let "gp++"
  done
done


# reservoir probeall AVX

gp=3400

eg_smp=1
lz_smp=1
mem_lim=0
prb_hsh=1
res_strata=1

set_avx_rand=0
set_always_probe=0

SAMPLE_WITH_PARA

for rand_buffer_size in 10 33 66 100 333 666 1000 3333 6666 10000 33333 66666 100000; do
  for iii in {1..2}; do
    NORMAL
    #compile depends on whether we want to profile.
    RUN_SHJ
    let "gp++"
  done
done

# COMMENT

########################  MULTICORE

thread_test=0




epsl_r=0.5
epsl_s=0.5

gp=4100

eg_smp=1
lz_smp=0
mem_lim=1
prb_hsh=1
res_strata=1

set_avx_rand=0
set_always_probe=1

SAMPLE_WITH_PARA


for Threads in 1 2 4 8; do
  for iii in {1..2}; do
    NORMAL
    #compile depends on whether we want to profile.
    RUN_LAZY_HASH
    let "gp++"
  done
done

# eager MULTICORE

gp=4100

eg_smp=0
lz_smp=1
mem_lim=1
prb_hsh=1
res_strata=1

set_avx_rand=0
set_always_probe=1

SAMPLE_WITH_PARA


for Threads in 1 2 4 8; do
  for iii in {1..2}; do
    NORMAL
    #compile depends on whether we want to profile.
    RUN_SHJ
    RUN_PMJ
    let "gp++"
  done
done

# reservoir MULTICORE

gp=4200

eg_smp=1
lz_smp=1
mem_lim=0
prb_hsh=1
res_strata=1

set_avx_rand=0
set_always_probe=1

SAMPLE_WITH_PARA


for Threads in 1 2 4 8; do
  for iii in {1..2}; do
    NORMAL
    #compile depends on whether we want to profile.
    RUN_SHJ
    let "gp++"
  done
done

# COMMENT

# eager probeall MULTICORE

thread_test=0

epsl_r=0.5
epsl_r=0.5
bern=1
univ=1

gp=4300

eg_smp=0
lz_smp=1
mem_lim=1
prb_hsh=1
res_strata=1

set_avx_rand=0
set_always_probe=0

SAMPLE_WITH_PARA


for Threads in 1 2 4 8; do
  for iii in {1..2}; do
    NORMAL
    #compile depends on whether we want to profile.
    RUN_SHJ
    let "gp++"
  done
done


# reservoir probeall MULTICORE

gp=4400

eg_smp=1
lz_smp=1
mem_lim=0
prb_hsh=1
res_strata=1

set_avx_rand=0
set_always_probe=0

SAMPLE_WITH_PARA


for Threads in 1 2 4 8; do
  for iii in {1..2}; do
    NORMAL
    #compile depends on whether we want to profile.
    RUN_SHJ
    let "gp++"
  done
done


COMMENT

thread_test=1

######################################### STREAM FEATURE


function STREAM_RUN_LAZY() {
  NORMAL
  profile_breakdown=1        # set to 1 if we want to measure time breakdown!
  compile=$profile_breakdown # compile depends on whether we want to profile.
  compile
  for benchmark in "AR" "AD"; do #
    for algo in NPO PRO; do
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
      esac
    done
  done
}

function STREAM_RUN_SHJ() {
  NORMAL
  profile_breakdown=1        # set to 1 if we want to measure time breakdown!
  compile=$profile_breakdown # compile depends on whether we want to profile.
  compile
  for benchmark in "AR" "AD"; do #
    for algo in SHJ_JM_NP SHJ_JBCR_NP; do
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
      esac
    done
  done

}

function STREAM_RUN_PMJ() {
  NORMAL
  profile_breakdown=1        # set to 1 if we want to measure time breakdown!
  compile=$profile_breakdown # compile depends on whether we want to profile.
  compile
  for benchmark in "AR" "AD"; do #
    for algo in PMJ_JM_NP PMJ_JBCR_NP; do
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
      esac
    done
  done

}




<<COMMENT

epsl_r=0.5
epsl_s=0.5

gp=5100

eg_smp=1
lz_smp=0
mem_lim=1
prb_hsh=1
res_strata=1

set_avx_rand=0
set_always_probe=1

SAMPLE_WITH_PARA

STREAM_RUN_LAZY

# eager STREAM

gp=5100

eg_smp=0
lz_smp=1
mem_lim=1
prb_hsh=1
res_strata=1

set_avx_rand=0
set_always_probe=1

SAMPLE_WITH_PARA

# STREAM_RUN_SHJ
STREAM_RUN_PMJ

# exit
COMMENT
# reservoir STREAM

gp=5200

epsl_r=0.5
epsl_s=0.5
eg_smp=1
lz_smp=1
mem_lim=0
prb_hsh=1
res_strata=1

set_avx_rand=0
set_always_probe=1

SAMPLE_WITH_PARA

STREAM_RUN_SHJ

<<COMMENT
# probehash STREAM

gp=5900

eg_smp=1
lz_smp=1
mem_lim=1
prb_hsh=0
res_strata=1

set_avx_rand=0
set_always_probe=1

SAMPLE_WITH_PARA

STREAM_RUN_SHJ

# eager probeall STREAM

gp=5300

eg_smp=0
lz_smp=1
mem_lim=1
prb_hsh=1
res_strata=1

set_avx_rand=0
set_always_probe=0

SAMPLE_WITH_PARA

STREAM_RUN_SHJ


# reservoir probeall STREAM

gp=5400

eg_smp=1
lz_smp=1
mem_lim=0
prb_hsh=1
res_strata=1

set_avx_rand=0
set_always_probe=0

SAMPLE_WITH_PARA

STREAM_RUN_SHJ

COMMENT
