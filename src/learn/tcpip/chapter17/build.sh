# !bin/bash

input=$1

if [ "$input" = "" ]
then
	if [ ! -d "./bin" ]; then
        mkdir bin
    fi
    
    cd bin \
    && g++ -g -O0 -Wall --std=c++11 -DUSE_DEFAULT_ADDRESS -DSOL_SOCKET_REUSEADDR ../echo_epollserver.cc -o echo_epollserver \
    && g++ -g -O0 -Wall --std=c++11 -DUSE_DEFAULT_ADDRESS ../echo_epollclient.cc -o echo_epollclient

fi

if [ "$input" = "clear" ]
then
	if [ -d "./bin" ]; then
        # 推荐使用 trash-put 代替 rm
        trash-put -rf ./bin 
    fi
fi
