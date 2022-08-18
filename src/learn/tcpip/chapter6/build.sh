#!/bin/bash
g++ -g -O0 -Wall --std=c++11 -DUSE_DEFAULT_ADDRESS -DUSE_DEBUG_MODE ./src/uecho_server.cc -o uecho_server
g++ -g -O0 -Wall --std=c++11 -DUSE_DEFAULT_ADDRESS ./src/uecho_client.cc -o uecho_client
g++ -g -O0 -Wall --std=c++11 -DUSE_DEFAULT_ADDRESS ./src/uecho_con_client.cc -o uecho_con_client
g++ -g -O0 -Wall --std=c++11 -DUSE_DEFAULT_ADDRESS -DUSE_DEBUG_MODE ./src/bound_host1.cc -o bound_host1
g++ -g -O0 -Wall --std=c++11 -DUSE_DEFAULT_ADDRESS ./src/bound_host2.cc -o bound_host2