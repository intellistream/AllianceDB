#!/bin/bash

cd ..
cmake .
make -j4

function benchmarkRun() {
  #####native execution
  echo "==benchmark:$benchmark -a $algo -n $Threads=="
  ./hashing -a $algo -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads
}

function Run() {
  #####native execution
  echo "==benchmark:$benchmark -a $algo -n $Threads=="
  ./hashing -a $algo -r $RSIZE -s $SSIZE -n $Threads
}

function KimRun() {
  #####native execution
  echo "==benchmark:$benchmark -a $algo -n $Threads #TEST:$id=="
  ./hashing -a $algo -t $ts -w $WINDOW_SIZE -e $STEP_SIZE -l $INTERVAL -d $distrbution -z $skew -D $TS_DISTRIBUTION -Z $ZIPF_FACTOR -n $Threads -I $id
}

# Configurable variables
# Generate a timestamp
algo=""
Threads=36
timestamp=$(date +%Y%m%d-%H%M)
output=test$timestamp.txt
for algo in PRO NPO SHJ_JM_NP SHJ_JBCR_NP PMJ_JM_NP PMJ_JBCR_NP; do #SHJ_HS_NP PMJ_HS_NP  #RPJ_JM_NP RPJ_JBCR_NP RPJ_HS_NP
  RSIZE=1
  SSIZE=1
  RPATH=""
  SPATH=""
  RKEY=0
  SKEY=0
  RTS=0
  STS=0
  TS_DISTRIBUTION=0
  ZIPF_FACTOR=0
  for benchmark in Kim; do #"Kim" "Stock" "DEBS" "YSB" #"Rovio" #"Google" "Amazon"
    case "$benchmark" in
    # Batch -a SHJ_JM_NP -n 8 -t 1 -w 1000 -e 1000 -l 10 -d 0 -Z 1
    "Kim")
      id=0
      WINDOW_SIZE=100000 #MS

      echo test case with no timestamp
      ts=0
      echo test case 1: varying key distribution 1 ~ 2
      for distrbution in 0 2; do
        STEP_SIZE=1000
        INTERVAL=10
        skew=0
        KimRun
        let "id++"
      done

      echo test case 2: varying key distribution zipf factor 3 ~ 7
      for skew in 0 0.2 0.4 0.8 1; do
        STEP_SIZE=1000
        INTERVAL=10
        distrbution=2
        KimRun
        let "id++"
      done

      echo test with uniform timestamp.
      ts=1
      TS_DISTRIBUTION=0
      echo test case 3: varying key distribution zipf factor 8 ~ 9
      for distrbution in 0 2; do
        STEP_SIZE=1000
        INTERVAL=10
        skew=0
        KimRun
        let "id++"
      done

      echo test case 4: varying key distribution zipf factor 10 ~ 14
      for skew in 0 0.2 0.4 0.8 1; do
        STEP_SIZE=1000
        INTERVAL=10
        distrbution=2
        KimRun
        let "id++"
      done

      echo test case 5: varying step size 15 ~ 18 # step size should be bigger than nthreads
      for STEP_SIZE in 100 500 1000 2000; do
        INTERVAL=10
        distrbution=0
        skew=0
        KimRun
        let "id++"
      done

      echo test case 6: varying interval 19 ~ 22
      for INTERVAL in 10 100 500 1000; do
        STEP_SIZE=1000
        distrbution=0
        skew=0
        KimRun
        let "id++"
      done

      echo test case 7: varying window size 23 ~ 26
      for WINDOW_SIZE in 1000 10000 50000 100000; do
        INTERVAL=10
        STEP_SIZE=1000 # TODO: no sure if this should be smaller or narrow down the window size
        distrbution=0
        skew=0
        KimRun
        let "id++"
      done

      TS_DISTRIBUTION=2
      echo test case 8: zipf distribution timestamp 27 ~ 31
      for ZIPF_FACTOR in 0 0.2 0.4 0.8 1; do
        INTERVAL=10
        STEP_SIZE=1000 # TODO: no sure if this should be smaller or narrow down the window size
        distrbution=0
        skew=0
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
