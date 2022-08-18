# # # !bin/bash

input=$1

USE_TEST="hello"

if [ "$input" = "" ]
then
	if [ ! -d "./bin" ]; then
        mkdir bin
    fi
    
    cd bin \
    && g++ -g -O0 -Wall --std=c++11 -DUSE_DEFAULT_ADDRESS ../oob_send.cc -o oob_send \
    && g++ -g -O0 -Wall --std=c++11 \
        -DUSE_DEFAULT_ADDRESS \
        -DSOL_SOCKET_REUSEADDR \
        ../oob_recv.cc -o oob_recv 

fi

if [ "$input" = "clear" ]
then
	if [ -d "./bin" ]; then
        # 推荐使用 trash-put 代替 rm
        trash-put -rf ./bin 
    fi
fi
