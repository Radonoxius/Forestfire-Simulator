#! /bin/bash

cd simulator

mkdir native/build 2> /dev/null
cd native/build
clang --std=c23 -c ../compute.c -Wall -Werror -O3

llvm-ar -rcs libcompute.a compute.o
rm compute.o

cd ../..

cargo b --release

cd ..