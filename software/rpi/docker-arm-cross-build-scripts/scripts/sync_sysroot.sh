#!/bin/bash

SYSROOT="$(pwd)/docker-arm-cross-build-scripts/sysroot-aarch64-rpi3-linux-gnu"
PI_ADDRESS="botbywire@192.168.210.239"
PI_DEST="/usr/local/lib"

# Make sure the sysroot directory exists
if [ ! -d "$SYSROOT" ]; then
    echo "Error: Sysroot directory not found at $SYSROOT"
    exit 1
fi

# Create destination directory with proper permissions if it doesn't exist
ssh $PI_ADDRESS "sudo mkdir -p $PI_DEST && sudo chmod 755 $PI_DEST"

# Sync only the relevant libraries from sysroot to the Pi
echo "Syncing libraries to $PI_ADDRESS:$PI_DEST..."
rsync -avz --rsync-path="sudo rsync" \
    $SYSROOT/usr/lib/* \
    $SYSROOT/lib/* \
    $PI_ADDRESS:$PI_DEST

# Update the shared library cache on the Pi
echo "Updating shared library cache on the Pi..."
ssh $PI_ADDRESS "sudo ldconfig"

echo "Sync completed successfully!"