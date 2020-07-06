#!/bin/sh

em++ --bind -std=c++17 \
    src/*.cxx src/App/*.cxx -Iinclude/ \
    -O3  -g \
    -s USE_PTHREADS=1 -s PTHREAD_POOL_SIZE=14 \
    -s INITIAL_MEMORY=1024MB -s ALLOW_MEMORY_GROWTH=1 \
    -s WASM=1 \
    -Wcast-align -Wover-aligned -s WARN_UNALIGNED=1 \
    -o main.js

