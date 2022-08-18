#!bin/bash

input=$1

if [ "$input" = "" ]
then
	if [ ! -d "./bin" ]; then
        mkdir bin
    fi
    
    cd bin \
    && g++ -g -O0 -Wall --std=c++11 -DUSE_DEFAULT_ADDRESS -DREUSE_SOCKET_ADDR ../src/echo_server.cc -o echo_server \
    && g++ -g -O0 -Wall --std=c++11 -DUSE_DEFAULT_ADDRESS ../src/echo_client.cc -o echo_client
fi

if [ "$input" = "clear" ]
then
	if [ -d "./bin" ]; then
        # 推荐使用 trash-put 代替 rm
        trash-put -rf ./bin 
    fi
fi
