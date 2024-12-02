#!/bin/bash
gcc -o client ./*.c ../lib/*.c `mysql_config --cflags --libs` && ./client