export CC=/home/tony/Platforms/bananapi/buildroot-2021.11.1/output/host/bin/arm-buildroot-linux-gnueabihf-gcc
export CXX=/home/tony/Platforms/bananapi/buildroot-2021.11.1/output/host/bin/arm-buildroot-linux-gnueabihf-g++
cmake .. -DENABLE_UNIT_TESTS=OFF

