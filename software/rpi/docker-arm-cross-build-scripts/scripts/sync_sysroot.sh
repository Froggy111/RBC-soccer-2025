#!/bin/bash
SYSROOT="$(pwd)/docker-arm-cross-build-scripts/sysroot-aarch64-rpi3-linux-gnu"

echo "Syncing libraries from Raspberry Pi to sysroot at $SYSROOT..."

ls

# Sync libraries from the Pi
rsync -avz --copy-links --safe-links botbywire@192.168.210.239:/lib/aarch64-linux-gnu/ $SYSROOT/lib/aarch64-linux-gnu/
rsync -avz --copy-links --safe-links botbywire@192.168.210.239:/usr/lib/aarch64-linux-gnu/ $SYSROOT/usr/lib/aarch64-linux-gnu/
rsync -avz --copy-links --safe-links botbywire@192.168.210.239:/usr/local/lib/ $SYSROOT/usr/local/lib/

echo "Sysroot update complete!"