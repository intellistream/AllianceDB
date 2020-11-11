#!/bin/bash
#./sorting/scripts/benchmark.sh
#./hashing/scripts/benchmark.sh
#./hashing/scripts/draw.sh

exp_dir="/data1/xtra"
cp pcm* $exp_dir

#TODO: all shell should add a input parameter for config

cd ./sorting/scripts || exit
bash benchmark_pcm.sh
cd - || exit
cd ./hashing/scripts || exit
bash benchmark_pcm.sh

cd - || exit
cd ./sorting/scripts || exit
bash benchmark_perf.sh
cd - || exit
cd ./hashing/scripts || exit
bash benchmark_perf.sh
bash draw.sh