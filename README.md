# Forestfire-Simulator
Simulates a simplistic forest fire using `OpenCL`!

## What's it about?
This program simulates a forest fire on an *iGPU (Integrated GPU)*.

NOTE: THIS PROJECT WAS TESTED ON INTEGRATED GPUs (Intel graphics and ARM Mali-G610). IT MAY NOT WORK PROPERLY ON DISCRETE GPUs OR OTHER DEVICES.

You give an NxN (N = tree grid dimension) square matrix of trees initially, which contains live trees and some burning trees (ratio being controlled by the starting factor).

Starting factor (SF) is an 8 bit unsigned integer. A value of 255 means all trees are alive initially and 0 means all trees start off burning. Mathematically:

SF = 255 - ((# burning trees)/(N ^ 2) * 255)

A tree catches fire in the next iteration with a probability of 0.25 if at least one of its neighbouring trees is on fire.
Burning trees become dead in the next iteration.

The simulation ends when there are no burning trees left.

Epoch count is the number of iterations until the simulation ends.

## OpenCL features used
You need to have an iGPU that supports `OpenCL v2.0` or higher. You will also need the official drivers for that.

Also make sure that the iGPU supports `C11 style atomic integer operations` and `SVM` memory.

The program might crash if these features are not supported. **Use a different device in such cases!**

## Building from source
On Linux, you can build the entire project by using the provided build-scripts.

The build-script (build.sh) needs `clang`, `llvm`, `rustc` and `cargo` to compile the project for linux (host machine, aarch64 or x64).

This project can also be built for windows. To build for the windows target (x64) using the provided script, youll need the `x64-windows-gnullvm` rust target. Make sure that the `x64-MinGW64-LLVM` toolchain is also installed in a standard-system directory or is in `PATH`. Youll also require OpenCL headers and the OpenCL-ICD import library/dll. Additionally, you may have to modify `build_windows.sh` and the `rustflags` for the windows target.

OTHER TARGETS ARE NOT SUPPORTED!

This project uses `Rust 2024 edition` and `C23`.

**Hope it was fun!**