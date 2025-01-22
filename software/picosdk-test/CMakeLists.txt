cmake_minimum_required(VERSION 3.23)

include(pico_sdk_import.cmake)

project(pico-test C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(blink blink.c) 

pico_enable_stdio_usb(blink 1)

pico_add_extra_outputs(blink)

target_link_libraries(blink pico_stdlib)
