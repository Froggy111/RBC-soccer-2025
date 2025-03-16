# Docker ARM Cross Build Scripts

This repository contains Dockerfiles and scripts to cross-compile common 
C and C++ libraries and programs for the Raspberry Pi (both 32 and 64 bit):

 - **Zlib**: compression library (OpenSSL and Python dependency)
 - **OpenSSL**: cryptography library (Python dependency)
 - **FFI**: foreign function interface (Python dependency, used to call C functions using ctypes)
 - **Bzip2**: compression library (Python dependency)
 - **GNU ncurses**: library for text-based user interfaces (Python dependency, used for the console)
 - **GNU readline**: library for line-editing and history (Python dependency, used for the console)
 - **GNU dbm**: library for key-value data (Python dependency)
 - **SQLite**: library for embedded databases (Python dependency)
 - **UUID**: library for unique identifiers (Python dependency)
 - **libX11**: X11 protocol client library (Tk dependency)
 - **Tcl/Tk**: graphical user interface toolkit (Python/Tkinter dependency)
 - **Python 3.10.4**: Python interpreter and libraries
 - **ZBar**: Bar and QR code decoding library
 - **Raspberry Pi Userland**: VideoCore GPU drivers
 - **VPX**: VP8/VP9 codec SDK
 - **x264**: H.264/MPEG-4 AVC encoder
 - **Xvid**: MPEG-4 video codec
 - **FFmpeg**: library to record, convert and stream audio and video
 - **OpenBLAS**: linear algebra library (NumPy dependency)
 - **NumPy**: multi-dimensional array container for Python (OpenCV dependency)
 - **SciPy**: Python module for mathematics, science, and engineering
 - **OpenCV 4.5.5**: computer vision library and Python module
 - **GDB Server**: on-target remote debugger
 - **GCC 11.1.0**: C, C++ and Fortran compilers
 - **GNU Make**: build automation tool
 - **Ninja**: build system
 - **CMake**: build system
 - **Distcc**: distributed compiler wrapper (uses your computer to speed up compilation on the Pi)
 - **CCache**: compiler cache
 - **cURL**: tool and library for transferring data over the network (Git dependency)
 - **Git**: version control system

Builds are carried out using Docker for reproducibility and isolation from your 
main system.


## Documentation
 
The documentation is still a work in progress, but parts of it are already available here:  
[**Documentation**](https://tttapa.github.io/Pages/Raspberry-Pi/C++-Development/index.html)

To see all the options, execute `./build.sh` script without any arguments.

***

## Pulling the libraries from Docker Hub

If you don't want to build everything from scratch (it takes quite a while to compile everything),
you can download the pre-built version from [**Docker Hub**](https://hub.docker.com/r/tttapa/)
using the `--pull` option of the `build.sh` script.

## Building everything yourself

To build everything yourself, you can just run the `build.sh` script without the `--pull` option.

The following is just an overview of the different steps that are executed by that script.

### The cross-compilation Toolchain

Crosstool-NG is used to build a modern GCC toolchain that runs on your computer and generates binaries for the Raspberry Pi.
This is much faster than compiling everything on the Pi itself.

The toolchain is built by the 
[**tttapa/docker-arm-cross-toolchain**](https://github.com/tttapa/docker-arm-cross-toolchain) repository.  
Instructions on how to customize the toolchain can be found on the [Building the Cross-Compilation Toolchain](https://tttapa.github.io/Pages/Raspberry-Pi/C++-Development/Building-The-Toolchain.html)
page of the documentation.

### Cross-compiling the necessary libraries

The toolchain is then used to compile all libraries for the Raspberry Pi.  
These libraries are installed in two locations:
1. in the **“sysroot”**: this is a folder where all system files and libraries are installed that are required for the build process of other libraries
2. in the **“staging area”**: this is the folder that will be copied to the SD card of the Raspberry Pi later. It contains the libraries that we built, but not the system files (because those are already part of the Ubuntu/Raspbian installation of the Pi).

Everything is installed in `/usr/local/`, so it shouldn't interfere with the software installed by your package manager.
[`userland`](https://github.com/raspberrypi/userland) is an exception, it's installed in `/opt/vc/`.

If you just want to know how to cross-compile a specific package, have a look at the scripts in the
[`toolchain/docker/merged/cross-build/install-scripts`](toolchain/docker/merged/cross-build/install-scripts) folder.  

### Exporting the toolchain, sysroot and staging area

Once all components have been built, they have to be extracted from the Docker build containers, and installed to the correct locations.  
You can extract everything using the `--extract` option of the `build.sh` script.

To test the newly cross-compiled binaries, just extract the contents of the right `staging-rpix-xxx-linux-gnuxxx.tar` archive to the root folder of the Raspberry Pi:

**Do NOT extract the staging area to the root directory of your computer, you will destroy your system!**

```sh
# Copy the staging area archive to the Pi
# This command may take a while, depending on the speed of your SD card
scp ./staging-rpi3-aarch64.tar RPi3:/tmp
# Install everything to the root of the filesystem
# (will only install to /usr/local and /opt)
# Enter the sudo password of the Pi if necessary
# This command may take a while, depending on the speed of your SD card
ssh -t RPi3 "sudo tar xf /tmp/staging-rpi3-aarch64.tar -C / --strip-components=1"
# Configure dynamic linker run-time bindings so newly installed libraries
# are found by the linker
# Enter the sudo password of the Pi if necessary
ssh -t RPi3 "sudo ldconfig"
```

### Using the cross-compiled libraries for your own projects

You can now use the toolchain and the libraries in the sysroot to build your
own software, either by using the appropriate CMake toolchain file, or by 
manually specifying the right compiler and the `--sysroot` flag.

For more information, see the [Documentation](https://tttapa.github.io/Pages/Raspberry-Pi/C++-Development).