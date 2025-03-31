#!/usr/bin/env bash

set -ex

# Version to install
version="1.6.3"  # Recent stable version of libgpiod
URL="https://git.kernel.org/pub/scm/libs/libgpiod/libgpiod.git/snapshot/libgpiod-${version}.tar.gz"

# Create download directory if it doesn't exist
mkdir -p "${DOWNLOADS}"

# Download libgpiod
pushd "${DOWNLOADS}"
wget -N "$URL" -O "libgpiod-${version}.tar.gz"
popd

# Extract
tar xzf "${DOWNLOADS}/libgpiod-${version}.tar.gz"

# Setup pkg-config for cross-compilation
. cross-pkg-config

# Build libgpiod
pushd "libgpiod-${version}"

# Run autogen.sh to generate the configure script
./autogen.sh

# Configure with autotools
# Set environment variables to prevent rpl_malloc issues
export ac_cv_func_malloc_0_nonnull=yes
export ac_cv_func_realloc_0_nonnull=yes

./configure --enable-tools=yes --prefix=/usr/local \
    --host=${HOST_TRIPLE} \
    --with-sysroot="${RPI_SYSROOT}" \
    CFLAGS="-I${RPI_SYSROOT}/usr/include" \
    LDFLAGS="-L${RPI_SYSROOT}/usr/lib"

# Use multiple cores for faster build
num_cores=$(nproc)
make -j$num_cores

# Install to sysroot and staging
make install DESTDIR="${RPI_SYSROOT}"
make install DESTDIR="${RPI_STAGING}"

# Cleanup
popd
rm -rf "libgpiod-${version}"

echo "libgpiod ${version} built and installed successfully"