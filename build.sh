#!/usr/bin/env bash

mkdir -p build

pushd build
g++ -std=c++17 -g -O0 -Wall -Wno-write-strings ../source/Main.cpp -o PrecedenceParser
popd
