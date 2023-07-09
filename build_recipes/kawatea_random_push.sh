#!/bin/bash

CWD=`pwd`
cd ../mkut
g++ -O3 -std=c++17 kawatea_random_single.cpp
cp a.out $CWD