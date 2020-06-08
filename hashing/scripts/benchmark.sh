#!/bin/bash
#set -e
## Create directories on your machine.
mkdir -p /data1/xtra/results/breakdown/partition_buildsort_probemerge_join
mkdir -p /data1/xtra/results/breakdown/partition_only
mkdir -p /data1/xtra/results/breakdown/partition_buildsort_only
mkdir -p /data1/xtra/results/breakdown/partition_buildsort_probemerge_only
mkdir -p /data1/xtra/results/breakdown/allIncludes

mkdir -p /data1/xtra/results/figure
mkdir -p /data1/xtra/results/gaps
mkdir -p /data1/xtra/results/latency
mkdir -p /data1/xtra/results/records
mkdir -p /data1/xtra/results/timestamps



## Set L3 Cache according to your machine.
sed -i -e "s/#define L3_CACHE_SIZE [[:alnum:]]*/#define L3_CACHE_SIZE 20971520/g" ../utils/params.h
sed -i -e "s/#define PERF_COUNTERS/#define NO_PERF_COUNTERS/g" ../utils/perf_counters.h
sed -i -e "s/#define NO_TIMING/#define TIMING/g" ../joins/common_functions.h

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
}
function Run() {
  #####native execution
  echo "==benchmark:$benchmark -a $algo -n $Threads=="
  echo 3 >/proc/sys/vm/drop_caches
  ../hashing -a $algo -r $RSIZE -s $SSIZE -n $Threads
}
function benchmarkRun() {
  #####native execution
  echo "==benchmark:$benchmark -a $algo -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -I $id -[ $progress_step -] $merge_step -G $group -g $gap -o /data1/xtra/results/breakdown/profile_$id.txt =="
  echo 3 >/proc/sys/vm/drop_caches
  ../hashing -a $algo -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -I $id -[ $progress_step -] $merge_step -G $group -g $gap -o /data1/xtra/results/breakdown/profile_$id.txt
  if [[ $? -eq 139 ]]; then echo "oops, sigsegv" exit -1; fi
}

function KimRun() {
  #####native execution
  echo "==KIM benchmark:$benchmark -a $algo -t $ts -w $WINDOW_SIZE -e $STEP_SIZE -q $STEP_SIZE_S -l $INTERVAL -d $distrbution -z $skew -D $TS_DISTRIBUTION -Z $ZIPF_FACTOR -n $Threads -I $id -W $FIXS -[ $progress_step -] $merge_step -G $group -P $DD -g $gap =="
  echo 3 >/proc/sys/vm/drop_caches
  ../hashing -a $algo -t $ts -w $WINDOW_SIZE -e $STEP_SIZE -q $STEP_SIZE_S -l $INTERVAL -d $distrbution -z $skew -D $TS_DISTRIBUTION -Z $ZIPF_FACTOR -n $Threads -I $id -W $FIXS -[ $progress_step -] $merge_step -G $group -P $DD -g $gap
  if [[ $? -eq 139 ]]; then echo "oops, sigsegv" exit -1; fi
}
function PARTITION_ONLY() {
  sed -i -e "s/#define JOIN/#define NO_JOIN/g" ../joins/common_functions.h
  sed -i -e "s/#define MERGE/#define NO_MERGE/g" ../joins/common_functions.h
  sed -i -e "s/#define MATCH/#define NO_MATCH/g" ../joins/common_functions.h
  sed -i -e "s/#define WAIT/#define NO_WAIT/g" ../joins/common_functions.h
}

function PARTITION_BUILD_SORT() {
  sed -i -e "s/#define NO_JOIN/#define JOIN/g" ../joins/common_functions.h
  sed -i -e "s/#define MERGE/#define NO_MERGE/g" ../joins/common_functions.h
  sed -i -e "s/#define MATCH/#define NO_MATCH/g" ../joins/common_functions.h
  sed -i -e "s/#define WAIT/#define NO_WAIT/g" ../joins/common_functions.h
}

function PARTITION_BUILD_SORT_MERGE() {
  sed -i -e "s/#define NO_JOIN/#define JOIN/g" ../joins/common_functions.h
  sed -i -e "s/#define NO_MERGE/#define MERGE/g" ../joins/common_functions.h
  sed -i -e "s/#define MATCH/#define NO_MATCH/g" ../joins/common_functions.h
  sed -i -e "s/#define WAIT/#define NO_WAIT/g" ../joins/common_functions.h
}

function PARTITION_BUILD_SORT_MERGE_JOIN() {
  sed -i -e "s/#define NO_JOIN/#define JOIN/g" ../joins/common_functions.h
  sed -i -e "s/#define NO_MERGE/#define MERGE/g" ../joins/common_functions.h
  sed -i -e "s/#define NO_MATCH/#define MATCH/g" ../joins/common_functions.h
  sed -i -e "s/#define WAIT/#define NO_WAIT/g" ../joins/common_functions.h
}

function ALL_ON() {
  sed -i -e "s/#define NO_JOIN/#define JOIN/g" ../joins/common_functions.h
  sed -i -e "s/#define NO_MERGE/#define MERGE/g" ../joins/common_functions.h
  sed -i -e "s/#define NO_MATCH/#define MATCH/g" ../joins/common_functions.h
  sed -i -e "s/#define NO_WAIT/#define WAIT/g" ../joins/common_functions.h
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
    if [ $algo == SHJ_JM_NP ] || [ $algo == SHJ_JM_P ] || [ $algo == SHJ_JBCR_NP ]; then
      SHJBENCHRUN
    else
      if [ $algo == PMJ_JM_NP ] || [ $algo == PMJ_JBCR_NP ]; then
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
    if [ $algo == SHJ_JM_NP ] || [ $algo == SHJ_JM_P ] || [ $algo == SHJ_JBCR_NP ] || [ $algo == SHJ_HS_NP ]; then
      SHJKIMRUN
    else
      if [ $algo == PMJ_JM_NP ] || [ $algo == PMJ_JBCR_NP ]; then
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
  progress_step=20
  merge_step=16 #not in use.
  group=2
  gap=12800
  DD=1
  sed -i -e "s/scalarflag [[:alnum:]]*/scalarflag 0/g" ../helper/sort_common.h
}

#compile once by default.
compile
# Configurable variables
# Generate a timestamp
timestamp=$(date +%Y%m%d-%H%M)
output=test$timestamp.txt

## general benchmark.
GENERAL_BENCH=1
if [ $GENERAL_BENCH == 1 ]; then
  profile_breakdown=0        # set to 1 if we want to measure time breakdown!
  compile=$profile_breakdown # compile depends on whether we want to profile.
  for benchmark in "Stock" "Rovio" "YSB" "DEBS" "AR" "RAR" "KD" "WS" "DD" ; do #"Stock" "Rovio" "YSB" "DEBS" "AR" "RAR" "KD" "WS" "DD"
    for algo in NPO SHJ_JM_NP SHJ_JBCR_NP PMJ_JM_NP PMJ_JBCR_NP ; do # NPO PRO SHJ_JM_NP SHJ_JBCR_NP PMJ_JM_NP PMJ_JBCR_NP
      case "$benchmark" in
      # Batch -a SHJ_JM_NP -n 8 -t 1 -w 1000 -e 1000 -l 10 -d 0 -Z 1
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
fi
## SCLAE STUDY
SCALE_STUDY=0
if [ $SCALE_STUDY == 1 ]; then
  profile_breakdown=0 #compile depends on whether we want to profile.
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
#    algo="PMJ_JBCR_NP"
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

PROFILE_YSB=0 ## Cache misses profiling with YSB, please run the program with sudo
if [ $PROFILE_YSB == 1 ]; then
  sed -i -e "s/#define TIMING/#define NO_TIMING/g" ../joins/common_functions.h #disable time measurement
  sed -i -e "s/#define NO_PERF_COUNTERS/#define PERF_COUNTERS/g" ../utils/perf_counters.h
  profile_breakdown=0      # disable measure time breakdown!
  eager=1 #with eager
  compile=1
  PARTITION_ONLY
  compile
  for benchmark in "YSB"; do #"
    id=205
    for algo in PRO SHJ_JM_NP SHJ_JBCR_NP PMJ_JM_NP PMJ_JBCR_NP; do
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

  PARTITION_BUILD_SORT_MERGE_JOIN
  compile
  for benchmark in "YSB"; do #"
    id=210
    for algo in NPO PRO SHJ_JM_NP SHJ_JBCR_NP PMJ_JM_NP PMJ_JBCR_NP; do # ~215
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
fi

## MICRO STUDY
PROFILE_MICRO=0
if [ $PROFILE_MICRO == 1 ]; then
  profile_breakdown=1 #compile depends on whether we want to profile.
  compile=1           #enable compiling.
  #benchmark experiment only apply for hashing directory.
  for benchmark in "SIMD_STUDY""BUCKET_SIZE_STUDY" "PRJ_RADIX_BITS_STUDY" "PMJ_SORT_STEP_STUDY" "GROUP_SIZE_STUDY" "HS_STUDY" "P_NP_STUDY"; do #
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
      ts=0 # batch data.
      algo="PMJ_JBCR_NP"
      ResetParameters
      echo GROUP_SIZE_STUDY PMJ 124 - 127
      for group in 1 2 4 8; do
        RUNALLMic
        let "id++"
      done

      algo="SHJ_JBCR_NP"
      ResetParameters
      echo GROUP_SIZE_STUDY SHJ 128 - 131
      for group in 1 2 4 8; do
        RUNALLMic
        let "id++"
      done
      ResetParameters
      algo="PMJ_JM_NP"
      RUNALLMic
      algo="SHJ_JM_NP"
      RUNALLMic
      python3 breakdown_group_pmj.py
      python3 breakdown_group_shj.py
      ;;
    "HS_STUDY")
      id=132
      ResetParameters
      ts=0 # batch data.
      FIXS=1
      STEP_SIZE=1600
      STEP_SIZE_S=1600
      echo HS_STUDY 132
      WINDOW_SIZE=1000
      gap=$(($STEP_SIZE / 500 * $WINDOW_SIZE))
      algo="SHJ_HS_NP"
      RUNALLMic
      algo="SHJ_JM_NP"
      RUNALLMic
      python3 breakdown_hsstudy.py
      ;;
    "P_NP_STUDY")
      id=133
      ResetParameters
      ts=0 # batch data.
      FIXS=1
      STEP_SIZE=12800
      STEP_SIZE_S=12800
      echo P_NP_STUDY 134
      WINDOW_SIZE=1000
      gap=$(($STEP_SIZE / 500 * $WINDOW_SIZE))
      algo="SHJ_JM_NP"
      RUNALLMic
      algo="SHJ_JM_P"
      RUNALLMic
      python3 breakdown_p_np_study.py
      ;;
    esac
  done
fi
#./draw.sh
python3 jobdone.py
