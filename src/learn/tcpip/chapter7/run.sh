#!/bin/bash

./file_server &
./file_client

# or 
# ./file_server &
# nc 127.0.0.1 2001

# or
# ./file_server &
# telnet 127.0.0.1 2001