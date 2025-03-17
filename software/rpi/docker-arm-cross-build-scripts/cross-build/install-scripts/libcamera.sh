#!/usr/bin/env bash

set -ex

# Define libcamera version and download URL
version=0.4.0
URL="https://github.com/libcamera-org/libcamera/archive/refs/tags/v${version}.tar.gz"

# Download the libcamera tarball into the DOWNLOADS folder
pushd "${DOWNLOADS}"
wget -N "$URL" -O libcamera-${version}.tar.gz
popd

# Extract the tarball and enter the source directory
tar xzf "${DOWNLOADS}/libcamera-${version}.tar.gz"
pushd libcamera-${version}

# Set up pkg-config so that it finds libraries in the Raspberry Pi sysroot.
# (Assumes cross-pkg-config is available and configures PKG_CONFIG_PATH accordingly.)
. cross-pkg-config

# Configure the build with Meson.
# Here we assume a Meson cross file exists at ../meson-crossfile.ini.
# Adjust the --prefix if you want a different install location on the target.
meson setup build \
    --cross-file ../meson-crossfile.ini \
    --prefix /usr/local \
    -Dbuildtype=release

# Build using Ninja (Meson’s default backend)
ninja -C build

# Install the compiled files into both the sysroot and the staging area.
# The DESTDIR variable makes sure that the install paths become DESTDIR + /usr/local.
DESTDIR="${RPI_SYSROOT}" ninja -C build install
DESTDIR="${RPI_STAGING}" ninja -C build install

# Cleanup: leave the source directory and remove it to keep the build environment tidy.
popd
rm -rf libcamera-${version}