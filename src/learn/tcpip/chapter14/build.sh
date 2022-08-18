# # # !bin/bash

input=$1

USE_TEST="hello"

if [ "$input" = "" ]
then
	if [ ! -d "./bin" ]; then
        mkdir bin
    fi
    
    cd bin \
    && g++ -g -O0 -Wall --std=c++11 -DUSE_DEFAULT_ADDRESS ../news_sender.cc -o news_sender \
    && g++ -g -O0 -Wall --std=c++11 -DUSE_DEFAULT_ADDRESS ../news_receiver.cc -o news_receiver

fi

if [ "$input" = "clear" ]
then
	if [ -d "./bin" ]; then
        # 推荐使用 trash-put 代替 rm
        trash-put -rf ./bin 
    fi
fi
