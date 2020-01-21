#!/bin/bash

cd ..
cmake .
make -j4

function benchmarkRun {
		#####native execution
		echo "==benchmark:$benchmark -a $algo -n $Threads=="
    ./hashing -a $algo -r $RSIZE -s $SSIZE -R $RPATH -S $SPATH -J $RKEY -K $SKEY -L $RTS -M $STS -n $Threads
}

function Run {
		#####native execution
		echo "==benchmark:$benchmark -a $algo -n $Threads=="
    ./hashing -a $algo -r $RSIZE -s $SSIZE -n $Threads
}

# Configurable variables
# Generate a timestamp
algo=""
RSIZE=1
SSIZE=1
RPATH=""
SPATH=""
RKEY=0
SKEY=0
RTS=0
STS=0
Threads=40
timestamp=$(date +%Y%m%d-%H%M)
output=test$timestamp.txt
for algo in PRO NPO SHJ_JM_NP SHJ_JBCR_NP PMJ_JM_NP PMJ_JBCR_NP #SHJ_HS_NP PMJ_HS_NP  #RPJ_JM_NP RPJ_JBCR_NP RPJ_HS_NP
do
  for benchmark in "Rovio" #"Kim" "Stock" "Rovio" "DEBS" "YSB" #"Google" "Amazon"
  do
    case "$benchmark" in
      # Batch
      "Kim")
        RSIZE=1000000
        SSIZE=1000000
        Run
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
        "Google")#Error yet.
        RSIZE=3747939
        SSIZE=11931801
        RPATH=/data1/xtra/datasets/google/users_key32_partitioned.csv
        SPATH=/data1/xtra/datasets/google/reviews_key32_partitioned.csv
        RKEY=1
        SKEY=1
        benchmarkRun
    ;;
        "Amazon")#Error yet.
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