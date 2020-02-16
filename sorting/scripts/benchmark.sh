#!/bin/bash

cd ..
cmake .
make -j4

function benchmarkRun() {
  #####native execution
  echo "==benchmark:$benchmark -a $algo -n $Threads=="
  ./sorting -a $algo -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1
}

function Run() {
  #####native execution
  echo "==benchmark:$benchmark -a $algo -n $Threads=="
  ./sorting -a $algo -r $RSIZE -s $SSIZE -n $Threads
}

function KimRun() {
  #####native execution
  echo "==benchmark:$benchmark -a $algo -n $Threads #TEST:$id=="
  ./sorting -a $algo -t $ts -w $WINDOW_SIZE -e $STEP_SIZE -q $STEP_SIZE_S -l $INTERVAL -d $distrbution -z $skew -D $TS_DISTRIBUTION -Z $ZIPF_FACTOR -n $Threads -I $id  -W $FIXS
}

DEFAULT_WINDOW_SIZE=1000
DEFAULT_STEP_SIZE=100
function ResetParameters() {
  TS_DISTRIBUTION=0                # uniform time distribution
  ZIPF_FACTOR=0                    # uniform time distribution
  distrbution=0                    # unique
  skew=0                           # uniform key distribution
  INTERVAL=1                       # interval of 1. always..
  STEP_SIZE=$DEFAULT_STEP_SIZE     # arrival rate = 1000 / ms
  WINDOW_SIZE=$DEFAULT_WINDOW_SIZE #MS rel size = window_size / interval * step_size.
  STEP_SIZE_S=-1                   # let S has the same arrival rate of R.
  FIXS=0
}

# Configurable variables
# Generate a timestamp
algo=""
Threads=32
timestamp=$(date +%Y%m%d-%H%M)
output=test$timestamp.txt
for algo in m-way m-pass; do #
  RSIZE=1
  SSIZE=1
  RPATH=""
  SPATH=""
  RKEY=0
  SKEY=0
  RTS=0
  STS=0
  for benchmark in "Kim"; do #"Kim" # "Rovio" "Stock" "DEBS" "YSB"
    case "$benchmark" in
    # Batch -a SHJ_JM_NP -n 8 -t 1 -w 1000 -e 1000 -l 10 -d 0 -Z 1
    "Kim")
      id=0
      ## Figure 1
      ResetParameters
      echo test varying input arrival rate 0 - 4 # test (1) means infinite arrival rate (batch).
      ts=0                                       # batch case
      echo relation size is $(expr $WINDOW_SIZE / $INTERVAL \* $STEP_SIZE)
      KimRun
      let "id++"

      ts=1 # stream case
      # step size should be bigger than nthreads
      for STEP_SIZE in 100 1000 10000 100000; do
        WINDOW_SIZE=$(expr $DEFAULT_WINDOW_SIZE \* $DEFAULT_STEP_SIZE / $STEP_SIZE) #ensure relation size is the same.
        echo relation size is $(expr $WINDOW_SIZE / $INTERVAL \* $STEP_SIZE)
        KimRun
        let "id++"
      done

      ## Figure 2
      ResetParameters
      TS_DISTRIBUTION=2
      echo test varying zipf distribution timestamp 5 - 9
      for ZIPF_FACTOR in 0 0.2 0.4 0.8 1; do #
        KimRun
        let "id++"
      done

      #
      ## Figure 3
      ResetParameters
      TS_DISTRIBUTION=0
      echo test varying key distribution 10 - 15
      distrbution=0 #unique
      KimRun
      let "id++"

      distrbution=2 #varying zipf factor
      for skew in 0 0.2 0.4 0.8 1; do
        KimRun
        let "id++"
      done

      ## Figure 4
      ResetParameters
      echo test varying window size 16 - 18
      for WINDOW_SIZE in 1000 5000 10000; do
        KimRun
        let "id++"
      done

      ## Figure 5
      ResetParameters
      ts=0 # data at rest.
      echo test varying key distribution 19 - 24
      distrbution=0 #unique
      KimRun
      let "id++"

      distrbution=2 #zipf
      for skew in 0 0.2 0.4 0.8 1; do
        KimRun
        let "id++"
      done

      ## Figure 6
      ResetParameters
      ts=0 # data at rest.
      echo test varying window size 25 - 27
      for WINDOW_SIZE in 1000 5000 10000; do
        KimRun
        let "id++"
      done

      ## Figure 7
      FIXS=1
      echo test relative arrival rate 28 - 31
      ts=1 # stream case
      # step size should be bigger than nthreads
      STEP_SIZE_S=100000
      for STEP_SIZE in 100 1000 10000 100000; do
        WINDOW_SIZE=$(expr $DEFAULT_WINDOW_SIZE \* $DEFAULT_STEP_SIZE / $STEP_SIZE) #ensure relation size is the same.
        echo relation size is $(expr $WINDOW_SIZE / $INTERVAL \* $STEP_SIZE)
        KimRun
        let "id++"
      done

      ## Figure 8
      FIXS=1
      echo test relative arrival rate 32 - 35
      ts=1 # stream case
      # step size should be bigger than nthreads
      STEP_SIZE_S=100
      for STEP_SIZE in 100 1000 10000 100000; do
        WINDOW_SIZE=$(expr $DEFAULT_WINDOW_SIZE \* $DEFAULT_STEP_SIZE / $STEP_SIZE) #ensure relation size is the same.
        echo relation size is $(expr $WINDOW_SIZE / $INTERVAL \* $STEP_SIZE)
        KimRun
        let "id++"
      done
      ;;

    "DEBS")
      RSIZE=1000000
      SSIZE=1000000
      RPATH=/data1/xtra/datasets/DEBS/posts_key32_partitioned.csv
      SPATH=/data1/xtra/datasets/DEBS/comments_key32_partitioned.csv
      RKEY=0
      SKEY=0
      benchmarkRun
      ;;
      # Batch-Stream
    "YSB")
      RSIZE=1000
      SSIZE=300000
      RPATH=/data1/xtra/datasets/YSB/campaigns_32t.txt
      SPATH=/data1/xtra/datasets/YSB/ad_30s_32t.txt
      RKEY=0
      SKEY=0
      RTS=0
      STS=1
      benchmarkRun
      ;;
      # Stream
    "Rovio") #matches:
      RSIZE=290076
      SSIZE=290076
      RPATH=/data1/xtra/datasets/rovio/30s_32t.txt
      SPATH=/data1/xtra/datasets/rovio/30s_32t.txt
      RKEY=0
      SKEY=0
      RTS=3
      STS=3
      benchmarkRun
      ;;
    "Stock") #Error yet.
      RSIZE=149711
      SSIZE=196175
      RPATH=/data1/xtra/datasets/stock/cj_30s_32t.txt
      SPATH=/data1/xtra/datasets/stock/sb_30s_32t.txt
      RKEY=0
      SKEY=0
      RTS=1
      STS=1
      benchmarkRun
      ;;
    "Google") #Error yet.
      RSIZE=3747939
      SSIZE=11931801
      RPATH=/data1/xtra/datasets/google/users_key32_partitioned.csv
      SPATH=/data1/xtra/datasets/google/reviews_key32_partitioned.csv
      RKEY=1
      SKEY=1
      benchmarkRun
      ;;
    "Amazon") #Error yet.
      RSIZE=10
      SSIZE=10
      RPATH=/data1/xtra/datasets/amazon/amazon_question_partitioned.csv
      SPATH=/data1/xtra/datasets/amazon/amazon_answer_partitioned.csv
      RKEY=0
      SKEY=0
      benchmarkRun
      ;;
    esac
  done
done
