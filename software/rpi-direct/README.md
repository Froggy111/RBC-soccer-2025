# Raspberry PI Zero 2W Setup

1. Install **Docker**
2. Run: `./docker-arm-cross-build-scripts/build.sh rpi3-aarch64 --pull --export`
3. For VSCode's CMake Tools Extension, `Command + Shift + P` => `CMake: Select a Kit` => `Raspberry Pi 3 (AArch64)`
4. Build Normally, find the binary in `./build` directory (in the same relative subdirectory where the executable was added)