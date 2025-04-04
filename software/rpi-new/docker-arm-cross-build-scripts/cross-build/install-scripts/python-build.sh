#!/usr/bin/env bash

set -ex

# Download
version=3.10.4
URL="https://www.python.org/ftp/python/$version/Python-$version.tgz"
pushd "${DOWNLOADS}"
wget -N "$URL"
popd

# Extract
tar xzf "${DOWNLOADS}/Python-$version.tgz"
pushd Python-$version

# Configure
export PKG_CONFIG_LIBDIR=/usr/local/lib:/usr/lib
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/lib/pkgconfig
./configure \
    --prefix="/usr/local" \
    --with-openssl="/usr/local"
cat config.log

# Build
make -j$(($(nproc) * 2))

# Install
sudo make altinstall

# Cleanup
popd
sudo rm -rf Python-$version