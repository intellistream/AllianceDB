#!/bin/bash

# Configurable variables
output=test.csv
# Generate a timestamp
timestamp=$(date +%Y%m%d-%H%M)

cd ..
cmake .
make -j4

for benchmark in "STOCK"
do
  case "$benchmark" in
    "STOCK")
    ./hashing \
    -a        \
    SHJ_JM_NP \
    -r        \
    15010  \
    -s        \
    15280  \
    -R        \
    /home/xtra/data/datasets/stock/cj_key32_partitioned.csv \
    -S  \
    /home/xtra/data/datasets/stock/sb_key32_partitioned.csv \
    -J  \
    0   \
    -K  \
    0   \
    -L  \
    1   \
    -M  \
    1   \
    >> output.txt
  ;;
  esac
done
