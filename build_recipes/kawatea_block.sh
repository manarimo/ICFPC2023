#!/bin/bash

CWD=`pwd`
cd ../kawatea
g++ -O3 -std=c++17 block.cpp
cp a.out $CWD