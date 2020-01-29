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
  ./hashing -a $algo -t $ts -w $WINDOW_SIZE -e $STEP_SIZE -l $INTERVAL -d $DISTRIBUTION -z $skew -Z $ZIPF_FACTOR -n $Threads -I $id
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
  for benchmark in Kim; do #"Kim" "Stock" "DEBS" "YSB" #"Rovio" #"Google" "Amazon"
    case "$benchmark" in
    # Batch -a SHJ_JM_NP -n 8 -t 1 -w 1000 -e 1000 -l 10 -d 0 -Z 1
    "Kim")
      id=0
      WINDOW_SIZE=100000 #MS

      echo test case with no timestamp
      ts=0

      for DISTRIBUTION in 0 2; do #id=1~2
        STEP_SIZE=1000
        INTERVAL=10
        skew=0
        KimRun
        let "id++"
      done

      for skew in 0 0.2 0.4 0.8 1; do #id= 3~7
        STEP_SIZE=1000
        INTERVAL=10
        DISTRIBUTION=2
        KimRun
        let "id++"
      done

      echo test with timestamp.
      ts=1

      echo test case 1: different key distribution
      for DISTRIBUTION in 0 1; do #id=5~6
        STEP_SIZE=1000
        INTERVAL=10
        ZIPF_FACTOR=0
        KimRun
        let "id++"
      done

      echo test case 2: for zipf distrbution
      for ZIPF_FACTOR in 0 0.2 0.4 0.8 1; do #id=7~11
        STEP_SIZE=1000
        INTERVAL=10
        DISTRIBUTION=2
        KimRun
        let "id++"
      done

      echo test case 3: distribution unique/nonunique
      # TODO: zipf distribution should be merged into this?
      for DISTRIBUTION in 0 1; do  #id=12~13
        STEP_SIZE=1000
        INTERVAL=10
        ZIPF_FACTOR=0
        KimRun
        let "id++"
      done

      echo test case 4: step size # step size should be bigger than nthreads
      for STEP_SIZE in 100 500 1000 2000; do #id=13~16
        INTERVAL=10
        DISTRIBUTION=0
        ZIPF_FACTOR=0
        KimRun
        let "id++"
      done

      echo test case 5: interval
      for INTERVAL in 10 100 500 1000; do  #id=17~20
        STEP_SIZE=1000
        DISTRIBUTION=0
        ZIPF_FACTOR=0
        KimRun
        let "id++"
      done

      echo test case 6: window size
      for WINDOW_SIZE in 1000 10000 50000 100000; do   #id=21 ~ 24
        INTERVAL=10
        STEP_SIZE=1000 # TODO: no sure if this should be smaller or narrow down the window size
        DISTRIBUTION=0
        ZIPF_FACTOR=0
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
