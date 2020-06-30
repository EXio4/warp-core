#!/bin/sh

em++ -std=c++17 src/*.cxx src/App/*.cxx -Iinclude/ -O2 -g -s ALLOW_MEMORY_GROWTH=1 -s WASM=1 -o main.js

