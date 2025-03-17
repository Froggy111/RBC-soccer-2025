#!/usr/bin/env bash

set -ex

# Download and install GLib for cross-compilation
version="2.76.4"  # Use a more stable version with better subproject support
download_url="https://download.gnome.org/sources/glib/2.76/glib-${version}.tar.xz"

pushd "${DOWNLOADS}"
if [ ! -f "glib-${version}.tar.xz" ]; then
    wget "${download_url}"
fi
if [ ! -d "glib-${version}" ]; then
    tar -xf "glib-${version}.tar.xz"
fi
popd

# Create a working copy
rm -rf glib-build
cp -r "${DOWNLOADS}/glib-${version}" glib-build
pushd glib-build

# Source cross-pkg-config to set pkg-config to find libraries in the sysroot
. cross-pkg-config

sudo apt-get update

# Install libffi for cross-compilation
# Switch to official release tarball which includes pre-generated configure script
libffi_version="3.4.4"
libffi_url="https://github.com/libffi/libffi/releases/download/v${libffi_version}/libffi-${libffi_version}.tar.gz"

pushd "${DOWNLOADS}"
if [ ! -f "libffi-${libffi_version}.tar.gz" ]; then
    wget "${libffi_url}"
fi
if [ ! -d "libffi-${libffi_version}" ]; then
    tar -xf "libffi-${libffi_version}.tar.gz"
fi
popd

# Build and install libffi for cross-compilation
pushd "${DOWNLOADS}/libffi-${libffi_version}"

# Configure with disabled static trampolines to avoid the open_temp_exec_file error
./configure --host=${HOST_TRIPLE} \
    --prefix=/usr/local \
    --with-sysroot=${RPI_SYSROOT} \
    --disable-exec-static-tramp

make -j$(($(nproc) * 2))
make DESTDIR="${RPI_SYSROOT}" install
make DESTDIR="${RPI_STAGING}" install
popd

# Create meson cross file
cat > glib.meson.ini <<EOF
[binaries]
c = '${HOST_TRIPLE}-gcc'
cpp = '${HOST_TRIPLE}-g++'
ar = '${HOST_TRIPLE}-ar'
strip = '${HOST_TRIPLE}-strip'
pkgconfig = 'pkg-config'

[built-in options]
c_args = ['-O2', '-pipe', '--sysroot=${RPI_SYSROOT}']
c_link_args = ['--sysroot=${RPI_SYSROOT}']
cpp_args = ['-O2', '-pipe', '--sysroot=${RPI_SYSROOT}', '-std=c++17']
cpp_link_args = ['--sysroot=${RPI_SYSROOT}']

[properties]
pkg_config_libdir = ['${RPI_SYSROOT}/usr/local/lib/pkgconfig', '${RPI_SYSROOT}/usr/lib/pkgconfig']

[host_machine]
system = 'linux'
cpu_family = '${HOST_ARCH}'
cpu = '${HOST_ARCH}'
endian = 'little'
EOF

# Configure with Meson using system pcre2 and disabling unnecessary components
export PKG_CONFIG_PATH="${RPI_SYSROOT}/usr/local/lib/pkgconfig:${PKG_CONFIG_PATH}"
meson setup build \
  --cross-file "glib.meson.ini" \
  --prefix=/usr/local \
  --libdir=lib \
  --buildtype=release \
  --default-library=shared \
  -Dtests=false \
  -Dselinux=disabled \
  -Dxattr=false \
  -Dlibmount=disabled \
  -Dman=false \
  -Ddtrace=false

# Build
ninja -C build -j$(($(nproc) * 2))

# Install
DESTDIR="${RPI_SYSROOT}" ninja -C build install
DESTDIR="${RPI_STAGING}" ninja -C build install

# Verify pkg-config file installation
echo "Checking for glib-2.0.pc in sysroot:"
find "${RPI_SYSROOT}" -name "glib-2.0.pc"

# Fix pkg-config files if needed - adjust paths in .pc files
for pcfile in $(find "${RPI_SYSROOT}/usr/local/lib/pkgconfig" -name "*.pc" 2>/dev/null); do
    # Modify any absolute paths in pkg-config files to be relative to the sysroot
    sed -i 's|^prefix=.*|prefix=/usr/local|g' "$pcfile"
    echo "Fixed pkg-config file: $pcfile"
done

# Clean up
popd
rm -rf glib-build