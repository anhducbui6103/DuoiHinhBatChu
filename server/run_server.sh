#!/bin/bash
gcc -o server ./*.c ./server_lib/*.c ../lib/*.c `mysql_config --cflags --libs` 