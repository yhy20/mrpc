#!bin/bash

input=$1

if [ "$input" = "" ]
then
	if [ ! -d "./bin" ]; then
        mkdir bin
    fi
    
    cd bin \
    && g++ -g -O0 -Wall --std=c++11 -DUSE_DEFAULT_ADDRESS -DREUSE_SOCKET_ADDR ./src/hello_server.cc -o hello_server \
    && g++ -g -O0 -Wall --std=c++11 -DUSE_DEFAULT_ADDRESS ./src/hello_client.cc -o hello_client \
    && g++ -g -O0 -Wall --std=c++11 ./src/low_open.cc -o low_open \
    && g++ -g -O0 -Wall --std=c++11 ./src/low_read.cc -o low_read \
    && g++ -g -O0 -Wall --std=c++11 ./src/fd_seri.cc -o fd_seri
fi

if [ "$input" = "clear" ]
then
	if [ -d "./bin" ]; then
        # 推荐使用 trash-put 代替 rm
        trash-put -rf ./bin 
    fi
fi
