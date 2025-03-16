#!/usr/bin/env bash

set -ex

# Download
version=1.10.2
URL="https://github.com/ninja-build/ninja/archive/v$version.tar.gz"
pushd "${DOWNLOADS}"
wget -N "$URL" -O ninja-$version.tar.gz
popd

# Extract
tar xzf "${DOWNLOADS}/ninja-$version.tar.gz"

# Configure
pushd ninja-$version
cmake -S. -Bbuild \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="/usr/local"

# Build
cmake --build build -j

# Install
sudo cp -a build/ninja "/usr/local/bin"

# Cleanup
popd
rm -rf ninja-$version