## Parallelizing Intra-Window Join on Multicores: An Experimental Study

This library introduces `AllianceDB` , an open-source benchmark suite for evaluating and improving stream operation algorithms on modern hardwares. It includes all codes and experimental traces to reproduce the experiments from our [SIGMOD 2021](https://dl-acm-org.libproxy1.nus.edu.sg/doi/10.1145/3448016.3452793) paper called [Parallelizing Intra-Window Join on Multicores: An Experimental Study](https://dl-acm-org.libproxy1.nus.edu.sg/doi/10.1145/3448016.3452793).

This is a static dump of the code created in 2022 to be uploaded to the ACM website and permanently hosted. Please refer to our [Github Repository on AllianceDB](https://github.com/intellistream/AllianceDB).

### Project Structure

- `run_all.sh` script for one-click experiment environment setup and results reproducing.
- `sorting/` folder containing scripts and source code for sort-merge join algorithms.
- `hashing/` folder containing scripts and source code for hash join algorithms.
- `docs/` folder containing useful documentations and figures for understanding both of our framework and datasets.
- `pcm*.cfg` configuration files to setup event counters used for [PCM](https://github.com/opcm/pcm) profiling.
- `cpu-mappings.txt` configuration file for setting up CPU affinity of threads..

### How to Reproduce Our Experimental Results?

The environment will be automatically configured and all of our experiments can be automatically reproduced by calling the following command with **root privileges**:

```shell
# -d : the experiment results directory
# -c : the L3 cache size of the current CPU
sudo bash run_all.sh -d /data1/xtra -c 19922944
```

#### Prerequisite

1. System specification:

   | Component          | Description                                              |
   | ------------------ | -------------------------------------------------------- |
   | Processor (w/o HT) | Intel(R) Xeon(R) Gold 6126 CPU, 2 (socket) * 12 * 2.6GHz |
   | L3 cache size      | 19MB                                                     |
   | Memory             | 64GB, DDR4 2666 MHz                                      |
   | OS & Compiler      | Linux 4.15.0, compile with g++ O3                        |

2. The profiling can be only supported on Intel CPUs, which provide diverse hardware counters.

3. Some PCM profiling results may be incorrect at the first run.

4. You can run any subset of the experiment sections individually by modifying the `exp_section` in `run_all.sh`.

5. To run with more than 8 threads, it needs to update the cpu-mapping in `cpu-mapping.txt`. 

#### Third-party Lib

Third-party Libs will be automatically installed in scripts, which contains:

1. CMake install, if CMake already installed, make sure the CMake version > 3.10.0.

```shell
sudo apt install -y cmake
```

2. Tex font rendering:

```shell
sudo apt install -y texlive-fonts-recommended texlive-fonts-extra
sudo apt install -y dvipng
sudo apt install -y font-manager
sudo apt install -y cm-super
```

3. Python3:

```shell
sudo apt install -y python3
sudo apt install -y python3-pip
pip3 install numpy
pip3 install matplotlib
```

4. NUMA library

```shell
sudo apt install -y libnuma-dev
```

5. Zlib

```shell
sudo apt install -y zlib1g-dev
```

6. python-tk

```shell
sudo apt install -y python-tk
```

7. perf

```shell
sudo apt install -y linux-tools-common
sudo apt install -y linux-tools-`uname -r` # XXX is the kernel version of your linux, use uname -r to check it. e.g. 4.15.0-91-generic
sudo echo -1 > /proc/sys/kernel/perf_event_paranoid # if permission denied, try to run this at root user.
sudo modprobe msr # load msr driver
```

#### Configurations

Inside the `run_all.sh`, there are three parameters can be manually configured according to the experiment requirements.

Default parameters are shown as below:

| Parameters          | Default              | Description                                   |
| ------------------- | -------------------- | --------------------------------------------- |
| exp_dir             | /data1/xtra          | Path to save all results and generate figures |
| L3_CACHE_SIZE       | 19922944 (19MB)      | Size of L3 cache                              |
| Experiment Sections | All (e.g. APP_BENCH) | All experiments shown in our paper            |

#### Real World Datasets

Real world datasets will be downloaded and moved to the `exp_dir/datasets` automatically by scripts.

We have 4 real datasets that are compressed in [datasets.tar.gz](https://www.dropbox.com/s/64z4xtpyhhmhojp/datasets.tar.gz). Download and call `tar -zvxf datasets.tar.gz` to unzip those datasets.

We extracted the useful columns of those datasets, the one is joined key and another is timestamp. The detailed file descriptions of the datasets are shown as below:

| Dataset | Files description                                            |
| ------- | ------------------------------------------------------------ |
| DEBS    | `comments_key32_partitioned.csv` : user_id | comments_payload<br />`posts_key32_partitioned.csv` : user_id \| posts_payload |
| YSB     | `ad_events.txt` : campaign_id | timestamp<br />`campaigns_id.txt` : campaign_id \| campaign_payload |
| Rovio   | `1000ms_1t.txt`  : combined_id \| payload\|price \| timestamp |
| Stock   | `cj_1000ms_1t.txt` : stockid | timestamp <br />`sb_1000ms_1t.txt` : stockid \| timestamp |

#### Results

All results are in `exp_dir/results/`.

All figures are in `exp_dir/results/figures`.