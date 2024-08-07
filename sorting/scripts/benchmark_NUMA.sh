#!/bin/bash
#set -e
## Set L3 Cache according to your machine.
sed -i -e "s/#define L3_CACHE_SIZE [[:alnum:]]*/#define L3_CACHE_SIZE 26214400/g" ../utils/params.h

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
  echo "==benchmark:$benchmark -a $algo -t $ts -w $WINDOW_SIZE -e $STEP_SIZE -q $STEP_SIZE_S -l $INTERVAL -d $distrbution -z $skew -D $TS_DISTRIBUTION -Z $ZIPF_FACTOR -n $Threads -g $gap -P $DD -W $FIXS -I $id =="
  #echo 3 >/proc/sys/vm/drop_caches
  ../sorting -a $algo -t $ts -w $WINDOW_SIZE -e $STEP_SIZE -q $STEP_SIZE_S -l $INTERVAL -d $distrbution -z $skew -D $TS_DISTRIBUTION -Z $ZIPF_FACTOR -n $Threads -W $FIXS -g $gap -P $DD -I $id
}


function SetStockParameters() { #matches: 15595000. #inputs= 60527 + 77227
  ts=1 # stream case
  WINDOW_SIZE=1000
  RSIZE=60527
  SSIZE=77227
  RPATH=$exp_dir/datasets/stock/cj_1000ms_1t.txt
  SPATH=$exp_dir/datasets/stock/sb_1000ms_1t.txt
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
  RPATH=$exp_dir/datasets/rovio/1000ms_1t.txt
  SPATH=$exp_dir/datasets/rovio/1000ms_1t.txt
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
  RPATH=$exp_dir/datasets/YSB/campaigns_id.txt
  SPATH=$exp_dir/datasets/YSB/ad_events.txt
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
  RPATH=$exp_dir/datasets/DEBS/posts_key32_partitioned.csv
  SPATH=$exp_dir/datasets/DEBS/comments_key32_partitioned.csv
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
  sed -i -e "s/scalarflag [[:alnum:]]*/scalarflag 0/g" ../joins/joincommon.h
}

#recompile by default.
compile
# Configurable variables
# Generate a timestamp
timestamp=$(date +%Y%m%d-%H%M)
output=test$timestamp.txt

#general benchmark.
compile=0
for algo in m-way m-pass; do
  for benchmark in "LargeScaleStock" "LargeScaleRovio" "LargeScaleYSB" "LargeScaleDEBS"; do # "ScaleStock" "ScaleRovio" "ScaleYSB" "ScaleDEBS" "AR" "RAR" "AD" "KD" "WS"
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
      ts=0
      STEP_SIZE=160
      STEP_SIZE_S=160
      echo test DD 25 - 28
      for DD in 1 10 50 100; do
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
    "LargeScaleStock")
      id=58
      ResetParameters
      SetStockParameters
      echo test scalability of Stock 58 - 61
      for Threads in 4 8 16 32; do
        benchmarkRun
        let "id++"
      done
      ;;
    "LargeScaleRovio")
      id=62
      ResetParameters
      SetRovioParameters
      echo test scalability 62 - 65
      for Threads in 4 8 16 32; do
        benchmarkRun
        let "id++"
      done
      ;;
    "LargeScaleYSB")
      id=66
      ResetParameters
      SetYSBParameters
      echo test scalability 66 - 69
      for Threads in 4 8 16 32; do
        benchmarkRun
        let "id++"
      done
      ;;
    "LargeScaleDEBS")
      id=70
      ResetParameters
      SetDEBSParameters
      echo test scalability 70 - 73
      for Threads in 4 8 16 32; do
        benchmarkRun
        let "id++"
      done
      ;;
      #    "Google") #Error yet.
      #      RSIZE=3747939
      #      SSIZE=11931801
      #      RPATH=$exp_dir/datasets/google/users_key32_partitioned.csv
      #      SPATH=$exp_dir/datasets/google/reviews_key32_partitioned.csv
      #      RKEY=1
      #      SKEY=1
      #      benchmarkRun
      #      ;;
      #    "Amazon") #Error yet.
      #      RSIZE=10
      #      SSIZE=10
      #      RPATH=$exp_dir/datasets/amazon/amazon_question_partitioned.csv
      #      SPATH=$exp_dir/datasets/amazon/amazon_answer_partitioned.csv
      #      RKEY=0
      #      SKEY=0
      #      benchmarkRun
      #      ;;
    esac
  done
done

compile=1
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
