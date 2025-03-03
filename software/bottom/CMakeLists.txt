# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
  set(USERHOME $ENV{USERPROFILE})
else()
  set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
  include(${picoVscode})
endif()
# ====================================================================================

# ================================= HELPER FUNCTIONS =================================
# Initialize the global libraries list
set(GLOBAL_LIBS "" CACHE INTERNAL "List of global libraries")

# Function to add a library to the global list
function(add_global_library lib)
  # Ensure the library exists
  if(NOT TARGET ${lib})
    message(FATAL_ERROR "add_global_library: Target '${lib}' does not exist")
  endif()

  # Add to global list if not already there
  set(GLOBAL_LIBS ${GLOBAL_LIBS} CACHE INTERNAL "List of global libraries")
  list(APPEND GLOBAL_LIBS ${lib})
  list(REMOVE_DUPLICATES GLOBAL_LIBS)
  set(GLOBAL_LIBS ${GLOBAL_LIBS} CACHE INTERNAL "List of global libraries")
endfunction()

# Function to link a target with all global libraries
function(target_link_globals target_name)
  # Ensure the target exists
  if(NOT TARGET ${target_name})
    message(FATAL_ERROR "target_link_globals: Target '${target_name}' does not exist")
  endif()

  # Link all global libraries to the target
  foreach(lib ${GLOBAL_LIBS})
    target_link_libraries(${target_name} ${lib})
  endforeach()
endfunction()
# ====================================================================================

# ===================================== PICO SDK =====================================
set(PICO_BOARD pico CACHE STRING "Board type")

# use external tinyusb (picosdk provided tinyusb hardcodes CFG_TUSB_OS as OPT_OS_PICO, but we want OPT_OS_FREERTOS)
# if testing without comms lib and freertos, comment this out
set(PICO_SDK_TINYUSB_PATH "")
set(PICO_TINYUSB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/tinyusb")

cmake_minimum_required(VERSION 3.23)

include(pico_sdk_import.cmake)

# Metadata
project(bottom C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Init PicoSDK
# set(PICOTOOL_FORCE_FETCH_FROM_GIT ON)
pico_sdk_init()
# ====================================================================================

# ===================================== FREERTOS =====================================
# FreeRTOS
if(NOT DEFINED FREERTOS_KERNEL_PATH)
  set(FREERTOS_KERNEL_PATH ${CMAKE_SOURCE_DIR}/external/freertos)
endif()
include(FreeRTOS_Kernel_import.cmake)
# ====================================================================================

add_global_library(pico_stdlib)
# add_global_library(pico_stdio_usb) # only include this for testing without using our own comms lib
add_global_library(pico_async_context_freertos)
add_global_library(FreeRTOS-Kernel-Heap4)

# add tinyusb
add_subdirectory(${PICO_TINYUSB_PATH}/src/ ${CMAKE_BINARY_DIR}/tinyusb)

# configs
add_subdirectory(configs)

# Libraries
add_subdirectory(libs) # global libs need to be explicitly linked to with target_link_globals(TARGET)

# Main
add_subdirectory(src)

# Tests
add_subdirectory(test)
