#!/bin/bash
g++ -g -O0 -Wall --std=c++11 -DUSE_DEFAULT_ADDRESS ./src/gethostbyname.cc -o gethostbyname
g++ -g -O0 -Wall --std=c++11 -DUSE_DEFAULT_ADDRESS ./src/gethostbyaddr.cc -o gethostbyaddr