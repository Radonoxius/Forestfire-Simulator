#! /bin/bash

mkdir native/build 2> /dev/null
cd native/build
clang-19 --std=c23 -c ../compute.c -Wall -Werror -O3

llvm-ar-19 -rcs libcompute.a compute.o
rm compute.o

cd ../..

cargo clean

cargo r --release 15 230 t