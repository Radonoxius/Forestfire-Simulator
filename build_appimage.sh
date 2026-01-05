#! /bin/bash

cd simulator

mkdir native/build 2> /dev/null
cd native/build
clang --std=c23 -c ../compute.c -Wall -Werror -O3

llvm-ar -rcs libcompute.a compute.o
rm compute.o

cd ../..

cargo b --release --features appimage

cd ..

mv simulator/target/release/simulator Simulator.AppDir/usr/bin/simulator 2> /dev/null

export ARCH=$(uname -m)

cd Simulator.AppDir
ln -s usr/bin/simulator AppRun

ARCH=$ARCH appimagetool .

rm AppRun
cd usr/bin
rm simulator
cd ../..

chmod +x Simulator-$ARCH.AppImage
mkdir ../build 2> /dev/null
mv Simulator-$ARCH.AppImage ../build/

cd ..