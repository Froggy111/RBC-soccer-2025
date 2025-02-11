# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
  set(USERHOME $ENV{USERPROFILE})
else()
  set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
  include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

cmake_minimum_required(VERSION 3.23)

include(pico_sdk_import.cmake)

# Metadata
project(bottom C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Init PicoSDK
pico_sdk_init()

# globally used
link_libraries(pico_stdlib)
link_libraries(pico_stdio_usb)
# root directory as include directory
include_directories(${CMAKE_SOURCE_DIR})

# Libraries
add_subdirectory(libs)

# Main
add_subdirectory(src)

# Tests
add_subdirectory(test)
