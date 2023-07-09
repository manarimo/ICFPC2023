#!/bin/bash

CWD=`pwd`
cd ../kawatea
g++ -O3 -std=c++17 block_pillar_iterate.cpp
cp a.out $CWD