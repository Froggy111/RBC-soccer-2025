#!/usr/bin/env bash

set -ex

# Download
version=4.11.0
URL="https://github.com/opencv/opencv/archive/${version}.tar.gz"
URL_CONTRIB="https://github.com/opencv/opencv_contrib/archive/${version}.tar.gz"

# Create download directory if it doesn't exist
mkdir -p "${DOWNLOADS}"

# Download main OpenCV and contrib modules
pushd "${DOWNLOADS}"
wget -N "$URL" -O "opencv-${version}.tar.gz"
wget -N "$URL_CONTRIB" -O "opencv_contrib-${version}.tar.gz"
popd

# Extract
tar xzf "${DOWNLOADS}/opencv-${version}.tar.gz"
tar xzf "${DOWNLOADS}/opencv_contrib-${version}.tar.gz"

# Create build directory
mkdir -p "opencv-${version}/build-arm"

# Check for ARM architecture and set NEON/VFPV3 flags
case "${HOST_TRIPLE}" in
    aarch64* ) 
        OPENCV_ENABLE_NEON=ON
        OPENCV_ENABLE_VFPV3=OFF
        ;;
    armv8* )   
        OPENCV_ENABLE_NEON=ON
        OPENCV_ENABLE_VFPV3=OFF
        ;;
    armv7* )   
        OPENCV_ENABLE_NEON=ON
        OPENCV_ENABLE_VFPV3=ON
        ;;
    armv6* )   
        OPENCV_ENABLE_NEON=OFF
        OPENCV_ENABLE_VFPV3=OFF
        ;;
    *      )   
        echo "Unknown architecture ${HOST_TRIPLE}"
        exit 1
        ;;
esac

# Setup pkg-config for cross-compilation
. cross-pkg-config

# Build OpenCV
pushd "opencv-${version}/build-arm"

# Configure with CMake
cmake \
    -DCMAKE_TOOLCHAIN_FILE=../platforms/linux/arm-gnueabi.toolchain.cmake \
    -DGNU_MACHINE="${HOST_TRIPLE}" \
    -DCMAKE_SYSTEM_PROCESSOR="${HOST_ARCH}" \
    -DCMAKE_SYSROOT="${RPI_SYSROOT}" \
    -DCMAKE_INSTALL_PREFIX="/usr/local" \
    -DCMAKE_BUILD_TYPE=Release \
    -DOPENCV_EXTRA_MODULES_PATH="../../opencv_contrib-${version}/modules" \
    -DENABLE_NEON=${OPENCV_ENABLE_NEON} \
    -DENABLE_VFPV3=${OPENCV_ENABLE_VFPV3} \
    -DOPENCV_ENABLE_NONFREE=ON \
    -DWITH_JPEG=ON \
    -DWITH_PNG=ON \
    -DWITH_TIFF=ON \
    -DWITH_OPENEXR=OFF \
    -DWITH_FFMPEG=ON \
    -DWITH_V4L=ON \
    -DWITH_LIBV4L=ON \
    -DWITH_GSTREAMER=OFF \
    -DBUILD_opencv_python2=OFF \
    -DBUILD_opencv_python3=OFF \
    -DBUILD_JAVA=OFF \
    -DBUILD_PERF_TESTS=OFF \
    -DBUILD_TESTS=OFF \
    -DBUILD_EXAMPLES=OFF \
    -DOPENCV_GENERATE_PKGCONFIG=ON \
    -DCV_ENABLE_INTRINSICS=ON \
    -DCMAKE_FIND_ROOT_PATH="${RPI_SYSROOT}" \
    -DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER \
    -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY \
    -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY \
    -DCMAKE_FIND_ROOT_PATH_MODE_PACKAGE=ONLY \
    -DCMAKE_C_FLAGS="-I${RPI_SYSROOT}/usr/local/include -I${RPI_SYSROOT}/opt/vc/include" \
    -DCMAKE_CXX_FLAGS="-I${RPI_SYSROOT}/usr/local/include -I${RPI_SYSROOT}/opt/vc/include" \
    -DCMAKE_EXE_LINKER_FLAGS="-L${RPI_SYSROOT}/usr/local/lib -L${RPI_SYSROOT}/opt/vc/lib" \
    .. \
    || { cat CMakeFiles/CMakeError.log && exit 1; }

# Use multiple cores for faster build
num_cores=$(nproc)
make -j$num_cores

# Install to sysroot and staging
make install DESTDIR="${RPI_SYSROOT}"
make install DESTDIR="${RPI_STAGING}"

# Cleanup
popd
rm -rf "opencv-${version}/build-arm"

echo "OpenCV ${version} built and installed successfully"