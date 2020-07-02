#!/bin/sh

em++ -std=c++17 src/*.cxx src/App/*.cxx -Iinclude/ -O2 -g -s USE_PTHREADS=1 -s PTHREAD_POOL_SIZE=12 -s TOTAL_MEMORY=1024MB -s WASM=1 -o main.js

