#!/bin/bash
#set -e

profile_breakdown=1 # set to 1 if we want to measure time breakdown! and also dedefine eager in common_function.h

function compile() {
  cmake .. | tail -n +90
  make -C .. clean -s
  make -C .. -j4 -s
}
function Run() {
  #####native execution
  echo "==benchmark:$benchmark -a $algo -n $Threads=="
  echo 3 >/proc/sys/vm/drop_caches
  ../hashing -a $algo -r $RSIZE -s $SSIZE -n $Threads
}

function KimRun() {
  #####native execution
  echo "==KIM benchmark:$benchmark -a $algo -t $ts -w $WINDOW_SIZE -e $STEP_SIZE -q $STEP_SIZE_S -l $INTERVAL -d $distrbution -z $skew -D $TS_DISTRIBUTION -Z $ZIPF_FACTOR -n $Threads -I $id -W $FIXS -[ $progress_step -] $merge_step -G $group -g $gap=="
  echo 3 >/proc/sys/vm/drop_caches
  ../hashing -a $algo -t $ts -w $WINDOW_SIZE -e $STEP_SIZE -q $STEP_SIZE_S -l $INTERVAL -d $distrbution -z $skew -D $TS_DISTRIBUTION -Z $ZIPF_FACTOR -n $Threads -I $id -W $FIXS -[ $progress_step -] $merge_step -G $group -g $gap
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
    if [ $algo == SHJ_JM_NP ] || [ $algo == SHJ_JBCR_NP ]; then
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

function RUNALLKIM() {
  if [ $profile_breakdown == 1 ]; then
    if [ $algo == SHJ_JM_NP ] || [ $algo == SHJ_JBCR_NP ]; then
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


function benchmarkRun() {
  #####native execution
  echo "==benchmark:$benchmark -a $algo -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -I $id -[ $progress_step -] $merge_step -G $group -g $gap=="
  echo 3 >/proc/sys/vm/drop_caches
  ../hashing -a $algo -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -I $id -[ $progress_step -] $merge_step -G $group -g $gap
  if [[ $? -eq 139 ]]; then echo "oops, sigsegv" exit -1; fi
}

function SetStockParameters() { #matches: 229517.
  ts=1 # stream case
  #  WINDOW_SIZE=1000
  #  RSIZE=60257
  #  SSIZE=77227
  #  RPATH=/data1/xtra/datasets/stock/cj_3s_1t.txt
  #  SPATH=/data1/xtra/datasets/stock/sb_3s_1t.txt
  WINDOW_SIZE=5000
  RSIZE=116941
  SSIZE=151500
  RPATH=/data1/xtra/datasets/stock/cj_60s_1t.txt
  SPATH=/data1/xtra/datasets/stock/sb_60s_1t.txt
  RKEY=0
  SKEY=0
  RTS=1
  STS=1
  gap=229
}

function SetRovioParameters() { #matches: 27660233
  ts=1 # stream case
  WINDOW_SIZE=50
  RSIZE=51001
  SSIZE=51001
  RPATH=/data1/xtra/datasets/rovio/500ms_1t.txt
  SPATH=/data1/xtra/datasets/rovio/500ms_1t.txt
  RKEY=0
  SKEY=0
  RTS=3
  STS=3
  gap=27660
}

function SetYSBParameters() { #matches: 40100000.
  ts=1 # stream case
  WINDOW_SIZE=400
  RSIZE=1000
  SSIZE=40100000
  RPATH=/data1/xtra/datasets/YSB/campaigns_id.txt
  SPATH=/data1/xtra/datasets/YSB/ad_events.txt
  RKEY=0
  SKEY=0
  RTS=0
  STS=1
  gap=40100
}

function SetDEBSParameters() { #matches: 251033140
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
  gap=128000
  sed -i -e "s/scalarflag [[:alnum:]]*/scalarflag 0/g" ../helper/sort_common.h
}

#recompile by default.
compile
# Configurable variables
# Generate a timestamp
timestamp=$(date +%Y%m%d-%H%M)
output=test$timestamp.txt
#benchmark experiment only apply for hashing directory.
for benchmark in ""; do #"PRJ_RADIX_BITS_STUDY" "PMJ_SORT_STEP_STUDY" "GROUP_SIZE_STUDY"
  case "$benchmark" in
  "SIMD_STUDY")
    id=104
    ResetParameters
    ts=0 # batch data.
    echo SIMD PMJ 104 - 107
    for algo in "PMJ_JM_NP" "PMJ_JBCR_NP"; do
      for scalar in 0 1; do
        sed -i -e "s/scalarflag [[:alnum:]]*/scalarflag $scalar/g" ../helper/sort_common.h
        RUNALLKIM
        let "id++"
      done
    done
    python3 breakdown_simd.py
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
        KimRun
        let "id++"
      done
    done
    python3 breakdown_bucket.py
    ;;
  "PRJ_RADIX_BITS_STUDY")
    algo="PRO"
    id=113
    ResetParameters
    ts=0   # batch data.
    for b in 8 10 12 14 16 18; do
      echo RADIX BITS STUDY $id
      sed -i -e "s/NUM_RADIX_BITS [[:alnum:]]*/NUM_RADIX_BITS $b/g" ../joins/prj_params.h
      compile
      KimRun
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
      FULLKIMRUN
      let "id++"
    done
    python3 breakdown_sort.py
    python3 latency_sort.py
    python3 progressive_sort.py
    ;;
  "GROUP_SIZE_STUDY")
    id=124
    algo="PMJ_JBCR_NP"
    ResetParameters
    ts=0 # batch data.
    echo GROUP_SIZE_STUDY PMJ 124 - 127
    for group in 1 2 4 8; do
      FULLKIMRUN
      let "id++"
    done

    algo="SHJ_JBCR_NP"
    ResetParameters
    echo GROUP_SIZE_STUDY SHJ 128 - 131
    for group in 1 2 4 8; do
      SHJKIMRUN
      let "id++"
    done
    python3 breakdown_group_pmj.py
    python3 breakdown_group_shj.py
    ;;
  esac
done

# general benchmark.
for algo in NPO PRO SHJ_JM_NP SHJ_JBCR_NP PMJ_JM_NP PMJ_JBCR_NP ; do #NPO PRO SHJ_JM_NP SHJ_JBCR_NP PMJ_JM_NP PMJ_JBCR_NP
  for benchmark in "AR" "RAR" "RAR2" "AD" "KD" "WS" "KD2" "WS2" "WS3" "WS4" ; do # "ScaleStock" "ScaleRovio" "ScaleYSB" "ScaleDEBS" # "Stock" "Rovio" "YSB" "DEBS" "AR" "RAR" "RAR2" "AD" "KD" "WS" "KD2" "WS2" "WS3" "WS4"
    case "$benchmark" in
    # Batch -a SHJ_JM_NP -n 8 -t 1 -w 1000 -e 1000 -l 10 -d 0 -Z 1
    "AR") #test arrival rate and assume both inputs have same arrival rate.
      id=0
      ## Figure 1
      ResetParameters
      FIXS=0 #varying both.
      STEP_SIZE=12800
      echo test varying input arrival rate 0 - 4 # test (1) means infinite arrival rate (batch).
      ts=0                                       # batch case
      echo relation size is $(expr $WINDOW_SIZE / $INTERVAL \* $STEP_SIZE)
      RUNALLKIM
      let "id++"

      ts=1 # stream case
      # step size should be bigger than nthreads
      for STEP_SIZE in 1600 3200 6400 12800; do #128000
        #WINDOW_SIZE=$(expr $DEFAULT_WINDOW_SIZE \* $DEFAULT_STEP_SIZE / $STEP_SIZE) #ensure relation size is the same.
        echo relation size is $(expr $WINDOW_SIZE / $INTERVAL \* $STEP_SIZE)
        RUNALLKIM
        let "id++"
      done
      ;;
    "RAR") #test relative arrival rate when S is large
      id=28
      ## Figure 7
      ResetParameters
      FIXS=1
      echo test relative arrival rate 28 - 31
      ts=1 # stream case
      # step size should be bigger than nthreads
      STEP_SIZE_S=128000
      for STEP_SIZE in 1600 3200 6400 12800; do
        #        WINDOW_SIZE=$(expr $DEFAULT_WINDOW_SIZE \* $DEFAULT_STEP_SIZE / $STEP_SIZE) #ensure relation size is the same.
        echo relation size is $(expr $WINDOW_SIZE / $INTERVAL \* $STEP_SIZE)
        RUNALLKIM
        let "id++"
      done
      ;;
    "RAR2") #test relative arrival rate when S is small
      id=32
      ## Figure 8
      ResetParameters
      FIXS=1
      echo test relative arrival rate 32 - 35
      ts=1 # stream case
      # step size should be bigger than nthreads
      # remember to fix the relation size of S.
      STEP_SIZE_S=12800
      for STEP_SIZE in 1600 3200 6400 12800; do
        #        WINDOW_SIZE=$(expr $DEFAULT_WINDOW_SIZE \* $DEFAULT_STEP_SIZE / $STEP_SIZE) #ensure relation size is the same.
        echo relation size is $(expr $WINDOW_SIZE / $INTERVAL \* $STEP_SIZE)
        RUNALLKIM
        let "id++"
      done
      ;;
    "AD") #test arrival distribution
      id=5
      ## Figure 2
      ResetParameters
      TS_DISTRIBUTION=2
      echo test varying timestamp distribution 5 - 9
      for ZIPF_FACTOR in 0 0.4 0.8 1.2 1.6; do #
        RUNALLKIM
        let "id++"
      done
      ;;
    "KD") #test key distribution
      id=10
      ## Figure 3
      ResetParameters
      echo test varying key distribution 10 - 15
      distrbution=0 #unique
      RUNALLKIM
      let "id++"

      distrbution=2 #varying zipf factor
      for skew in 0 0.4 0.8 1.2 1.6; do
        RUNALLKIM
        let "id++"
      done
      ;;
    "KD2") #test key distribution when data at rest.
      id=19
      ## Figure 5
      ResetParameters
      ts=0 # data at rest.
      echo test varying key distribution 19 - 24
      distrbution=0 #unique
      RUNALLKIM
      let "id++"

      distrbution=2 #zipf
      for skew in 0 0.4 0.8 1.2 1.6; do
        RUNALLKIM
        let "id++"
      done
      ;;
    "WS") #test window size
      id=16
      ## Figure 4
      ResetParameters
      echo test varying window size 16 - 18
      for WINDOW_SIZE in 500 750 1000; do
        RUNALLKIM
        let "id++"
      done
      ;;
    "WS2") #test window size when data at rest.
      id=25
      ## Figure 6
      ResetParameters
      ts=0 # data at rest.
      echo test varying window size 25 - 27
      for WINDOW_SIZE in 500 750 1000; do
        RUNALLKIM
        let "id++"
      done
      ;;
    "WS3") #test window size for extra large size of window
      id=36
      ## Figure 4 extra
      ResetParameters
      echo test varying window size 36
      for WINDOW_SIZE in 1500; do
        RUNALLKIM
        let "id++"
      done
      ;;
    "WS4") #test window size for extra large size of window when data at rest
      id=37
      ## Figure 6 extra
      ResetParameters
      ts=0 # data at rest.
      echo test varying window size 37
      for WINDOW_SIZE in 1500; do
        RUNALLKIM
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
python3 jobdone.py

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