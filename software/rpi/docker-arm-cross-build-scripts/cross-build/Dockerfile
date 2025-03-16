ARG CROSS_TOOLCHAIN_IMAGE

FROM ghcr.io/tttapa/docker-arm-cross-toolchain:${CROSS_TOOLCHAIN_IMAGE} as cross-toolchain

FROM tttapa/rpi-cpp-toolchain-base-ubuntu as build-base

ARG HOST_ARCH
ARG HOST_TRIPLE
ARG HOST_TRIPLE_NO_VENDOR
ARG HOST_TRIPLE_LIB_DIR
ARG HOST_BITNESS
ARG HOST_PYTHON_MACHINE

ENV HOST_ARCH ${HOST_ARCH}
ENV HOST_TRIPLE ${HOST_TRIPLE}
ENV HOST_TRIPLE_NO_VENDOR ${HOST_TRIPLE_NO_VENDOR}
ENV HOST_TRIPLE_LIB_DIR ${HOST_TRIPLE_LIB_DIR}
ENV HOST_BITNESS ${HOST_BITNESS}
ENV HOST_PYTHON_MACHINE ${HOST_PYTHON_MACHINE}

ENV TOOLCHAIN_PATH="/home/develop/x-tools/${HOST_TRIPLE}"
ENV PATH="${PATH}:${TOOLCHAIN_PATH}/bin"
ENV DOWNLOADS="/tmp/downloads"

ENV RPI_SYSROOT="/home/develop/RPi-sysroot"
ENV RPI_STAGING="/home/develop/RPi-staging"

# Copy the cross-compilation toolchain
COPY --from=cross-toolchain \
    "/home/develop/x-tools/${HOST_TRIPLE}" \
    "${TOOLCHAIN_PATH}"

# Create a sysroot and staging area for the RPi
WORKDIR /home/develop
RUN mkdir "${RPI_SYSROOT}" "${RPI_STAGING}" && \
    cp -rp "${TOOLCHAIN_PATH}/${HOST_TRIPLE}/sysroot/"* "${RPI_SYSROOT}"/ && \
    chmod -R u+w "${RPI_SYSROOT}"
RUN mkdir -p "${DOWNLOADS}"
RUN mkdir -p ${RPI_STAGING}/usr/local && \
    [ "${HOST_BITNESS}" -eq "64" ] && \
    ln -s lib "${RPI_STAGING}/usr/local/lib64" || \
    true
RUN mkdir -p ${RPI_SYSROOT}/usr/local && \
    [ "${HOST_BITNESS}" -eq "64" ] && \
    ln -s lib "${RPI_SYSROOT}/usr/local/lib64" || \
    true

# Copy the cross-compilation pkg-config wrappers
COPY --chown=develop:develop tools/ ./tools/
ENV PATH="/home/develop/tools:${PATH}"

# Create a symlink for /usr/local/lib64
RUN sudo ln -s lib /usr/local/lib64

# Zlib, OpenSSL, Python dependencies 
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

FROM build-base AS python-deps-build

# Zlib build
COPY --chown=develop:develop install-scripts/zlib-build.sh .
RUN chmod +x zlib-build.sh && ./zlib-build.sh

# Zlib ARM
COPY --chown=develop:develop install-scripts/zlib.sh .
RUN chmod +x zlib.sh && ./zlib.sh
 
# OpenSSL build
COPY --chown=develop:develop install-scripts/openssl-build.sh .
RUN chmod +x openssl-build.sh && ./openssl-build.sh

# OpenSSL ARM
COPY --chown=develop:develop install-scripts/openssl.sh .
RUN chmod +x openssl.sh && ./openssl.sh

# FFI build
COPY --chown=develop:develop install-scripts/libffi-build.sh .
RUN chmod +x libffi-build.sh && ./libffi-build.sh

# FFI ARM
COPY --chown=develop:develop install-scripts/libffi.sh .
RUN chmod +x libffi.sh && ./libffi.sh

# Bzip2 ARM
COPY --chown=develop:develop install-scripts/bzip2.sh .
RUN chmod +x bzip2.sh && ./bzip2.sh

# GNU NCurses build
COPY --chown=develop:develop install-scripts/ncurses-build.sh .
RUN chmod +x ncurses-build.sh && ./ncurses-build.sh

# GNU NCurses ARM
COPY --chown=develop:develop install-scripts/ncurses.sh .
RUN chmod +x ncurses.sh && ./ncurses.sh

# GNU Readline ARM
COPY --chown=develop:develop install-scripts/readline.sh .
RUN chmod +x readline.sh && ./readline.sh

# GNU DBM ARM
COPY --chown=develop:develop install-scripts/dbm.sh .
RUN chmod +x dbm.sh && ./dbm.sh

# SQLite ARM
COPY --chown=develop:develop install-scripts/sqlite.sh .
RUN chmod +x sqlite.sh && ./sqlite.sh

# UUID ARM
COPY --chown=develop:develop install-scripts/uuid.sh .
RUN chmod +x uuid.sh && ./uuid.sh

# Userland, FFmpeg, ...
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

FROM build-base AS video-build
 
# Raspberry Pi Userland (VideoCore)
COPY install-scripts/${HOST_TRIPLE}.userland.cmake .
COPY --chown=develop:develop install-scripts/userland.sh .
RUN chmod +x userland.sh && ./userland.sh

# VPX ARM
COPY --chown=develop:develop install-scripts/vpx.sh .
RUN chmod +x vpx.sh && ./vpx.sh

# x264 ARM
COPY --chown=develop:develop install-scripts/x264.sh .
RUN chmod +x x264.sh && ./x264.sh

# Xvid ARM
COPY --chown=develop:develop install-scripts/xvid.sh .
RUN chmod +x xvid.sh && ./xvid.sh

# FFmpeg ARM
COPY --chown=develop:develop install-scripts/ffmpeg.sh .
RUN chmod +x ffmpeg.sh && ./ffmpeg.sh

# X11
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

FROM build-base AS x11-build

# X11 ARM
COPY --chown=develop:develop install-scripts/x11.sh .
COPY --chown=develop:develop patches/libx11-1.6.9.configure.ac.patch .
RUN chmod +x x11.sh && ./x11.sh

# Tcl/Tk
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# Tcl/Tk ARM
COPY --chown=develop:develop install-scripts/tk.sh .
RUN chmod +x tk.sh && ./tk.sh

# Python
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

FROM build-base AS python-build

COPY --from=python-deps-build --chown=develop ${RPI_SYSROOT} ${RPI_SYSROOT}
COPY --from=python-deps-build --chown=develop ${RPI_STAGING} ${RPI_STAGING}
COPY --from=python-deps-build /usr/local /usr/local

COPY --from=x11-build --chown=develop ${RPI_SYSROOT} ${RPI_SYSROOT}
COPY --from=x11-build --chown=develop ${RPI_STAGING} ${RPI_STAGING}

RUN sudo ldconfig

# Python build
COPY --chown=develop:develop install-scripts/python-build.sh .
RUN chmod +x python-build.sh && ./python-build.sh

# Python ARM
COPY --chown=develop:develop install-scripts/python.sh .
RUN chmod +x python.sh && ./python.sh

# NumPy, SciPy, OpenCV
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

FROM python-build AS scipy-build

# OpenBLAS ARM
COPY --chown=develop:develop install-scripts/openblas.sh .
RUN chmod +x openblas.sh && ./openblas.sh

# Install NumPy and Crossenv on build
RUN python3.10 -m pip install crossenv
RUN python3.10 -m crossenv \
    --machine ${HOST_PYTHON_MACHINE} \
    "${RPI_SYSROOT}/usr/local/bin/python3.10" crossenv

# Wheel, Cython, Pybind11, Numpy and Scipy packages ARM
RUN . crossenv/bin/activate && pip install wheel cython pybind11
COPY --chown=develop:develop install-scripts/numpy.sh .
COPY --chown=develop:develop patches/npymath.ini.patch .
RUN chmod +x numpy.sh && ./numpy.sh
COPY --chown=develop:develop install-scripts/pythran.sh .
RUN chmod +x pythran.sh && ./pythran.sh
COPY --chown=develop:develop patches/numpy.utils.py.patch .
RUN . crossenv/bin/activate && patch "$VIRTUAL_ENV/../build/lib/python3.10/site-packages/numpy/lib/utils.py" numpy.utils.py.patch
COPY --chown=develop:develop install-scripts/scipy.sh .
RUN chmod +x scipy.sh && ./scipy.sh

# OpenCV
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

FROM scipy-build AS opencv-build

COPY --from=video-build --chown=develop ${RPI_STAGING} ${RPI_STAGING}
COPY --from=video-build --chown=develop ${RPI_SYSROOT} ${RPI_SYSROOT}

# OpenCV ARM
COPY --chown=develop:develop install-scripts/opencv.sh .
RUN chmod +x opencv.sh && ./opencv.sh

# Move the compiled Python packages to the staging area
RUN rm -rf crossenv/cross/lib/python3.10/site-packages/__pycache__ && \
    mv "crossenv/cross/lib/python3.10/site-packages/"* \
       "${RPI_STAGING}/usr/local/lib/python3.10/site-packages"

# GoogleTest
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

FROM build-base AS gtest-build

COPY --chown=develop:develop install-scripts/${HOST_TRIPLE}.cmake .
COPY --chown=develop:develop install-scripts/googletest.sh .
RUN chmod +x googletest.sh && ./googletest.sh

# CMake & Ninja
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

FROM build-base AS cmake-build

# CMake build
COPY --chown=develop:develop install-scripts/cmake-build.sh .
RUN chmod +x cmake-build.sh && ./cmake-build.sh

# Ninja build
COPY --chown=develop:develop install-scripts/ninja-build.sh .
RUN chmod +x ninja-build.sh && ./ninja-build.sh

# System libraries
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

FROM build-base AS build

COPY --from=opencv-build --chown=develop /home/develop/crossenv /home/develop/crossenv
COPY --from=opencv-build /usr/local /usr/local

COPY --from=cmake-build /usr/local /usr/local

COPY --from=opencv-build --chown=develop ${RPI_SYSROOT} ${RPI_SYSROOT}
COPY --from=opencv-build --chown=develop ${RPI_STAGING} ${RPI_STAGING}

COPY --from=gtest-build --chown=develop ${RPI_SYSROOT} ${RPI_SYSROOT}
COPY --from=gtest-build --chown=develop ${RPI_STAGING} ${RPI_STAGING}

COPY --chown=develop:develop install-scripts/${HOST_TRIPLE}.cmake .

# Copy gdbserver to the staging area, as well as the latest C++ and Fortran 
# libraries, OpenMP ...
RUN mkdir -p "${RPI_STAGING}/usr/local/lib/${HOST_TRIPLE_LIB_DIR}/" && \
    cp -a \
        "${RPI_SYSROOT}/lib/libstdc++.so"* \
        "${RPI_STAGING}/usr/local/lib/${HOST_TRIPLE_LIB_DIR}/" && \
    cp -a \
        "${RPI_SYSROOT}/lib/libgomp.so"* \
        "${RPI_STAGING}/usr/local/lib/${HOST_TRIPLE_LIB_DIR}/" && \
    cp -a \
        "${RPI_SYSROOT}/lib/libgfortran.so"* \
        "${RPI_STAGING}/usr/local/lib/${HOST_TRIPLE_LIB_DIR}/" && \
    cp -a \
        "$TOOLCHAIN_PATH/${HOST_TRIPLE}/debug-root/usr/bin/gdbserver" \
        "${RPI_STAGING}/usr/local/bin"

RUN sudo ldconfig

# Development build
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

FROM build as developer-build

COPY --from=scipy-build /usr/local /usr/local
COPY --from=scipy-build --chown=develop /home/develop/crossenv /home/develop/crossenv

# DistCC ARM
COPY --chown=develop:develop install-scripts/distcc.sh .
COPY --chown=develop:develop patches/pump.patch .
RUN chmod +x distcc.sh && ./distcc.sh

# Make ARM
COPY --chown=develop:develop install-scripts/make.sh .
RUN chmod +x make.sh && ./make.sh

# CMake ARM
COPY --chown=develop:develop install-scripts/cmake.sh .
RUN chmod +x cmake.sh && ./cmake.sh

# CCache ARM
COPY --chown=develop:develop install-scripts/ccache.sh .
RUN chmod +x ccache.sh && ./ccache.sh

# cURL ARM
COPY --chown=develop:develop install-scripts/curl.sh .
RUN chmod +x curl.sh && ./curl.sh

# Git ARM
COPY --chown=develop:develop install-scripts/git.sh .
RUN chmod +x git.sh && ./git.sh

# Ninja ARM
COPY --chown=develop:develop install-scripts/ninja.sh .
RUN chmod +x ninja.sh && ./ninja.sh
