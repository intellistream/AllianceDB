# How to Reproduce Our Experimental Results?

All of our experiments can be automatically reproduced by calling a few pre-prepared scripts.

## Third-party Lib

1. tex font rendering:

```shell
sudo apt-get install texlive-fonts-recommended texlive-fonts-extra
sudo apt-get install dvipng
```

2. python3:

```shell
sudo apt-get install python3
pip3 install numpy
pip3 install matplotlib
```

3.  NUMA library

```shell
sudo apt-get install -y libnuma-dev
```

4. Zlib

```shell
sudo apt install zlib1g-dev
```

5. python-tk

```shell
sudo apt-get install python-tk
```

### Prerequisite

1. Profiling only supports Intel CPUs.
2. Prepare cpu-mapping, and need to configure the path in hashing/cpu_mapping.txt. Do remember we have modified the path of CUSTOM_CPU_MAPPING, which reads this file and generate mapping of cpus.
3. configure the results output path, ensure you have permissions to access and modify the files inside.
4. configure cache size at utils/params.h
5. where to run scripts? need to be at scripts folder.
6. prepare real world datasets, move them to the `expDir/datasets`, currently, the datasets path in scripts are fixed, need to update to configurable

### Setup

1. TODO: test whether we can still run all experiments without Stock.

2. TODO: add some instructions on how to run the scripts, such as how to configure cache size and other profiling strategies. Also, we may need to mention we need to run scripts at sorting/hashing folder.

   | Parameters                       | Default         | Description                                   |
   | -------------------------------- | --------------- | --------------------------------------------- |
   | expDir                           | /data1/xtra     | path to save all results and generate figures |
   | L3_CACHE_SIZE (rather important) | 20971520 (20MB) | size of l3 cache                              |
   | PERF_COUNTERS/NO_PERF_COUNTERS   | No              | Unknown                                       |
   | NO_TIMING/TIMING                 | No              | turn on/off breakdown timer                   |
   | compile                          | 1               | compile the framework                         |
   | Threads                          | 1 2 4 8         | experiments threads settings                  |

3. TODO: run hashing first, and then run sorting, or we can write another scripts to launch all those scripts.

### Step 1

git clone the repo.

### Step 2
open hashing/scripts/benchmark.sh
   At Line 19, set L3 cache size according to your machine specification;
   run the benchmark.sh to conduct experiments.

Remember to adjust the following lines in the scripts according to your needs.
  1. Line 299 to 427. 
     conducts most experiments for four datasets and micro-datasets.
     At Line 296, set profile_breakdown=1 if we want to measure time breakdown.
  2. Line 432 to 477.
     conducts the scalability test for four datasets. In our paper, we only show the results of YSB as an example.
  3. Line 495 to 562.
      conducts the cache miss profiling study using YSB as an example. 
      For this test, you must run the script with sudo, i.e., "sudo ./benchmark.sh".
      After the test, you have to manually fill in the results to "profile_ysb_partition.py" and "profile_ysb_probe.py" corresponding to the test IDs.
  4. Line 566 to 673.
     conducts experiments for algorithm parameters tuning.
### Step 3
Repeat the experiments for sorting/scripts/benchmark.sh

### Step 4
call ./draw.sh to generate figures.

## Datasets

We have 4 real datasets that are compressed in [datasets.tar.gz](https://drive.google.com/file/d/1DJIES8AEIQSfw9HF4xxgZ9OHFiUxZijw/view?usp=sharing). Download and call tar -zvxf datasets.tar.gz to unzip those datasets.

We extracted the useful columns of those datasets, the one is joined key and another is timestamp.

DEBS: 

​	comments_key32_partitioned.csv: user_id|comments_payload

​	posts_key32_partitioned.csv: user_id|posts_payload

​	Reproduce results: open hashing/scripts/benchmark.sh, change the data path in line 274 and 275. 

YSB:

​	ad_events.txt: campaign_id|timestamp

​	campaigns_id.txt: campaign_id|campaign_payload

​	Reproduce results: open hashing/scripts/benchmark.sh, change the data path in line 246 and 247. 

Rovio:

​	1000ms_1t.txt: combined_id|payload|price|timestamp

​	Reproduce results: open hashing/scripts/benchmark.sh, change the data path in line 260 and 261. 

Stock: 

​	This dataset is not open-sourced out of privacy, the data structure is stockid|timestamp.



