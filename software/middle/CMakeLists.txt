cmake_minimum_required(VERSION 3.23)

include(pico_sdk_import.cmake)

# Metadata
project(bottom C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# link libraries
find_package(spdlog REQUIRED)
pico_sdk_init()
link_libraries(pico_stdlib spdlog::spdlog)

# Add them as an executable
add_executable(main src/main/main.cpp)
pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 1)
pico_add_extra_outputs(main)

# Tests
add_subdirectory("test")