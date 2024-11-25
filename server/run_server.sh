#!/bin/bash
gcc -o server ./*.c ./server_lib/*.c `mysql_config --cflags --libs` && ./server