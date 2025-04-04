#!/usr/bin/env bash

set -ex

# Download
version=openssl-3.0.2
URL="https://github.com/openssl/openssl/archive/$version.tar.gz"
pushd "${DOWNLOADS}"
wget -N "$URL"
popd

# Extract
tar xzf "${DOWNLOADS}/$version.tar.gz"
pushd openssl-$version

# Configure
./config \
    --prefix="/usr/local"

# Build
make -j$(($(nproc) * 2))

# Install
sudo make install_sw

# Cleanup
popd
rm -rf openssl-$version
