# # # !bin/bash

input=$1

USE_TEST="hello"

if [ "$input" = "" ]
then
	if [ ! -d "./bin" ]; then
        mkdir bin
    fi
    
    cd bin \
    && g++ -g -O0 -Wall --std=c++11 ../src/sock_type.cc -o sock_type \
    && g++ -g -O0 -Wall --std=c++11 ../src/get_buf.cc -o get_buf \
    && g++ -g -O0 -Wall --std=c++11 ../src/set_buf.cc -o set_buf \
    && g++ -g -O0 -Wall --std=c++11 -DUSE_DEFAULT_ADDRESS ../src/echo_server.cc -o echo_server \
    && g++ -g -O0 -Wall --std=c++11 -DUSE_DEFAULT_ADDRESS ../src/echo_client.cc -o echo_client \
    && g++ -g -O0 -Wall --std=c++11 -DUSE_DEFAULT_ADDRESS ../src/reuseaddr_server.cc -o reuseaddr_server \
    && g++ -g -O0 -Wall --std=c++11 ../src/create_file.cc -o create_file \
    && g++ -g -O0 -Wall --std=c++11 \
        -DUSE_DEFAULT_ADDRESS \
        -DSOL_SOCKET_REUSEADDR \
        -DIPPROTO_TCP_NODELAY \
        ../src/no_nagle_server.cc -o no_nagle_server \
    && g++ -g -O0 -Wall --std=c++11 -DUSE_DEFAULT_ADDRESS ../src/no_nagle_client.cc -o no_nagle_client 

fi

if [ "$input" = "clear" ]
then
	if [ -d "./bin" ]; then
        # 推荐使用 trash-put 代替 rm
        trash-put -rf ./bin 
    fi
fi
