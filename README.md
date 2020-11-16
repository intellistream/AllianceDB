# How to Reproduce Our Experimental Results?

All of our experiments can be automatically reproduced by calling a few pre-prepared scripts.

## TODO

1. correct name in all scripts (e.g., drawing which figure.).
2. clean all data path, do not use fixed "/data1/xtra/...", both in scripts and code.  - roughly done, need to be tested.
3. move all control variables into a dedicated header file "control.hpp". Currently, they are scattered in multiple files including common_functions.h perf_counter.h, params.h
4. draw figure scripts has not automated yet.
6. create a auto-deploy scripts in the end.
7. Auto install all required lib and tools. Auto config all required configurations.

## Third-party Lib

1. tex font rendering:

```shell
sudo apt-get install texlive-fonts-recommended texlive-fonts-extra
sudo apt-get install dvipng
sudo apt install font-manager
sudo apt-get install cm-super
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

6. perf

```shell
sudo apt install linux-tools-common
sudo apt install linux-tools-XXX # XXX is the kernel version of your linux, use uname -a to check it. e.g. 4.15.0-91-generic
sudo echo -1 > /proc/sys/kernel/perf_event_paranoid
sudo modprobe msr
```

### Prerequisite

1. Profiling only supports Intel CPUs.
2. Prepare cpu-mapping, and need to configure the path in hashing/cpu_mapping.txt. 
3. create `exp_dir` and configure the results output path `exp_dir` in `run_all.sh`.
4. configure cache size at `run_all.sh`.
5. prepare real world datasets, move them to the `exp_dir/datasets`, currently, the datasets path in scripts are fixed, need to update to configurable

```shell
exp_dir="/data1/xtra"
wget https://www.dropbox.com/s/64z4xtpyhhmhojp/datasets.tar.gz
tar -zvxf datasets.tar.gz
mv datasets $exp_dir
```

6. Our program should be running as root: `sudo bash run_all.sh`
7. Default parameters:

| Parameters          | Default              | Description                                   |
| ------------------- | -------------------- | --------------------------------------------- |
| exp_dir             | /data1/xtra          | Path to save all results and generate figures |
| L3_CACHE_SIZE       | 20971520 (20MB)      | Size of l3 cache                              |
| Experiment Sections | All (e.g. APP_BENCH) | All experiments shown in our paper            |

## Datasets

We have 4 real datasets that are compressed in [datasets.tar.gz](https://www.dropbox.com/s/64z4xtpyhhmhojp/datasets.tar.gz). Download and call tar -zvxf datasets.tar.gz to unzip those datasets.

We extracted the useful columns of those datasets, the one is joined key and another is timestamp.

DEBS: 

​	comments_key32_partitioned.csv: user_id|comments_payload

​	posts_key32_partitioned.csv: user_id|posts_payload

YSB:

​	ad_events.txt: campaign_id|timestamp

​	campaigns_id.txt: campaign_id|campaign_payload

Rovio:

​	1000ms_1t.txt: combined_id|payload|price|timestamp

Stock: 

​	cj_1000ms_1t.txt: stockid|timestamp

​	sb_1000ms_1t.txt: stockid|timestamp



