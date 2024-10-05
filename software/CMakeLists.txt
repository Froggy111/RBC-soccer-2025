cmake_minimum_required(VERSION 3.30.2)
project(
  RBC-2025
  VERSION 1.0
  DESCRIPTION
  "Robocup Soccer 2025"
)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(SOURCE_FILES "src/main/main.cpp")

# Set the output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build)

# Source
file(GLOB_RECURSE SRC_FILES "src/*.cpp")

# Include relevant directories
include_directories("src")

# Find spdlog package
find_package(spdlog REQUIRED)

# Add them as an executable
add_executable(${PROJECT_NAME} ${SRC_FILES})

# Link spdlog library
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)