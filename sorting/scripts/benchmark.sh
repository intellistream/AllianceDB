#!/bin/bash

cd ..
cmake .
make -j4

function benchmarkRun() {
  #####native execution
  echo "==benchmark:$benchmark -a $algo -n $Threads=="
  ./sorting -a $algo -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads
}

function Run() {
  #####native execution
  echo "==benchmark:$benchmark -a $algo -n $Threads=="
  ./sorting -a $algo -r $RSIZE -s $SSIZE -n $Threads
}

function KimRun() {
  #####native execution
  echo "==benchmark:$benchmark -a $algo -n $Threads #TEST:$id=="
  ./sorting -a $algo -t $ts -w $WINDOW_SIZE -e $STEP_SIZE -l $INTERVAL -d $distrbution -z $skew -D $TS_DISTRIBUTION -Z $ZIPF_FACTOR -n $Threads -I $id
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
  TS_DISTRIBUTION=0 # uniform time distribution
  ZIPF_FACTOR=0     # uniform time distribution
  distrbution=2     # uniform key distribution
  skew=0            # uniform key distribution
  INTERVAL=1        # interval of 1.
  STEP_SIZE=1000    # arrival rate = 1000 / ms
  WINDOW_SIZE=1000  #MS rel size = window_size / interval * step_size.
  for benchmark in Kim; do #"Kim" "Stock" "DEBS" "YSB" #"Rovio" #"Google" "Amazon"
    case "$benchmark" in
    # Batch -a SHJ_JM_NP -n 8 -t 1 -w 1000 -e 1000 -l 10 -d 0 -Z 1
    "Kim")
      id=0
      ## Figure 1
      echo test varying input arrival rate 0 - 4 # test (1) means infinite arrival rate (batch).
      ts=0                                       # batch case
      #      KimRun
      let "id++"

      ts=1 # stream case
      # step size should be bigger than nthreads
      for STEP_SIZE in 100 1000 10000 100000; do
        WINDOW_SIZE=$(expr 1000 \* 1000 / $STEP_SIZE) #ensure relation size is the same.
        #        echo Figure 1 window size is $WINDOW_SIZE
        #        KimRun
        let "id++"
      done

      #      ## Figure 2
      TS_DISTRIBUTION=2
      WINDOW_SIZE=10000 #default
      STEP_SIZE=100     #default
      echo test varying zipf distribution timestamp 5 - 9
      for ZIPF_FACTOR in 0 0.2 0.4 0.8 1; do
        #        KimRun
        let "id++"
      done
      #
      ## Figure 3
      TS_DISTRIBUTION=2
      ZIPF_FACTOR=0.4
      echo test varying key distribution 10 - 15
      distrbution=0 #unique
#      KimRun
      let "id++"

      distrbution=2 #zipf
      for skew in 0 0.2 0.4 0.8 1; do
#        KimRun
        let "id++"
      done

      distrbution=2 #uniform
      skew=0.4
      ## Figure 4
      echo test varying window size 16 - 18
      for WINDOW_SIZE in 1000 10000 100000; do
#        KimRun
        let "id++"
      done

      ## Figure 5
      TS_DISTRIBUTION=0
      ZIPF_FACTOR=0
      echo test varying key distribution 19 - 24
      distrbution=0 #unique
      KimRun
      let "id++"

      distrbution=2 #zipf
      for skew in 0 0.2 0.4 0.8 1; do
        KimRun
        let "id++"
      done

      distrbution=2 #uniform
      skew=0.4
      ## Figure 6
      echo test varying window size 25 - 27
      for WINDOW_SIZE in 1000 10000 100000; do
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
      RPATH=/data1/xtra/datasets/YSB/campaigns_40t.txt
      SPATH=/data1/xtra/datasets/YSB/ad_30s_40t.txt
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
      RPATH=/data1/xtra/datasets/rovio/30s_40t.txt
      SPATH=/data1/xtra/datasets/rovio/30s_40t.txt
      RKEY=0
      SKEY=0
      RTS=3
      STS=3
      benchmarkRun
      ;;
    "Stock") #Error yet.
      RSIZE=149711
      SSIZE=196175
      RPATH=/data1/xtra/datasets/stock/cj_30s_40t.txt
      SPATH=/data1/xtra/datasets/stock/sb_30s_40t.txt
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
