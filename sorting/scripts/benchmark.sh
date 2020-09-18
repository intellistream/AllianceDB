#!/bin/bash

#set -e
## Set L3 Cache according to your machine.
sed -i -e "s/#define L3_CACHE_SIZE [[:alnum:]]*/#define L3_CACHE_SIZE 19922944/g" ../utils/params.h
sed -i -e "s/#define PERF_COUNTERS/#define NO_PERF_COUNTERS/g" ../utils/perf_counters.h
sed -i -e "s/#define NO_TIMING/#define TIMING/g" ../joins/common_functions.h

# change cpu-mapping path here, e.g. following changes /data1/xtra/cpu-mapping.txt to /data1/xtra/cpu-mapping.txt
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
}

function benchmarkRun() {
  #####native execution
  echo "==benchmark:$benchmark -a $algo -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -g $gap -o /data1/xtra/results/breakdown/profile_$id.txt -I $id== "
  #echo 3 >/proc/sys/vm/drop_caches
  ../sorting -a $algo -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -g $gap -o /data1/xtra/results/breakdown/profile_$id.txt -I $id
  if [[ $? -eq 139 ]]; then echo "oops, sigsegv" exit 1; fi
}

#function perfUarchBenchmarkRun() {
##  #####native execution
##  echo "==benchmark:$benchmark -a $algo -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -g $gap -o /data1/xtra/results/breakdown/perf_$id.csv -I $id== "
#  #echo 3 >/proc/sys/vm/drop_caches
#  perf stat -I10 -x, -a --topdown  -o /data1/xtra/results/breakdown/perf_$id.csv ../sorting -a $algo  -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -g $gap -I $id
##  perf stat -I10 -a -x, -o /data1/xtra/results/breakdown/perf_a_$id.csv -e cache-references,cache-misses,cycles,instructions,branches,faults,migrations ../sorting -a $algo  -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -g $gap -I $id
##  vtune -collect uarch-expoloration XXX
##  toplev.py -I 10 -l2  -a -x, -o /data1/xtra/results/breakdown/perf_a_$id.csv ../sorting -a $algo -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -g $gap -I $id
#  if [[ $? -eq 139 ]]; then echo "oops, sigsegv" exit 1; fi
#}

function perfUarchBenchmarkRun() {
#  #####native execution
#  echo "==benchmark:$benchmark -a $algo -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -g $gap -o /data1/xtra/results/breakdown/perf_$id.csv -I $id== "
  #####native execution
  echo "==benchmark:$benchmark -a $algo -t $ts -w $WINDOW_SIZE -e $STEP_SIZE -q $STEP_SIZE_S -l $INTERVAL -d $distrbution -z $skew -D $TS_DISTRIBUTION -Z $ZIPF_FACTOR -n $Threads -g $gap -P $DD -W $FIXS -I $id =="

  #echo 3 >/proc/sys/vm/drop_caches
#  perf stat -I10 -x, -a --topdown  -o /data1/xtra/results/breakdown/perf_$id.csv ../sorting -a $algo  -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -g $gap -I $id
  perf stat -I10 -x, -a --topdown  -o /data1/xtra/results/breakdown/perf_$id.csv ../sorting -a $algo -t $ts -w $WINDOW_SIZE -e $STEP_SIZE -q $STEP_SIZE_S -l $INTERVAL -d $distrbution -z $skew -D $TS_DISTRIBUTION -Z $ZIPF_FACTOR -n $Threads -W $FIXS -g $gap -P $DD -I $id
#  perf stat -I10 -a -x, -o /data1/xtra/results/breakdown/perf_a_$id.csv -e cache-references,cache-misses,cycles,instructions,branches,faults,migrations ../sorting -a $algo  -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -g $gap -I $id
#  vtune -collect uarch-expoloration XXX
#  toplev.py -I 10 -l2  -a -x, -o /data1/xtra/results/breakdown/perf_a_$id.csv ../sorting -a $algo -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -g $gap -I $id
  if [[ $? -eq 139 ]]; then echo "oops, sigsegv" exit 1; fi
}

function perfUtilBenchmarkRun() {
  #####native execution
  echo "==benchmark:$benchmark -a $algo -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -g $gap -o /data1/xtra/results/breakdown/perf_$id.csv -I $id== "
  #echo 3 >/proc/sys/vm/drop_caches
  perf stat -I10 -x, -o /data1/xtra/results/breakdown/perf_$id.csv -e cache-misses,cycles ../sorting -a $algo  -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -g $gap -I $id
  if [[ $? -eq 139 ]]; then echo "oops, sigsegv" exit 1; fi
}

function Run() {
  #####native execution
  echo "==benchmark:$benchmark -a $algo -n $Threads=="
  #echo 3 >/proc/sys/vm/drop_caches
  ../sorting -a $algo -r $RSIZE -s $SSIZE -n $Threads
}

function KimRun() {
  #####native execution
  echo "==benchmark:$benchmark -a $algo -t $ts -w $WINDOW_SIZE -e $STEP_SIZE -q $STEP_SIZE_S -l $INTERVAL -d $distrbution -z $skew -D $TS_DISTRIBUTION -Z $ZIPF_FACTOR -n $Threads -g $gap -P $DD -W $FIXS -I $id =="
  #echo 3 >/proc/sys/vm/drop_caches
  ../sorting -a $algo -t $ts -w $WINDOW_SIZE -e $STEP_SIZE -q $STEP_SIZE_S -l $INTERVAL -d $distrbution -z $skew -D $TS_DISTRIBUTION -Z $ZIPF_FACTOR -n $Threads -W $FIXS -g $gap -P $DD -I $id
}

function PARTITION_ONLY() {
  sed -i -e "s/#define JOIN/#define NO_JOIN/g" ../joins/common_functions.h
  sed -i -e "s/#define SORT/#define NO_SORT/g" ../joins/common_functions.h
}
function PARTITION_BUILD_SORT() {
  sed -i -e "s/#define JOIN/#define NO_JOIN/g" ../joins/common_functions.h
  sed -i -e "s/#define NO_SORT/#define SORT/g" ../joins/common_functions.h
}
function PARTITION_BUILD_SORT_MERGE_JOIN() {
  sed -i -e "s/#define NO_JOIN/#define JOIN/g" ../joins/common_functions.h
  sed -i -e "s/#define SORT/#define NO_SORT/g" ../joins/common_functions.h
}

function SetStockParameters() { #matches: 15595000. #inputs= 60527 + 77227
  ts=1 # stream case
  WINDOW_SIZE=1000
  RSIZE=60527
  SSIZE=77227
  RPATH=/data1/xtra/datasets/stock/cj_1000ms_1t.txt
  SPATH=/data1/xtra/datasets/stock/sb_1000ms_1t.txt
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
  RPATH=/data1/xtra/datasets/rovio/1000ms_1t.txt
  SPATH=/data1/xtra/datasets/rovio/1000ms_1t.txt
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
  RPATH=/data1/xtra/datasets/YSB/campaigns_id.txt
  SPATH=/data1/xtra/datasets/YSB/ad_events.txt
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
  RPATH=/data1/xtra/datasets/DEBS/posts_key32_partitioned.csv
  SPATH=/data1/xtra/datasets/DEBS/comments_key32_partitioned.csv
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
  gap=12800
  DD=1
  sed -i -e "s/scalarflag [[:alnum:]]*/scalarflag 0/g" ../joins/common_functions.h
}

#recompile by default.
compile
# Configurable variables
# Generate a timestamp
timestamp=$(date +%Y%m%d-%H%M)
output=test$timestamp.txt

#general benchmark.
GENERAL_BENCH=0
if [ $GENERAL_BENCH == 1 ]; then
sed -i -e "s/#define NO_TIMING/#define TIMING/g" ../joins/common_functions.h #enable time measurement
sed -i -e "s/#define PERF_COUNTERS/#define NO_PERF_COUNTERS/g" ../utils/perf_counters.h #disable hardware counters
profile_breakdown=0
compile=0
for algo in m-way m-pass; do
  for benchmark in  "Stock" "Rovio" "YSB" "DEBS" "AR" "RAR" "KD" "WS" "DD" "ScaleStock" "ScaleRovio" "ScaleYSB" "ScaleDEBS"; do
    case "$benchmark" in
    # Batch -a SHJ_JM_NP -n 8 -t 1 -w 1000 -e 1000 -l 10 -d 0 -Z 1
    "AR") #test arrival rate and assume both inputs have same arrival rate.
      id=0
      ## Figure 1
      ResetParameters
      FIXS=0 #varying both.
      ts=1   # stream case
      # step size should be bigger than nthreads
      for STEP_SIZE in 3200 6400 12800 25600; do #3200 6400 12800 25600
        #WINDOW_SIZE=$(expr $DEFAULT_WINDOW_SIZE \* $DEFAULT_STEP_SIZE / $STEP_SIZE) #ensure relation size is the same.
        echo relation size is $(expr $WINDOW_SIZE / $INTERVAL \* $STEP_SIZE)
        gap=$(($STEP_SIZE / 500 * $WINDOW_SIZE))
        KimRun
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
        KimRun
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
        KimRun
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
        KimRun
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
        gap=$(($STEP_SIZE / 1000 * $WINDOW_SIZE))
        KimRun
        let "id++"
      done
      ;;
    "DD") #test data duplication
      id=25
      ## Figure 6
      ResetParameters
      FIXS=1
      ts=1
      STEP_SIZE=6400
      STEP_SIZE_S=6400
      echo test DD 25 - 28
      for DD in 1 100 1000 10000; do
        gap=$(($STEP_SIZE * $WINDOW_SIZE * $DD / 500))
        KimRun
        let "id++"
      done
      ;;
    "Stock")
      id=38
      ResetParameters
      SetStockParameters
      benchmarkRun
      ;;
    "Rovio") #matches:
      id=39
      ResetParameters
      SetRovioParameters
      benchmarkRun
      ;;
    "YSB")
      id=40
      ResetParameters
      SetYSBParameters
      benchmarkRun
      ;;
    "DEBS")
      id=41
      ResetParameters
      SetDEBSParameters
      benchmarkRun
      ;;
    "ScaleStock")
      id=42
      ResetParameters
      SetStockParameters
      echo test scalability of Stock 42 - 45
      for Threads in 1 2 4 8; do
        benchmarkRun
        let "id++"
      done
      ;;
    "ScaleRovio")
      id=46
      ResetParameters
      SetRovioParameters
      echo test scalability 46 - 49
      for Threads in 1 2 4 8; do
        benchmarkRun
        let "id++"
      done
      ;;
    "ScaleYSB")
      id=50
      ResetParameters
      SetYSBParameters
      echo test scalability 50 - 53
      for Threads in 1 2 4 8; do
        benchmarkRun
        let "id++"
      done
      ;;
    "ScaleDEBS")
      id=54
      ResetParameters
      SetDEBSParameters
      echo test scalability 54 - 57
      for Threads in 1 2 4 8; do
        benchmarkRun
        let "id++"
      done
      ;;
    esac
  done
done
fi

## MICRO STUDY
PROFILE_MICRO=0
if [ $PROFILE_MICRO == 1 ]; then
  sed -i -e "s/#define NO_TIMING/#define TIMING/g" ../joins/common_functions.h #enable time measurement
  sed -i -e "s/#define PERF_COUNTERS/#define NO_PERF_COUNTERS/g" ../utils/perf_counters.h #disable hardware counters
  for benchmark in "SIMD_STUDY"; do #
    case "$benchmark" in
    "SIMD_STUDY")
      id=100
      ResetParameters
      ts=0 # batch data.
      echo SIMD 100-103
      PARTITION_ONLY
      compile=1
      for algo in "m-way" "m-pass"; do
        for scalar in 0 1; do
          sed -i -e "s/scalarflag [[:alnum:]]*/scalarflag $scalar/g" ../joins/common_functions.h
          compile
          KimRun
          let "id++"
        done
      done
      PARTITION_BUILD_SORT
      compile=1
      for algo in "m-way" "m-pass"; do
        for scalar in 0 1; do
          sed -i -e "s/scalarflag [[:alnum:]]*/scalarflag $scalar/g" ../joins/common_functions.h
          compile
          KimRun
          let "id++"
        done
      done
      ;;
    esac
  done
fi

PROFILE_YSB=1 ## Cache misses profiling with YSB, please run the program with sudo
if [ $PROFILE_YSB == 1 ]; then
  sed -i -e "s/#define TIMING/#define NO_TIMING/g" ../joins/common_functions.h #disable time measurement
  sed -i -e "s/#define NO_PERF_COUNTERS/#define PERF_COUNTERS/g" ../utils/perf_counters.h
  for benchmark in "YSB"; do
    id=201
    PARTITION_ONLY
    compile=1
    compile
    for algo in "m-way" "m-pass"; do
      case "$benchmark" in
      "YSB")
        ResetParameters
        SetYSBParameters
        rm /data1/xtra/results/breakdown/profile_$id.txt
        benchmarkRun
        ;;
      esac
      let "id++"
    done

    PARTITION_BUILD_SORT_MERGE_JOIN
    compile=1
    compile
    for algo in "m-way" "m-pass"; do
      case "$benchmark" in
      "YSB")
        ResetParameters
        SetYSBParameters
        rm /data1/xtra/results/breakdown/profile_$id.txt
        benchmarkRun
        ;;
      esac
      let "id++"
    done
  done
  sed -i -e "s/#define PERF_COUNTERS/#define NO_PERF_COUNTERS/g" ../utils/perf_counters.h
fi

#export PATH=~/workspace/pmu-tools:$PATH
PERF_YSB=0 ## Hardware Counters profiling with YSB, please run the program with sudo
if [ $PERF_YSB == 1 ]; then
#  compile=1
#  compile
  sed -i -e "s/#define TIMING/#define NO_TIMING/g" ../joins/common_functions.h
  for benchmark in "Kim"; do #"YSB
    id=300
    for algo in "m-way" "m-pass"; do
      case "$benchmark" in
      "Kim")
        ResetParameters
#        SetYSBParameters
#        SetDEBSParameters
        STEP_SIZE=1280
        STEP_SIZE_S=12800
        WINDOW_SIZE=10000
        rm /data1/xtra/results/breakdown/perf_$id.csv
        perfUarchBenchmarkRun
        ;;
      esac
      let "id++"
    done
  done

#  for benchmark in "YSB"; do #"YSB
#    id=400
#    for algo in "m-way" "m-pass"; do
#      case "$benchmark" in
#      "YSB")
#        ResetParameters
#        SetYSBParameters
#        rm /data1/xtra/results/breakdown/perf_$id.csv
#        perfUtilBenchmarkRun
#        ;;
#      esac
#      let "id++"
#    done
#  done
fi
