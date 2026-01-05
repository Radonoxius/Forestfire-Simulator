#! /bin/bash

cd simulator

mkdir native/build 2> /dev/null
cd native/build
x86_64-w64-mingw32-clang --std=c23 -c ../compute.c -O3

x86_64-w64-mingw32-llvm-ar -rcs libcompute.a compute.o
rm compute.o

cd ../..

cargo clean

cargo b --target=x86_64-pc-windows-gnullvm --release

mkdir ../build 2> /dev/null
mv target/x86_64-pc-windows-gnullvm/release/simulator.exe ../build/
cp -r shaders ../build/

cd ..