# Provide a default value if not set externally
ARG CROSS_TOOLCHAIN_IMAGE=latest

FROM ghcr.io/tttapa/docker-arm-cross-toolchain:${CROSS_TOOLCHAIN_IMAGE} AS cross-toolchain

FROM tttapa/rpi-cpp-toolchain-base-ubuntu AS build-base

ARG HOST_ARCH
ARG HOST_TRIPLE
ARG HOST_TRIPLE_NO_VENDOR
ARG HOST_TRIPLE_LIB_DIR
ARG HOST_BITNESS
ARG HOST_PYTHON_MACHINE

ENV HOST_ARCH=${HOST_ARCH} \
    HOST_TRIPLE=${HOST_TRIPLE} \
    HOST_TRIPLE_NO_VENDOR=${HOST_TRIPLE_NO_VENDOR} \
    HOST_TRIPLE_LIB_DIR=${HOST_TRIPLE_LIB_DIR} \
    HOST_BITNESS=${HOST_BITNESS} \
    HOST_PYTHON_MACHINE=${HOST_PYTHON_MACHINE}

ENV TOOLCHAIN_PATH="/home/develop/opt/x-tools/${HOST_TRIPLE}"
ENV PATH="${PATH}:${TOOLCHAIN_PATH}/bin"
ENV DOWNLOADS="/tmp/downloads"

ENV RPI_SYSROOT="/home/develop/RPi-sysroot"
ENV RPI_STAGING="/home/develop/RPi-staging"

# Copy the cross-compilation toolchain from the earlier stage
COPY --from=cross-toolchain "/home/develop/opt/x-tools/${HOST_TRIPLE}" "${TOOLCHAIN_PATH}"

WORKDIR /home/develop

# Create sysroot and staging directories
RUN mkdir "${RPI_SYSROOT}" "${RPI_STAGING}" && \
    cp -rp "${TOOLCHAIN_PATH}/${HOST_TRIPLE}/sysroot/"* "${RPI_SYSROOT}/" && \
    chmod -R u+w "${RPI_SYSROOT}"
RUN mkdir -p "${DOWNLOADS}"
RUN mkdir -p ${RPI_STAGING}/usr/local && [ "${HOST_BITNESS}" -eq "64" ] && ln -s lib "${RPI_STAGING}/usr/local/lib64" || true
RUN mkdir -p ${RPI_SYSROOT}/usr/local && [ "${HOST_BITNESS}" -eq "64" ] && ln -s lib "${RPI_SYSROOT}/usr/local/lib64" || true

# Copy pkg-config wrappers and update PATH
COPY --chown=develop:develop tools/ ./tools/
ENV PATH="/home/develop/tools:${PATH}"

# Create a symlink for /usr/local/lib64
RUN sudo ln -s lib /usr/local/lib64

# ---------------------------------------------------------------------------
# Stage: Video Dependencies Build (needed for OpenCV)
# ---------------------------------------------------------------------------
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

# ---------------------------------------------------------------------------
# Stage: OpenCV Build
# ---------------------------------------------------------------------------
FROM video-build AS opencv-build

# Ensure sysroot and staging areas are available
COPY --from=video-build --chown=develop ${RPI_STAGING} ${RPI_STAGING}
COPY --from=video-build --chown=develop ${RPI_SYSROOT} ${RPI_SYSROOT}

# Build OpenCV ARM
COPY --chown=develop:develop install-scripts/opencv.sh .
RUN chmod +x opencv.sh && ./opencv.sh

# ---------------------------------------------------------------------------
# Final stage: Name the stage "build" to satisfy the build target
# ---------------------------------------------------------------------------
FROM opencv-build AS build
