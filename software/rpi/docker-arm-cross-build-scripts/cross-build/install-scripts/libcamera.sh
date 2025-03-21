#!/usr/bin/env bash

set -ex

# Download
# Using the git repository as libcamera doesn't have frequent releases
version="0.4.0"
git_url="https://github.com/libcamera-org/libcamera.git"  # Use actual git URL, not tarball

pushd "${DOWNLOADS}"
if [ ! -d libcamera-git ]; then
  git clone ${git_url} libcamera-git
fi
cd libcamera-git
git fetch
git checkout "v${version}"  # Checkout the specific version
popd

# Create a working copy
rm -rf libcamera-build
mkdir -p libcamera-build
cp -r "${DOWNLOADS}/libcamera-git/"* libcamera-build/
pushd libcamera-build

# Source cross-pkg-config to set pkg-config to find libraries in the sysroot
. cross-pkg-config

# Make sure PKG_CONFIG_PATH is properly set to include GLib locations
export PKG_CONFIG_PATH="${RPI_SYSROOT}/usr/local/lib/pkgconfig:${RPI_SYSROOT}/usr/lib/pkgconfig:${RPI_SYSROOT}/usr/lib/${HOST_TRIPLE}/pkgconfig:${PKG_CONFIG_PATH}"

# Create symbolic link for glib lib if missing (sometimes needed)
if [ ! -f "${RPI_SYSROOT}/usr/local/lib/libglib-2.0.so" ] && [ -f "${RPI_SYSROOT}/usr/local/lib/libglib-2.0.so.0" ]; then
  ln -sf "${RPI_SYSROOT}/usr/local/lib/libglib-2.0.so.0" "${RPI_SYSROOT}/usr/local/lib/libglib-2.0.so"
fi

# Debug: Show where glib-2.0.pc is located
find ${RPI_SYSROOT} -name "glib-2.0.pc" || echo "glib-2.0.pc not found in sysroot"

# Create a Meson cross file for the target architecture
cat > cross-file.txt << EOF
[binaries]
c = '${HOST_TRIPLE}-gcc'
cpp = '${HOST_TRIPLE}-g++'
ar = '${HOST_TRIPLE}-ar'
strip = '${HOST_TRIPLE}-strip'
pkgconfig = 'pkg-config'

[built-in options]
c_args = ['-O2', '-pipe', '--sysroot=${RPI_SYSROOT}', '-Wno-error=array-bounds']
c_link_args = ['--sysroot=${RPI_SYSROOT}', '-L${RPI_SYSROOT}/usr/local/lib']
cpp_args = ['-O2', '-pipe', '--sysroot=${RPI_SYSROOT}', '-std=c++17', '-Wno-error=array-bounds']
cpp_link_args = ['--sysroot=${RPI_SYSROOT}', '-L${RPI_SYSROOT}/usr/local/lib']

[properties]
pkg_config_libdir = ['${RPI_SYSROOT}/usr/local/lib/pkgconfig', '${RPI_SYSROOT}/usr/lib/pkgconfig', '${RPI_SYSROOT}/usr/lib/${HOST_TRIPLE}/pkgconfig']

[host_machine]
system = 'linux'
cpu_family = '${HOST_ARCH}'
cpu = '${HOST_ARCH}'
endian = 'little'
EOF

# Configure with Meson
LDFLAGS="-L${RPI_SYSROOT}/usr/local/lib" meson setup build \
  --cross-file cross-file.txt \
  --prefix=/usr/local \
  --libdir=lib \
  --buildtype=release \
  --default-library=shared \
  -Dpipelines=rpi/vc4 \
  -Dv4l2=true \
  -Dgstreamer=disabled \
  -Dqcam=disabled \
  -Ddocumentation=disabled \
  -Dlc-compliance=disabled \
  -Dpycamera=disabled \
  -Dtest=false \
  -Dcam=disabled

# Build
ninja -C build -j$(($(nproc) * 2))

# Install
DESTDIR="${RPI_SYSROOT}" ninja -C build install
DESTDIR="${RPI_STAGING}" ninja -C build install

# Cleanup
popd
rm -rf libcamera-build