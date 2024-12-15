#!/bin/bash
gcc -o client ./*.c ../lib/*.c `mysql_config --cflags --libs` `sdl2-config --cflags --libs` -lSDL2_image -lSDL2_ttf && ./client 