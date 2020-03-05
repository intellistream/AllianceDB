#!/bin/bash

cd ..
cmake .
make -j4

function benchmarkRun() {
  #####native execution
  echo "==benchmark:$benchmark -a $algo -n $Threads=="
  echo 3 >/proc/sys/vm/drop_caches
  ./hashing -a $algo -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads -B 1 -t 1 -I $id
}

function Run() {
  #####native execution
  echo "==benchmark:$benchmark -a $algo -n $Threads=="
  echo 3 >/proc/sys/vm/drop_caches
  ./hashing -a $algo -r $RSIZE -s $SSIZE -n $Threads
}

function KimRun() {
  #####native execution
  echo "==benchmark:$benchmark -a $algo  #TEST:$id -n $Threads -t $ts -w $WINDOW_SIZE -e $STEP_SIZE -l $INTERVAL -d $distrbution -z $skew -D $TS_DISTRIBUTION -Z $ZIPF_FACTOR -n $Threads -I $id=="
  echo 3 >/proc/sys/vm/drop_caches
  ./hashing -a $algo -t $ts -w $WINDOW_SIZE -e $STEP_SIZE -q $STEP_SIZE_S -l $INTERVAL -d $distrbution -z $skew -D $TS_DISTRIBUTION -Z $ZIPF_FACTOR -n $Threads -I $id -W $FIXS
}

DEFAULT_WINDOW_SIZE=2000 #(ms) -- 2 seconds
DEFAULT_STEP_SIZE=100    # |tuples| per ms. -- 50K per seconds.
function ResetParameters() {
  TS_DISTRIBUTION=0                # uniform time distribution
  ZIPF_FACTOR=0                    # uniform time distribution
  distrbution=0                    # unique
  skew=0                           # uniform key distribution
  INTERVAL=1                       # interval of 1. always..
  STEP_SIZE=$DEFAULT_STEP_SIZE     # arrival rate = 1000 / ms
  WINDOW_SIZE=$DEFAULT_WINDOW_SIZE # MS rel size = window_size / interval * step_size.
  STEP_SIZE_S=-1                   # let S has the same arrival rate of R.
  FIXS=0
  ts=1 # stream case
  Threads=32
}

# Configurable variables
# Generate a timestamp
algo=""
Threads=32
timestamp=$(date +%Y%m%d-%H%M)
output=test$timestamp.txt
for algo in SHJ_JM_NP SHJ_JBCR_NP PMJ_JM_NP PMJ_JBCR_NP PRO NPO; do #
  RSIZE=1
  SSIZE=1
  RPATH=""
  SPATH=""
  RKEY=0
  SKEY=0
  RTS=0
  STS=0
  for benchmark in "AR" "AD" "KD" "WS" "KD2" "WS2" "RAR" "RAR2" "WS3" "WS4"; do #"Stock"  "Rovio" "YSB"  "DEBS" # "ScaleStock" "AD" "KD" "WS" "KD2" "WS2" "RAR" "RAR2" "WS3" "WS4"
    case "$benchmark" in
    # Batch -a SHJ_JM_NP -n 8 -t 1 -w 1000 -e 1000 -l 10 -d 0 -Z 1
    "AR") #test arrival rate
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
      for STEP_SIZE in 50 100 250 500; do #
        #WINDOW_SIZE=$(expr $DEFAULT_WINDOW_SIZE \* $DEFAULT_STEP_SIZE / $STEP_SIZE) #ensure relation size is the same.
        echo relation size is $(expr $WINDOW_SIZE / $INTERVAL \* $STEP_SIZE)
        KimRun
        let "id++"
      done
      ;;
    "AD") #test arrival distribution
      id=5
      ## Figure 2
      ResetParameters
      TS_DISTRIBUTION=2
      echo test varying timestamp distribution 5 - 9
      for ZIPF_FACTOR in 0 0.2 0.4 0.8 1; do #
        KimRun
        let "id++"
      done
      ;;
    "KD") #test key distribution
      id=10
      ## Figure 3
      ResetParameters
      echo test varying key distribution 10 - 15
      distrbution=0 #unique
      KimRun
      let "id++"

      distrbution=2 #varying zipf factor
      for skew in 0 0.2 0.4 0.8 1; do
        KimRun
        let "id++"
      done
      ;;
    "WS") #test window size
      id=16
      ## Figure 4
      ResetParameters
      echo test varying window size 16 - 18
      for WINDOW_SIZE in 1000 5000 10000; do
        KimRun
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
      KimRun
      let "id++"

      distrbution=2 #zipf
      for skew in 0 0.2 0.4 0.8 1; do
        KimRun
        let "id++"
      done
      ;;
    "WS2") #test window size when data at rest.
      id=25
      ## Figure 6
      ResetParameters
      ts=0 # data at rest.
      echo test varying window size 25 - 27
      for WINDOW_SIZE in 1000 5000 10000; do
        KimRun
        let "id++"
      done
      ;;
    "RAR") #test relative arrival rate when S is large
      id=28
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
      ;;
    "RAR2") #test relative arrival rate when S is small
      id=32
      ## Figure 8
      FIXS=1
      echo test relative arrival rate 32 - 35
      ts=1 # stream case
      # step size should be bigger than nthreads
      # remember to fix the relation size of S.
      STEP_SIZE_S=100
      for STEP_SIZE in 100 1000 10000 100000; do
        WINDOW_SIZE=$(expr $DEFAULT_WINDOW_SIZE \* $DEFAULT_STEP_SIZE / $STEP_SIZE) #ensure relation size is the same.
        echo relation size is $(expr $WINDOW_SIZE / $INTERVAL \* $STEP_SIZE)
        KimRun
        let "id++"
      done
      ;;
    "WS3") #test window size for extra large size of window
      id=36
      ## Figure 4 extra
      ResetParameters
      echo test varying window size 36
      for WINDOW_SIZE in 100000; do
        KimRun
        let "id++"
      done
      ;;
    "WS4") #test window size for extra large size of window when data at rest
      id=37
      ## Figure 6 extra
      ResetParameters
      ts=0 # data at rest.
      echo test varying window size 37
      for WINDOW_SIZE in 100000; do
        KimRun
        let "id++"
      done
      ;;
    "Stock")
      id=38
      ts=1 # stream case
      RSIZE=194341
      SSIZE=240148
      RPATH=/data1/xtra/datasets/stock/cj_60s_1t.txt
      SPATH=/data1/xtra/datasets/stock/sb_60s_1t.txt
      RKEY=0
      SKEY=0
      RTS=1
      STS=1
      benchmarkRun
      ;;
    "Rovio") #matches:
      id=39
      ts=1 # stream case
      RSIZE=580700
      SSIZE=580700
      RPATH=/data1/xtra/datasets/rovio/60s_1t.txt
      SPATH=/data1/xtra/datasets/rovio/60s_1t.txt
      RKEY=0
      SKEY=0
      RTS=3
      STS=3
      benchmarkRun
      ;;
    "YSB")
      id=40
      ts=1 # stream case
      RSIZE=1000
      SSIZE=600000
      RPATH=/data1/xtra/datasets/YSB/campaigns_1t.txt
      SPATH=/data1/xtra/datasets/YSB/ad_60s_1t.txt
      RKEY=0
      SKEY=0
      RTS=0
      STS=1
      benchmarkRun
      ;;
    "DEBS")
      id=41
      ts=1 # stream case
      RSIZE=1000000
      SSIZE=1000000
      RPATH=/data1/xtra/datasets/DEBS/posts_key32_partitioned.csv
      SPATH=/data1/xtra/datasets/DEBS/comments_key32_partitioned.csv
      RKEY=0
      SKEY=0
      RTS=0
      STS=0
      benchmarkRun
      ;;
    "ScaleStock")
      id=42
      ts=1 # stream case
      RSIZE=194341
      SSIZE=240148
      RPATH=/data1/xtra/datasets/stock/cj_60s_1t.txt
      SPATH=/data1/xtra/datasets/stock/sb_60s_1t.txt
      RKEY=0
      SKEY=0
      RTS=1
      STS=1
      echo test scalability of Stock 42 - 45
      for Threads in 1 8 16 32; do
        benchmarkRun
        let "id++"
      done
      ;;
    "ScaleRovio")
      id=46
      ts=1 # stream case
      RSIZE=580700
      SSIZE=580700
      RPATH=/data1/xtra/datasets/rovio/60s_1t.txt
      SPATH=/data1/xtra/datasets/rovio/60s_1t.txt
      RKEY=0
      SKEY=0
      RTS=3
      STS=3
      echo test scalability 46 - 49
      for Threads in 1 8 16 32; do
        benchmarkRun
        let "id++"
      done
      ;;
    "ScaleYSB")
      id=50
      ts=1 # stream case
      RSIZE=1000
      SSIZE=600000
      RPATH=/data1/xtra/datasets/YSB/campaigns_1t.txt
      SPATH=/data1/xtra/datasets/YSB/ad_60s_1t.txt
      RKEY=0
      SKEY=0
      RTS=0
      STS=1
      echo test scalability 50 - 53
      for Threads in 1 8 16 32; do
        benchmarkRun
        let "id++"
      done
      ;;
    "ScaleDEBS")
      id=54
      ts=1 # stream case
      RSIZE=1000000
      SSIZE=1000000
      RPATH=/data1/xtra/datasets/DEBS/posts_key32_partitioned.csv
      SPATH=/data1/xtra/datasets/DEBS/comments_key32_partitioned.csv
      RKEY=0
      SKEY=0
      RTS=0
      STS=0
      echo test scalability 54 - 57
      for Threads in 1 8 16 32; do
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