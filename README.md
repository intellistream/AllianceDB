# How to Reproduce Our Experimental Results?

All of our experiments can be automatically reproduced by calling a few pre-prepared scripts.

## Prerequisite

1. cmake > 3.10.0.
2. Profiling only supports Intel CPUs.
3. You may need to reset pmu once at first run, some pcm profiling results may be incorrect at first time.
4. Prepare cpu-mapping in `cpu-mapping.txt`. 
5. real world datasets will be moved to the `exp_dir/datasets` automatically by scripts.
6. Our program should be running as root with two params exp_dir and l3_cache_size: `sudo bash run_all.sh -d /data1/xtra -c 19922944`.
7. You can run any subset of the experiment sections by modifying the exp_section in `run_all.sh`.

## Third-party Lib (will be automatically installed in scripts)

1. cmake install

```
sudo apt install -y cmake
```

1. tex font rendering:

```shell
sudo apt install -y texlive-fonts-recommended texlive-fonts-extra
sudo apt install -y dvipng
sudo apt install -y font-manager
sudo apt install -y cm-super
```

2. python3:

```shell
sudo apt install -y python3
sudo apt install -y python3-pip
pip3 install numpy
pip3 install matplotlib
```

3.  NUMA library

```shell
sudo apt install -y libnuma-dev
```

4. Zlib

```shell
sudo apt install -y zlib1g-dev
```

5. python-tk

```shell
sudo apt install -y python-tk
```

6. perf

```shell
sudo apt install -y linux-tools-common
sudo apt install -y linux-tools-`uname -r` # XXX is the kernel version of your linux, use uname -r to check it. e.g. 4.15.0-91-generic
sudo echo -1 > /proc/sys/kernel/perf_event_paranoid # if permission denied, try to run this at root user.
sudo modprobe msr
```

### Configurations

Default parameters:

| Parameters          | Default              | Description                                   |
| ------------------- | -------------------- | --------------------------------------------- |
| exp_dir             | /data1/xtra          | Path to save all results and generate figures |
| L3_CACHE_SIZE       | 19922944 (19MB)      | Size of l3 cache                              |
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

## Results

All results are in `exp_dir/results/`.

All figures are in `exp_dir/results/figures`.
