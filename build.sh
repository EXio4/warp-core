#!/bin/sh

em++ src/*.cxx src/App/*.cxx -Iinclude/ -O0 -s WASM=1 -o main.js

