#!/bin/bash
g++ -g -O0 -Wall --std=c++11 -DUSE_DEFAULT_ADDRESS -DUSE_DEBUG_MODE ./src/hello_server.cc -o hello_server
g++ -g -O0 -Wall --std=c++11 -DUSE_DEFAULT_ADDRESS ./src/tcp_client.cc -o tcp_client
