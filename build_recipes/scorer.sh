#!/bin/bash

CWD=`pwd`
cd ../amylase/score
g++ -O3 -std=c++17 -I../../library main.cpp
cp a.out $CWD