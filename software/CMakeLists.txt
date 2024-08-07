cmake_minimum_required(VERSION 3.30.2)
project(
  RBC-2025
  VERSION 1.0
  DESCRIPTION
  "Robocup Soccer 2025"
)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)

# Include directories
include_directories("${CMAKE_SOURCE_DIR}/lib" "${CMAKE_SOURCE_DIR}/src/main" "${CMAKE_SOURCE_DIR}/src/input" "${CMAKE_SOURCE_DIR}/src/output" "${CMAKE_SOURCE_DIR}/src/strategy" "${CMAKE_SOURCE_DIR}/src/utils")

# Collect all source files
file(GLOB_RECURSE LIB_SOURCES lib/*.cpp lib/*.hpp)
file(GLOB_RECURSE SRC_SOURCES src/*.cpp src/*.hpp)
file(GLOB_RECURSE UTILS_SOURCES utils/*.cpp utils/*.hpp)

# Add executable target
add_executable(RBC-2025
  ${LIB_SOURCES}
  ${SRC_SOURCES}
  ${UTILS_SOURCES}
  src/main/main.cpp
)