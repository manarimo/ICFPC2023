#!/bin/bash

CWD=`pwd`
cd ../amylase/charibert
g++ -O3 -std=c++17 main.cpp
cp a.out $CWD