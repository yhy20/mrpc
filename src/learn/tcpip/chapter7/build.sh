#!bin/bash

input=$1

if [ "$input" = "" ]
then
	if [ ! -d "./bin" ]; then
        mkdir bin
    fi
    
    cd bin \
    && g++ -g -O0 -Wall --std=c++11 -DUSE_DEFAULT_ADDRESS -D ./src/file_server.cc -o file_server \
    && g++ -g -O0 -Wall --std=c++11 -DUSE_DEFAULT_ADDRESS ./src/file_client.cc -o file_client
fi

if [ "$input" = "clear" ]
then
	if [ -d "./bin" ]; then
        # 推荐使用 trash-put 代替 rm
        trash-put -rf ./bin 
    fi
fi

