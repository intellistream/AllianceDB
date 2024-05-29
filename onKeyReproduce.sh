#/bin/bash
#assume you have already acquired the ubuntu 22.04
#1. install tool chain
sudo apt update
sudo apt install gcc g++ python3 pip cmake make -y
#2. install doxygen for docs
sudo apt-get install doxygen -y
sudo apt-get install graphviz -y
doxygen Doxyfile
# install lib torch, cpu only
pip3 install torch==1.13.0 torchvision torchaudio --index-url https://download.pytorch.org/whl/cpu
#other oython pkgs dor plotting
pip3 install matplotlib pandas numpy
# build
mkdir build
cd build
cmake -DCMAKE_PREFIX_PATH=`python3 -c 'import torch;print(torch.utils.cmake_prefix_path)'` ..
make -j24
# reproduce
cd benchmark
mkdir figures
mkdir results
cd scripts
./rerunAll.sh
cd ../../..
rm -rf results
rm -rf figures
cp -r build/benchmark/results .
cp -r build/benchmark/figures .
# now, the doc is in doc/html folder by using index.heml
# figures are in figures.
# raw results are in results.
