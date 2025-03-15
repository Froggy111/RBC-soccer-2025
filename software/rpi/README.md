# Cross-Compilation for RPI Setup
https://github.com/abhiTronix/raspberry-pi-cross-compilers/wiki/64-Bit-Native-Compiler:-Installation-Instructions

1. Download the file from [here](https://sourceforge.net/projects/raspberry-pi-cross-compilers/files/Bonus%20Raspberry%20Pi%20GCC%2064-Bit%20Toolchains/Raspberry%20Pi%20GCC%2064-Bit%20Native-Compiler%20Toolchains/Bookworm/GCC%2014.2.0/), unzip it and place it in `rpi`

2. Install `aarch64-linux-gnu` through whatever means your operating system allows for.

3. Add symlinks [using this script](https://github.com/abhiTronix/raspberry-pi-cross-compilers/wiki/64-Bit-Native-Compiler:-Installation-Instructions#1-setup-important-symlinks), which you might have to modify based on where you placed the compiler.

4. Cross-compile away!