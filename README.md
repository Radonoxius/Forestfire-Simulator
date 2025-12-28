# Forestfire-Simulator
Simulates a simplistic forest fire using `OpenCL`!

## What's it about?
This program simulates a forest fire on a *GPU (or some other device)*.

You give an NxN (N = tree grid dimension) square matrix of trees initially, which contains live trees and some burning trees (ratio being controlled by the starting factor).

Starting factor is an 8 bit unsigned integer. A value of 255 means all trees are alive initially and 0 means all trees start off burning.

A tree catches fire in the next iteration with a probability of 0.25 if at least one of its neighbouring trees is on fire.
Burning trees become dead in the next iteration.

The simulation ends when there are no burning trees left.

Epoch count is the number of iterations until the simulation ends.

## OpenCL features used
You need to have a device that supports `OpenCL v2.0` or higher. You will also need the official drivers for the said device.

Also make sure that the device supports `C11 style atomic integer operations` and `SVM` memory (coarse/fine grained).

The program might crash if these features are not supported. **Use a different device in such case!**

## Building from source
On Linux, you can build the entire project by using the provided build-script.

The build-script(build.sh) needs `clang`, `llvm`, `rustc` and `cargo` to compile the project.

This project uses `Rust 2024 edition` and `C23`.

**Hope it was fun!**
