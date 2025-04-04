#!/usr/bin/env bash

set -e

cd "$(dirname "${BASH_SOURCE[0]}")"

function print_usage {
    echo
    echo "Usage"
    echo "    $0 <board> [--dev] [--push] [--pull] [--export] [--buildx]"
    echo
    echo "Boards"
    echo "    rpi"
    echo "        Raspberry Pi 1, 32-bit."
    echo "        Recommended for: RPi A/B/A+/B+, CM 1, RPi Zero/Zero W"
#    echo
#    echo "    rpi-dev"
#    echo "        Raspberry Pi 1, 32-bit, with development tools."
#    echo "        Recommended for: RPi A/B/A+/B+, CM 1, RPi Zero/Zero W"
    echo
    echo "    rpi3-armv8"
    echo "        Raspberry Pi 3, 32-bit."
    echo "        Recommended for: RPi 2B rev. 1.2, RPi 3B/3B+, CM 3, RPi 4B/400, CM 4, RPi Zero 2 W"
#    echo 
#    echo "    rpi3-armv8-dev"
#    echo "        Raspberry Pi 3, 32-bit, with development tools"
#    echo "        Recommended for: RPi 2B rev. 1.2, RPi 3B/3B+, CM 3, RPi 4B/400, CM 4, RPi Zero 2 W"
    echo
    echo "    rpi3-aarch64"
    echo "        Raspberry Pi 3, 64-bit."
    echo "        Recomended for: RPi 2B rev. 1.2, RPi 3B/3B+, CM 3, RPi 4B/400, CM 4, RPi Zero 2 W"
#    echo
#    echo "    rpi3-aarch64-dev"
#    echo "        Raspberry Pi 3, 64-bit, with development tools"
#    echo "        Recomended for: RPi 2B rev. 1.2, RPi 3B/3B+, CM 3, RPi 4B/400, CM 4, RPi Zero 2 W"
    echo
    echo "Options"
    echo "    --dev"
    echo "        Cross-compile the development tools as well (e.g. distcc, ccache, CMake, Git, Ninja, Make)"
    echo
    echo "    --push"
    echo "        After building, push the resulting image to Docker Hub"
    echo
    echo "    --pull"
    echo "        Don't build the image locally, pull everything from Docker Hub"
    echo
    echo "    --export"
    echo "        Export the toolchain, sysroot and staging area to your computer"
    echo
    echo "    --buildx"
    echo "        Build the image using docker buildx"
    echo
}

# Check the number of arguments
if [ "$#" -lt 1 ]; then
    echo
    echo "Build or pull the Raspberry Pi GCC toolchain and cross-compiled libraries."
    print_usage
    exit 0
fi

# Check the board name
name="$1"
case "$name" in
rpi)
    target=armv6-rpi-linux-gnueabihf
    arch=armv6
    board=rpi
    dev=nodev
    ;;
rpi-dev)
    target=armv6-rpi-linux-gnueabihf
    arch=armv6
    board=rpi
    dev=dev
    ;;
rpi3-armv8)
    target=armv8-rpi3-linux-gnueabihf
    arch=armv8
    board=rpi3
    dev=nodev
    ;;
rpi3-armv8-dev)
    target=armv8-rpi3-linux-gnueabihf
    arch=armv8
    board=rpi3
    dev=dev
    ;;
rpi3-aarch64)
    target=aarch64-rpi3-linux-gnu
    arch=aarch64
    board=rpi3
    dev=nodev
    ;;
rpi3-aarch64-dev)
    target=aarch64-rpi3-linux-gnu
    arch=aarch64
    board=rpi3
    dev=dev
    ;;
*) echo; echo "Unknown board option '$1'"; print_usage; exit 1 ;;
esac

# Parse the other options
shift

build=true
push=false
export=false
docker_build_cpuset=
buildx=
cache=true

while (( "$#" )); do
    case "$1" in
        --push)             push=true                            ;;
        --pull)             build=false                          ;;
        --export)           export=true                          ;;
        --buildx)           buildx=buildx                        ;;
        --cpuset-cpus=*)    docker_build_cpuset="$1"             ;;
        --dev)              dev=dev                              ;;
        --no-cache)            cache=false                           ;;
        *) echo; echo "Unknown option '$1'"; print_usage; exit 1 ;;
    esac
    shift
done

# Add -dev to tag if development build was selected
case "$dev" in
nodev)
    docker_target=build
    tag=$target
    ;;
dev)
    docker_target=developer-build
    tag=$target-dev
    ;;
esac

# Build or pull the Docker image with cross-compiled libraries
image=tttapa/docker-arm-cross-build-scripts:$tag
if [ $build = true ]; then
    echo "Building Docker image $image"
    . env/$target.env
    build_args=$(python3 ./env/env2arg.py env/$target.env)
    pushd cross-build

    if [ $cache = false ]; then
        docker $buildx build \
            --tag $image \
            ${build_args} \
            --target $docker_target \
            --no-cache .
    else
        docker $buildx build \
            --tag $image \
            ${build_args} \
            --target $docker_target \
            .
    fi

    popd
    # Push the Docker image 
    if [ $push = true ]; then
        echo "Pushing Docker image $image"
        docker push $image
    fi
else
    echo "Pulling Docker image $image"
    [ ! -z $(docker images -q $image) ] || docker pull $image
fi

# Export the toolchain etc. from the Docker image to the computer
image=tttapa/docker-arm-cross-build-scripts:$tag
if [ $export = true ]; then
    . ./scripts/export.sh
    export_all $image $target $target
fi
