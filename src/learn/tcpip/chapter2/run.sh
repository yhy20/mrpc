#!/bin/bash

./hello_server &
./tcp_client

# or 
#./hello_server &
# nc 127.0.0.1 2001

# or
# ./hello_server &
# telnet 127.0.0.1 2001