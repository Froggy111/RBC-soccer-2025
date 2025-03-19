cmake_minimum_required(VERSION 3.23)

# Metadata
project(middle-rpi C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory("libs")
add_subdirectory("src")
add_subdirectory("tests")