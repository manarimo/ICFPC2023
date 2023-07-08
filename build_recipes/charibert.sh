#!/bin/bash

CWD=`pwd`
cd ../amylase/charibert
g++ -O3 -std=c++17 block_pillar.cpp
cp a.out $CWD