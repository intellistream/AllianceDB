# OoOJoin
Integrate PECJ as ooo operators into alliancedb
- Please also see PECJ_WITH_APPENDIX.pdf for the full paper with Appendix.
## How to Reproduce Our Experimental Results?

1. Everything will be automatically built and evaluated by calling the following command under **a sudo user** (You maybe asked to enter the password):

```shell
./onKeyReproduce.sh
```
This will also copy figures and results to `figures` and `results` folder at the root directory of this project for your convenience to find them.

### Prerequisite

1. System specification:

   | Component          | Description                            |
   | ------------------ |----------------------------------------|
   | Processor (w/o HT) | Intel(R) Xeon(R) Gold 6252 CPU, 24*2   |
   | L3 cache size      | 35.75MB                                |
   | Memory             | 384 GB                                 |
   | OS & Compiler      | Ubuntu22.04 (Physical or Docker) g++11 |

2. To make the scalability evaluation work as expected, please make sure that the core id 0~23 represent 24 physical cores at the same NUMA node (if there is some NUMA platform).

3. The torch installation command is designed for X64, and also works for ARM64, but may need some changes on other architectures like RISCV.

### Docker usage

1. Docker scripts which only set up os and dependencies (Tested on X64 and ARM64):

We provide docker scripts to install Ubuntu22.04 environment, please do the following to set it up
```shell
cd docker
./docker_est.sh
```
Upon seeing the docker command line, you may cd to home, proceed to **manually clone this repo** and then run the `onKeyReproduce.sh`.

2. Hands-free docker with precompiled program and its source (X64 only):

- First, download the image from https://drive.google.com/file/d/145958378seGGaDCS7-pOnDpIOvadsv1h/view?usp=sharing
- cd to the path of downloaded `pecj_x64.tar`, and run the following (may require sudo depending on your docker installation)

```shell
docker load --input pecj_x64.tar
docker run --name="pecj" -h OoOJoin -it  pecj:anomo
```

This will switch host command line into that of docker conatiner, namely `pecj`. You will find the sources located at 
`/home/OoOJoin/projects/OoOJoin`, and the binaries are pre-compiled at `/home/OoOJoin/projects/OoOJoin/build`.
You can then run 
```shell
cd /home/OoOJoin/projects/OoOJoin/
./onKeyReproduce.sh
```
which will 
   - recompile the project
   - reconduct the evaluations
   - copy the results and figures into `/home/OoOJoin/projects/OoOJoin/results` and `/home/OoOJoin/projects/OoOJoin/figures`

You may then exit and run `docker cp` to get the data and figures from container `pecj` by aforementioned paths.
:warning: We have removed the .git folder in docker images which contains our identification data, and source code inside dockers are lastly updated on July 14, 2023.
It will come back with avaliable `git pull` after the double-blind review is done.

If the older built and evaluation is not correctly overwritten by new results, please try the following inside your docker container `pecj`
```shell
cd /home/OoOJoin/projects/OoOJoin/
rm -rf build figures results
./onKeyReproduce.sh
```

### Third-party Lib

Third-party Libs will be automatically installed in scripts, which contains:

1. CMake, gcc/g++, make install, if already installed, make sure the CMake version > 3.10.0, and the major version of gcc/g++ is above 11.

```shell
sudo apt install -y cmake gcc g++ make
```

2. Tex font rendering (Optional):

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
pip3 install matplotlib pandas
```

4. Libtorch:

```shell
pip3 install torch==1.13.0 torchvision torchaudio --index-url https://download.pytorch.org/whl/cpu
```

### Configurations
Please refer to the `config*.csv` under each child directory of `scripts` folder, 
such as scripts/sec6_2StockQ1
it assigns the operator, dataset and other system configurations

### Real World Datasets
Please refer to `benchmark/datasets`
We have already make the datasets fitted into readable csv format for both human and machine, with <key, value, event time, arrival time, processed time>.

Logistic: 

​	benchmark/datasets/logistics/tr.csv

​	benchmark/datasets/logistics/ts.csv

Retail:

​	benchmark/datasets/retail/tr.csv

​	benchmark/datasets/retail/ts.csv

Rovio:

​	benchmark/datasets/rovio/rovio.csv (For both R and S)

Stock: 

​	benchmark/datasets/stock/cj_1000ms_1tLowDelayData_short.csv

​	benchmark/datasets/stock/sb_1000ms_1tHighDelayData_short.csv

### Results

All results are in `build/benchmark/results/`.

All figures are in `build/benchmark/figures`.

You can run the following to restart an evaluation round, all old data will be deleted
```shell
cd build/benchmark/scripts
./rerunAll.sh
```
## For Code Contributors
### How to build

(CUDA-related is only necessary if your pytorch has cuda, but it's harmless if you don't have cuda.)

### Create a docker for clion

Please refer to the .md in docker folder
The docker has minmal gcc/g++/cmake and libtorch, which is good enough.

### Build in shell

```shell
(export CUDACXX=/usr/local/cuda/bin/nvcc)
mkdir build && cd build
cmake -DCMAKE_PREFIX_PATH=`python3 -c 'import torch;print(torch.utils.cmake_prefix_path)'` ..
make 
```

### Tips for build in Clion

There are bugs in the built-in cmake of Clion, so you can not run
-DCMAKE_PREFIX_PATH=`python3 -c 'import torch;print(torch.utils.cmake_prefix_path)'`.
Following may help:

- Please run 'import torch;print(torch.utils.cmake_prefix_path)' manually first, and copy the path
- Paste the path to -DCMAKE_PREFIX_PATH=
- Manually set the environment variable CUDACXX as "/usr/local/cuda/bin/nvcc" in Clion's cmake settings
-

### Code Structure

- benchmark -- application code to use the generated shared library
- cmake -- cmake configuration files
- docsrc -- the source pictures to build docs
- include -- all the header files
- scripts -- python scripts to run automatic tests
- src -- corresponding source files, will generate a shared library
- test -- test code based on google test

### Local generation of the documents

You can also re-generate documents locally, if you have the doxygen and graphviz. Following are how to install them in ubuntu
21.10/22.04

```shell
sudo apt-get install doxygen
sudo apt-get install graphviz
```

Then, you can do

```shell
mkdir doc
doxygen Doxyfile
```

to get the documents in doc/html folder, and start at index.html
## Known issues

1. If you use Torch with cuda, the nvcc will refuse to work as it doesn't support c++20 yet. Therefore, we disabled the
   global requirement check of C++ 20, and only leave an "-std=c++20" option for g++. This will be fixed after nvidia
   can support c++20 in cuda.
2. The pytorch headers also conflict with gtest, so gtest is now excluded, we switch the test framework to header-only
   catch2
3. torch version 2.0.0 may fail on aarch64
