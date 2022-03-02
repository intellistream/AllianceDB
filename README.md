# Lazy Window Join

# Quick start

```shell
cd tools
sh rmote-debug-setup.sh
```

> then use CLion remote debug with port: `1234`, username: `root`, password: `pass1234`.

> Change the `CONTAINER_NAME`, `PORT` and `ROOT_PASSWD` if needed.

## Quick install G++11 on ubuntu older than hirsute

```shell
sudo add-apt-repository 'deb http://mirrors.kernel.org/ubuntu hirsute main universe'
sudo apt-get update
sudo apt install gcc-11 g++-11
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 11
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-11 11
```

## Requires Log4cxx native

```shell
sudo apt-get install -y liblog4cxx-dev
```

## Code Structure

- benchmark -- application code to use the generated shared library
- cmake -- cmake configuration files
- docs -- any documents
- include -- all the header files
- src -- corresponding source files, will generate a shared library
- test -- test code based on google test
- tools -- script to start a remote-debug environment that contains all required libs.

## How to build?
###Native Compile
mkdir build
cd build && cmake ..
make
###Cross Compile
mkdir CROSS_BUILD
cd CROSS_BUILD
export CC= {Full path and name of your cross C compiler}
export CXX= {Full path and name of your cross CPP compiler}
cmake .. -DENABLE_UNIT_TESTS=OFF
make
-please make sure your cross compiler also has the log4cxx
