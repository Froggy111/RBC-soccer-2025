cmake_minimum_required(VERSION 3.20)
project(greeter VERSION 1.0.0 LANGUAGES CXX)
enable_testing()

set(RPI_SYSROOT ./docker-arm-cross-build-scripts/sysroot-aarch64-rpi3-linux-gnu)
include_directories(${RPI_SYSROOT}/usr/local/include/libcamera)
include_directories(${RPI_SYSROOT}/usr/local/include/opencv4)

add_subdirectory("libs")
add_subdirectory("src")
add_subdirectory("tests")

# Packaging
set(CPACK_GENERATOR "TGZ")
set(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME}-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR})
include(CPack)
