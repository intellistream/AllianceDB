#!/bin/bash

CONTAINER_NAME=LWJ
PORT=1234
ROOT_PASSWD=pass1234
SHELL_FOLDER=$(cd "$(dirname "$0")";pwd)
docker build --build-arg DEFAULT_PASSWD=$ROOT_PASSWD -t clion/remote-cpp-env:0.5 -f $SHELL_FOLDER/Dockerfile.remote-cpp-env .
docker run -d --cap-add sys_ptrace -p127.0.0.1:$PORT:22 --name $CONTAINER_NAME clion/remote-cpp-env:0.5
ssh-keygen -f "$HOME/.ssh/known_hosts" -R "[localhost]:$PORT"
