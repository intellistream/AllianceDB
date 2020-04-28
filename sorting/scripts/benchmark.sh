#!/bin/bash
#set -e
compile=0
function compile() {
  if [ $comple != 0 ]; then
    cd ..
    cmake . | tail -n +90
    cd scripts
    make -C .. clean -s
    make -C .. -j4 -s
  fi
}

function benchmarkRun() {
  #####native execution
  echo "==benchmark:$benchmark -a $algo -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -g $gap -I $id== "
  #echo 3 >/proc/sys/vm/drop_caches
  ../sorting -a $algo -t $ts -w $WINDOW_SIZE -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -g $gap -I $id
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
  echo "==benchmark:$benchmark -a $algo -t $ts -w $WINDOW_SIZE -e $STEP_SIZE -q $STEP_SIZE_S -l $INTERVAL -d $distrbution -z $skew -D $TS_DISTRIBUTION -Z $ZIPF_FACTOR -n $Threads -g $gap  -W $FIXS -I $id =="
  #echo 3 >/proc/sys/vm/drop_caches
  ../sorting -a $algo -t $ts -w $WINDOW_SIZE -e $STEP_SIZE -q $STEP_SIZE_S -l $INTERVAL -d $distrbution -z $skew -D $TS_DISTRIBUTION -Z $ZIPF_FACTOR -n $Threads -W $FIXS -g $gap -I $id
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
  gap=12800
  sed -i -e "s/scalarflag [[:alnum:]]*/scalarflag 0/g" ../joins/joincommon.h
}

#recompile by default.
compile
# Configurable variables
# Generate a timestamp
timestamp=$(date +%Y%m%d-%H%M)
output=test$timestamp.txt
#benchmark experiment only apply for hashing directory.
for benchmark in ""; do #
  case "$benchmark" in
  "SIMD_STUDY")
    id=100
    ResetParameters
    ts=0 # batch data.
    echo SIMD 100-103
    for algo in "m-way" "m-pass"; do
      for scalar in 0 1; do
        sed -i -e "s/scalarflag [[:alnum:]]*/scalarflag $scalar/g" ../joins/joincommon.h
        compile
        KimRun
        let "id++"
      done
    done
    ;;
  esac
done

#general benchmark.
for algo in m-way m-pass; do
  for benchmark in  "KD"; do # "ScaleStock" "ScaleRovio" "ScaleYSB" "ScaleDEBS" "AR" "RAR" "RAR2" "AD" "KD" "WS" "KD2" "WS2" "WS3" "WS4"
    case "$benchmark" in
    # Batch -a SHJ_JM_NP -n 8 -t 1 -w 1000 -e 1000 -l 10 -d 0 -Z 1
    "AR") #test arrival rate and assume both inputs have same arrival rate.
      id=0
      ## Figure 1
      ResetParameters
      FIXS=0 #varying both.
      ts=1 # stream case
      # step size should be bigger than nthreads
      for STEP_SIZE in 1600 3200 6400 12800 25600; do #128000
        #WINDOW_SIZE=$(expr $DEFAULT_WINDOW_SIZE \* $DEFAULT_STEP_SIZE / $STEP_SIZE) #ensure relation size is the same.
        echo relation size is $(expr $WINDOW_SIZE / $INTERVAL \* $STEP_SIZE)
        gap=$STEP_SIZE
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
        gap=$STEP_SIZE
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
      gap=1
      echo test varying key distribution 15 - 19
      distrbution=2 #varying zipf factor
      for skew in 0 0.4 0.8 1.2 1.6; do
        KimRun
        let "id++"
      done
      ;;
    "WS") #test window size
      id=20
      ## Figure 5
      ResetParameters
      FIXS=1
      STEP_SIZE=12800
      STEP_SIZE_S=12800
      gap=1
      echo test varying window size 20 - 24
      for WINDOW_SIZE in 500 1000 1500 2000 2500; do
        gap=$(($STEP_SIZE / 1000 * $WINDOW_SIZE))
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
      #    "Google") #Error yet.
      #      RSIZE=3747939
      #      SSIZE=11931801
      #      RPATH=/data1/xtra/datasets/google/users_key32_partitioned.csv
      #      SPATH=/data1/xtra/datasets/google/reviews_key32_partitioned.csv
      #      RKEY=1
      #      SKEY=1
      #      benchmarkRun
      #      ;;
      #    "Amazon") #Error yet.
      #      RSIZE=10
      #      SSIZE=10
      #      RPATH=/data1/xtra/datasets/amazon/amazon_question_partitioned.csv
      #      SPATH=/data1/xtra/datasets/amazon/amazon_answer_partitioned.csv
      #      RKEY=0
      #      SKEY=0
      #      benchmarkRun
      #      ;;
    esac
  done
done
