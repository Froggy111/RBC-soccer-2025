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

# Determine the architecture
case "${HOST_TRIPLE}" in
    aarch64* ) OPENSSL_ARCH="linux-aarch64" ;;
    armv?*   ) OPENSSL_ARCH="linux-armv4" ;;
    *        ) echo "Unknown architecture ${HOST_TRIPLE}" && exit 1 ;;
esac

# Configure
. cross-pkg-config
./Configure \
    --prefix="/usr/local" \
    --cross-compile-prefix="${HOST_TRIPLE}-" \
    --release \
    --with-zlib-include="${RPI_SYSROOT}/usr/include" \
    --with-zlib-lib="${RPI_SYSROOT}/usr/lib" \
    "${OPENSSL_ARCH}" \
    "--sysroot=${RPI_SYSROOT}"

# Build
make -j$(($(nproc) * 2))

# Install
make install_sw DESTDIR="${RPI_SYSROOT}"
make install_sw DESTDIR="${RPI_STAGING}"

# Cleanup
popd
rm -rf openssl-$version
